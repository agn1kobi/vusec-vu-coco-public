/**
 * Bounds checker implementation
*/

#define DEBUG_TYPE "BoundsChecker"
#define EXPECTED_BLOCK_NUMS 16
#include "utils.h"
#include <string>
#include <iostream>

using namespace llvm;
using namespace std;


namespace {
    class BoundsChecker : public ModulePass {
    public:
        static char ID;
        BoundsChecker() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

        vector<Function*> deletedFucntions;
        vector<Function*> addedFunctions;

        bool isChanged = false;


    private:
       Function *boundsCheckCall;

        Value *getNextOperand(Argument *Arg, Function *Func);

        bool instrumentGeps(Function &F);
        Value *gepOffsetAccumulator(Value *V, IRBuilder<> &B);
        Value *determineArraySize(Value *V, IRBuilder<> &B, map<Value*, Value*> &sizeMap);
        SmallVector<Value *, EXPECTED_BLOCK_NUMS> Visited;
        Value *one;

        bool isFunctionCalledFromOtherPlaces(Function &TargetFunction);
        Function* calcArgs(Function &F);
        CallInst *findAndProcessCalls(Function &F, CallInst *callCmp);

    };
}

Value *BoundsChecker::getNextOperand(Argument *Arg, Function *Func){

    LOG_LINE("yes");
    auto ArgIter = Func->arg_begin();
    LOG_LINE("yes");
    while (&*ArgIter != Arg) {
        ++ArgIter;
    }
    LOG_LINE("yes");

    Argument *returnArg = &*(++ArgIter);

    return returnArg;

}

Value *BoundsChecker::determineArraySize(Value *V, IRBuilder<> &B, map<Value*, Value*> &sizeMap) {

    if (sizeMap.find(V) != sizeMap.end()) {
        return sizeMap[V];
    }

    if(isa<AllocaInst>(V)) {
        AllocaInst *AI = dyn_cast<AllocaInst>(V);
        return AI->getArraySize();
    }
    else if (isa<GlobalVariable>(V)) {
        GlobalVariable *GV = dyn_cast<GlobalVariable>(V);
        if (GV->hasInitializer() && GV->getInitializer()->getType()->isArrayTy()) {
            ArrayType *ArrType = cast<ArrayType>(GV->getInitializer()->getType());
            return ConstantInt::get(B.getInt32Ty(), ArrType->getNumElements());
        }
    }
    else if(isa<Constant>(V)) {
        Constant *C = dyn_cast<Constant>(V);
        if (ConstantInt *CI = dyn_cast<ConstantInt>(C)) {
            return CI;
        }else {
            return B.getInt32(1);
        }
    }
    else if(isa<Argument>(V)) {
        LOG_LINE("ITS AN PTR");
        Argument *Arg = dyn_cast<Argument>(V);
        if(Arg->getParent()->getName() == "main" && Arg->getArgNo() == 1) {
            Function *F = Arg->getParent();
            Argument *arc = F->getArg(0);
            Value *argc = dyn_cast<Value>(arc);
            return B.CreateAdd(argc, one);
        }
        else {
            Type *ArgType = Arg->getType();
            if (ArgType->isArrayTy()) {
                ArrayType *ArrType = cast<ArrayType>(ArgType);
                return ConstantInt::get(B.getInt32Ty(), ArrType->getNumElements());
            }

            LOG_LINE("yesb");
            Function *Func = Arg->getParent();

            return getNextOperand(Arg, Func);

            LOG_LINE("Return type failed its returning 1");

            return B.getInt32(1);
        }
    }
    else if(isa<LoadInst>(V)) {
        report_fatal_error("Error: goon load");
    }
    else if(isa<GetElementPtrInst>(V)) {
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
        return determineArraySize(GEP->getPointerOperand(), B, sizeMap);
    } else if (isa<PHINode>(V)) {
        LOG_LINE("This is a phi instruction");

        PHINode *PHI = dyn_cast<PHINode>(V);

        if (sizeMap.find(PHI) != sizeMap.end()) {
            return sizeMap[PHI];
        }

        PHINode *sizePHI = PHINode::Create(B.getInt32Ty(), PHI->getNumIncomingValues(), "size.phi", PHI);

        for (unsigned i = 0; i < PHI->getNumIncomingValues(); ++i) {
            Value *incomingValue = PHI->getIncomingValue(i);

            if (incomingValue == PHI) {
                sizePHI->addIncoming(sizePHI, PHI->getIncomingBlock(i));
            } else {
                Value *size = determineArraySize(incomingValue, B, sizeMap);
                sizePHI->addIncoming(size, PHI->getIncomingBlock(i));
            }
        }
        sizeMap[PHI] = sizePHI;
        return sizePHI;
    }

    return B.getInt32(0);
}

Value *BoundsChecker::gepOffsetAccumulator(Value *V, IRBuilder<> &B) {
    if (isa<GetElementPtrInst>(V)) {
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
        if (isa<GetElementPtrInst>(GEP->getOperand(0))) {
            Value *accumulatedOffset = gepOffsetAccumulator(GEP->getOperand(0), B);
            B.SetInsertPoint(GEP);
            Value *result = B.CreateAdd(accumulatedOffset, GEP->getOperand(1));
            return result;
        } else {
            return GEP->getOperand(1);
        }
    }
    report_fatal_error("Error in calculating offsets");
}

bool BoundsChecker::instrumentGeps(Function &F) {
    bool changed = false;

    IRBuilder<> B(&F.getEntryBlock());

    std::map<Value*, Value*> sizeMap;

    for (Instruction &II : instructions(F)) {
        Instruction *I = &II;

        if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I)) {
            
            LOG_LINE("GEP FOUNDDD");


            if (GlobalVariable *GV = dyn_cast<GlobalVariable>(GEP->getPointerOperand())) {
                if (GV->hasInitializer() && GV->getType()->getElementType()->isArrayTy()) {
                    LOG_LINE(" THIS IS A GLOBAL STRING BOUNDS CHEKING NOT NEEDED");
                    continue; 
                }
            }

            Value *offset = gepOffsetAccumulator(GEP, B);
            Value *arraySize = determineArraySize(GEP->getOperand(0), B, sizeMap);

            B.SetInsertPoint(AI);
            B.CreateCall(boundsCheckCall, {offset, arraySize} );
            changed = true;
        }
    }
    return changed;
}

bool BoundsChecker::isFunctionCalledFromOtherPlaces(Function &TargetFunction) {
    
    for (auto &FunctionInModule : TargetFunction.getParent()->functions()) {
        if (&FunctionInModule == &TargetFunction)
            continue;

        for (auto &BB : FunctionInModule) {
            for (auto &I : BB) {
                if (auto *Call = dyn_cast<CallInst>(&I)) {
                    if (Function *CalledFunction = Call->getCalledFunction()) {
                        if (CalledFunction == &TargetFunction) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}


CallInst *BoundsChecker::findAndProcessCalls(Function &F, CallInst *callCmp) {
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (CallInst *Call = dyn_cast<CallInst>(&I)) {
                if (Function *CalledFunction = Call->getCalledFunction()) {
                    if (CalledFunction->getName() == callCmp->getCalledFunction()->getName()) {
                        LOG_LINE("Call names are the same: " << CalledFunction->getName());
                        return Call;
                    }
                }
            }
        }
    }
    LOG_LINE("could not find the correct cloned name");
    return NULL;
}

Function* BoundsChecker::calcArgs(Function &F){

    if (F.getName() == "main") {
        LOG_LINE("Main function detected, returning NULL");
        return NULL;
    }
    for (size_t i = 0; i < addedFunctions.size(); i++)
    {
        if (&F == addedFunctions.at(i))
        {
            return NULL;
        }
    }
    for (size_t i = 0; i < deletedFucntions.size(); i++)
    {
        if (&F == deletedFucntions.at(i))
        {
            return NULL;
        }
    }
    
    

    Function *NewFunc;

    if (!isFunctionCalledFromOtherPlaces(F))
    {
        LOG_LINE("Not called in other places");
    }

    if (!F.isDeclaration()) {
        
        IRBuilder<> Builder(F.getContext());


        SmallVector<Argument*, 8> NewArgs;
        

        for (Argument &Arg : F.args()) {
            
            if (Arg.getType()->isPointerTy()) {
                
                LOG_LINE("This arg is a pointer type new fucntion forming");

                Type *elementType = Arg.getType()->getPointerElementType();
                NewFunc = addParamsToFunction(&F, {elementType}, NewArgs);

                isChanged = true;

                LOG_LINE("New fucntion has been made" << NewFunc);

                for (User *U : F.users()) {

                    
                    LOG_LINE("entered for loop for checking the users:");

                    if (CallInst *Call = dyn_cast<CallInst>(U)) {

                        
                        Value *firstOperand = Call->getOperand(0);

                        bool isArgCall = false;

                        Function *recFunc;
                        if (dyn_cast<Argument>(firstOperand)) {
                            LOG_LINE("Fist OP is an argument");
                            Function* callingFunction = Call->getFunction();
                            if (callingFunction) {
                                recFunc = calcArgs(*callingFunction); 
                                LOG_LINE("RETURNED RECURSION");
                                isArgCall = true;
                            }
                        }
                        

                        

                        LOG_LINE("IF STATEMENT FOR THE CALL CHANGING");

                        SmallVector<Value*, 8> NewCallArgs;

                        Value *sizeValue;
                        
                        CallInst *processedCall;

                        if (isArgCall)
                        {
                            processedCall = findAndProcessCalls(*recFunc, Call);
                            for (Value *Operand : processedCall->operands()) {
                                LOG_LINE("This is operand iterator");

                                NewCallArgs.push_back(Operand);

                                sizeValue = getNextOperand(&(*recFunc->arg_begin()), recFunc);
                                break;
                            }
                        } else {
                            for (Value *Operand : Call->operands()) {
                                LOG_LINE("This is operand iterator");

                                NewCallArgs.push_back(Operand);

                                LOG_LINE("before checking the alloc if statement");
                                if (AllocaInst *Alloca = dyn_cast<AllocaInst>(Operand)) {
                                    LOG_LINE("ENTERED ARRAY LOOKUP");
                                    sizeValue = Alloca->getArraySize();

                                    break;
                                }
                            }
                        }


                        NewCallArgs.push_back(sizeValue);

                        LOG_LINE("exited for look for vals statement");

                        Function *CalledFunction = Call->getCalledFunction();
                        LOG_LINE("THIS IS THE NAME: " << CalledFunction->getName() << " AND THIS " << F.getName());
                        if (CalledFunction->getName() != F.getName())
                        {
                            break;
                        }

                        unsigned int numArgsFunc = NewFunc->arg_size();
                        unsigned int numArgsCall = NewCallArgs.size();
                        LOG_LINE("Number of arguments in NewFunc: " << numArgsFunc << " this is the size of the call: " << numArgsCall);
                        
                        FunctionType* newFuncType = NewFunc->getFunctionType();
                        FunctionType* calledFuncType = Call->getFunctionType();

                        LOG_LINE("NewFunc Type: " << *newFuncType);
                        LOG_LINE("Called Function Type: " << *calledFuncType);

                        LOG_LINE("hello world");

                        CallInst *NewCall;
                        if(isArgCall) {
                            NewCall = CallInst::Create(NewFunc, NewCallArgs, "", processedCall);
                        }
                        else {
                            NewCall = CallInst::Create(NewFunc, NewCallArgs, "", Call);
                        }
                        LOG_LINE("yes");
                        Call->replaceAllUsesWith(NewCall);
                        LOG_LINE("yes");

                        Call->eraseFromParent();
                        deletedFucntions.push_back(&F);
                        addedFunctions.push_back(NewFunc);
                        LOG_LINE("returned newly made fucntion");
                        return NewFunc;

                        
                    }
                }
                                    

            }
        }


    }
    return NULL;
}


bool BoundsChecker::runOnModule(Module &M) {
    
    LLVMContext &C = M.getContext();
    Type *VoidTy = Type::getVoidTy(C);
    Type *Int32Ty = Type::getInt32Ty(C);
    one = ConstantInt::get(Type::getInt32Ty(M.getContext()), 1);
    auto FnCallee = M.getOrInsertFunction("__coco_check_bounds",
                                          VoidTy, Int32Ty, Int32Ty);
    boundsCheckCall = cast<Function>(FnCallee.getCallee());

     bool Changed = false;

     for (Function &F : M) {
        if (!shouldInstrument(&F))
            continue;

        LOG_LINE("Visiting function " << F.getName());

        Function *NewFunc = calcArgs(F);
        
        if (isChanged)
        {
            Changed |= instrumentGeps(*NewFunc);
            LOG_LINE("FUCNTION EXITED " << NewFunc->getName());

        } else {
            Changed |= instrumentGeps(F);
            LOG_LINE("FUCNTION EXITED " << F.getName());
        }
        isChanged = false;
    }

    return Changed;
}

char BoundsChecker::ID = 0;
static RegisterPass<BoundsChecker> X("coco-boundscheck", "Bounds checker pass");

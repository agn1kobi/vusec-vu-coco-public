/**
 * Bounds checker implementation
*/

#define DEBUG_TYPE "BoundsChecker"
#define EXPECTED_BLOCK_NUMS 16
#include "utils.h"

using namespace llvm;

namespace {
    class BoundsChecker : public ModulePass {
    public:
        static char ID;
        BoundsChecker() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

    private:
       Function *boundsCheckCall;

        bool instrumentGeps(Function &F);
        Value *gepOffsetAccumulator(Value *V, IRBuilder<> &B);
        Value *determineArraySize(Value *V, IRBuilder<> &B);
        //SmallVector<Value *, EXPECTED_BLOCK_NUMS> Visited;
        DenseMap<Instruction *, Value *> allocaSize;
        Value *one;
    };
}

Value *BoundsChecker::determineArraySize(Value *V, IRBuilder<> &B) {
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
        return B.getInt32(1);
        }
    }
    else if(isa<LoadInst>(V)) {
        report_fatal_error("Error: goon load");
    }
    else if(isa<GetElementPtrInst>(V)) {
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
        return determineArraySize(GEP->getPointerOperand(), B);
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

    for (Instruction &II : instructions(F)) {
        Instruction *I = &II;
        if(isa<AllocaInst>(I)) {
            //AllocaInst *AI = dyn_cast<AllocaInst>(I);
            
        }
        if(isa<CallInst>(I)) {
           // CallInst *CI = dyn_cast<CallInst>(I);
            LOG_LINE("CALL FOUNDDD");
            //Function *func = CI->getCalledFunction();
            
        }
        if (isa<GetElementPtrInst>(I)) {
            GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I);
            LOG_LINE("GEP FOUNDDD");


            if (GlobalVariable *GV = dyn_cast<GlobalVariable>(GEP->getPointerOperand())) {
                if (GV->hasInitializer() && GV->getType()->getElementType()->isArrayTy()) {
                    LOG_LINE(" THIS IS A GLOBAL STRING BOUNDS CHEKING NOT NEEDED");
                    continue; 
                }
            }

            Value *offset = gepOffsetAccumulator(GEP, B);
            Value *arraySize = determineArraySize(GEP->getOperand(0), B);

            B.SetInsertPoint(GEP);
            B.CreateCall(boundsCheckCall, {offset, arraySize} );
            changed = true;
        }
    }
    return changed;
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
        Changed |= instrumentGeps(F);
    }

    return Changed;
}

char BoundsChecker::ID = 0;
static RegisterPass<BoundsChecker> X("coco-boundscheck", "Bounds checker pass");

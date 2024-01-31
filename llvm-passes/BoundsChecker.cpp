#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
    class BoundsChecker : public ModulePass {
    public:
        static char ID;
        BoundsChecker() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

    private:
        Function *BoundsCheckFunc;

        bool insertBoundsCheck(Instruction *I);
        Value* getAccumulatedOffset(GetElementPtrInst *GEP);
    };
}

bool BoundsChecker::insertBoundsCheck(Instruction *I) {
    if (isa<GetElementPtrInst>(I)) {
        IRBuilder<> B(I);

        // Get the accumulated offset for the GEP instruction
        Value *Offset = getAccumulatedOffset(cast<GetElementPtrInst>(I));

        // Create a new call to the bounds check function, with the offset as an argument
        B.CreateCall(BoundsCheckFunc, { Offset });

        return true;
    }
    return false;
}

Value* BoundsChecker::getAccumulatedOffset(GetElementPtrInst *GEP) {
    // Accumulate the offset recursively for GEP instructions
    Value *Offset = nullptr;

    // Iterate over the indices of the GEP instruction
    for (unsigned i = 1; i < GEP->getNumOperands(); ++i) {
        Value *Index = GEP->getOperand(i);

        // If the index is a constant, add it to the accumulated offset
        if (ConstantInt *CI = dyn_cast<ConstantInt>(Index)) {
            if (!Offset)
                Offset = CI;
            else
                Offset = BinaryOperator::CreateAdd(Offset, CI);
        } else {
            // If the index is not a constant, we can't determine the offset statically
            // In a more complete implementation, you would handle dynamic offsets here.
            // For now, just return a constant 0.
            LLVMContext &C = GEP->getContext();
            Offset = ConstantInt::get(Type::getInt32Ty(C), 0);
            break;
        }
    }

    return Offset;
}

bool BoundsChecker::runOnModule(Module &M) {
    LLVMContext &C = M.getContext();
    Type *VoidTy = Type::getVoidTy(C);
    Type *Int32Ty = Type::getInt32Ty(C);

    // Insert or get the bounds check function
    auto FnCallee = M.getOrInsertFunction("bounds_check_function", VoidTy, Int32Ty);
    BoundsCheckFunc = cast<Function>(FnCallee.getCallee());

    bool Changed = false;

    for (Function &F : M) {
        for (Instruction &I : instructions(F)) {
            Changed |= insertBoundsCheck(&I);
        }
    }

    return Changed;
}

char BoundsChecker::ID = 0;
static RegisterPass<BoundsChecker> X("coco-boundscheck", "Bounds checker pass");

/**
 * Bounds Checker implementation
*/

#define DEBUG_TYPE "BoundsCheckerPass"
#include "utils.h"

namespace {
    class BoundsCheckerPass : public ModulePass {
    public:
        static char ID;
        BoundsCheckerPass() : ModulePass(ID) {}
        virtual bool runOnModule(Module &M) override;

    private:
        Function *PrintAllocFunc;

        bool instrumentAllocations(Function &F);
    };
}

char BoundsCheckerPass::ID = 0;
static RegisterPass<BoundsCheckerPass> X("coco-dummymodulepass", "Example LLVM module pass that inserts prints for every allocation.");

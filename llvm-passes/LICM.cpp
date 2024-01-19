/**
 * LICM implementation
*/

#define DEBUG_TYPE "LICMPass"
#include "utils.h"

namespace {
    class LICMPass : public LoopPass {
    public:
        static char ID;
        LICMPass() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        //void getAnalysisUsage(AnalysisUsage &AU) const override;
    };
}

bool LICMPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    return false;
}

// void LICMPass::getAnalysisUsage(AnalysisUsage &AU) {
//     return;
// }

char LICMPass::ID = 0;
RegisterPass<LICMPass> X("coco-licm", "LLVM LICM Pass");
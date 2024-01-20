/**
 * LICM implementation
*/

#define DEBUG_TYPE "LICMPass"
#define EXPECTED_BLOCK_NUMS 16
#include "utils.h"

namespace {
    class LICMPass : public LoopPass {
    public:
        static char ID;
        LICMPass() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        // void getAnalysisUsage(AnalysisUsage &AU) const override;
    private:
        bool isNotInSubLoop(Loop *L, LoopInfo *LI, BasicBlock *BB);
        bool isInvariant(Instruction *I, Loop *L, LoopInfo *LI);
        bool isComputedOutsideLoop(Use *U, Loop *L, LoopInfo *LI);
        bool isSafeToHoist(Instruction *I, Loop *L);
        bool completeDominator(Instruction *I, Loop *L);
    };
}

bool LICMPass::completeDominator(Instruction *I, Loop *L) {
    bool ret = false;
    BasicBlock *dominator = I->getParent();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    SmallVector<BasicBlock *, EXPECTED_BLOCK_NUMS> blocks;
    L->getExitBlocks(blocks);
    for(BasicBlock *BB : blocks) {
        if (DT->dominates(dominator, BB)) ret = true; else ret = false;
    }
    return ret;
}

bool LICMPass::isSafeToHoist(Instruction *I, Loop *L) {
    if(!(I->mayHaveSideEffects()) || completeDominator(I, L)) return true; else return false;
}

bool LICMPass::isComputedOutsideLoop(Use *U, Loop *L, LoopInfo *LI) {
    if(Instruction *I = dyn_cast<Instruction>(U)) {
        BasicBlock *BB = I->getParent();
        for(BasicBlock *basicBlock : L->blocks()) {
            if(BB == basicBlock) {
                return false;
            }
        }
        return true;
    }
    else {
        return false;
    }
}

bool LICMPass::isInvariant(Instruction *I, Loop *L, LoopInfo *LI) {
    bool ret = true;
    if(!I->isBinaryOp() && !I->isShift() && !isa<SelectInst>(I) && !I->isCast() && !isa<GetElementPtrInst>(I)) {
        ret = false;
        return ret;
    }
    for(Use &U: I->operands()) {
        if(isa<Constant>(U) || isComputedOutsideLoop(&U, L, LI)) ret = true; else ret = false;
    }
    return ret;
}

bool LICMPass::isNotInSubLoop(Loop *L, LoopInfo *LI, BasicBlock *BB) {
    Loop *loop = LI->getLoopFor(BB);
    LOG_LINE("checking if in subloop\n");
    if(loop == L) return true; else return false;
}

// void LICMPass::getAnalysisUsage(AnalysisUsage &AU) const {
//     // Tell LLVM we need some analysis info which we use for analyzing the
//     // DominatorTree.
//     AU.setPreservesCFG();
//     AU.addRequired<LoopInfoWrapperPass>();
//     getLoopAnalysisUsage(AU);
// }

bool LICMPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    bool ret = false;
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    for(BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB)) {
            LOG_LINE("The loop header dominates this Basic Block\n");
            LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            if(isNotInSubLoop(L, LI, BB)) {
                LOG_LINE("The basic block is not inside a subloop\n");
                for(auto i = BB->begin(), e = BB->end(); i != e; i++) {
                    Instruction *I = dyn_cast<Instruction>(i);
                    LOG_LINE("The instruction " << *I <<" is being checked for invariance and hoist safety\n");
                    if(isInvariant(I, L, LI) && isSafeToHoist(I, L)) {
                        LOG_LINE("The instruction " << *I <<" is invariant and safe to hoist\n");
                        BasicBlock *firstBlock = L->getLoopPreheader();
                        Instruction *preHIns = firstBlock->getTerminator();
                        I->moveBefore(preHIns);
                    }
                }
            }
        }
    }
    return ret;
}


char LICMPass::ID = 0;
RegisterPass<LICMPass> X("coco-licm", "LLVM LICM Pass");
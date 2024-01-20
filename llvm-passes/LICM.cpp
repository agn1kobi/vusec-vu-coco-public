/**
 * LICM implementation
*/

#define DEBUG_TYPE "LICMPass"
#define EXPECTED_BLOCK_NUMS 16
#include "utils.h"

using namespace std;

namespace {
    class LICMPass : public LoopPass {
    public:
        static char ID;
        LICMPass() : LoopPass(ID) {}
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override;
        void getAnalysisUsage(AnalysisUsage &AU) const override;
    private:
        bool isNotInSubLoop(Loop *L, LoopInfo *LI, BasicBlock *BB);
        bool isInvariant(Instruction *I, Loop *L, LoopInfo *LI);
        bool isComputedOutsideLoop(Use *U, Loop *L, LoopInfo *LI);
        bool isSafeToHoist(Instruction *I, Loop *L);
        bool completeDominator(Instruction *I, Loop *L);
    };
}

bool LICMPass::completeDominator(Instruction *I, Loop *L) {
    bool ret = true;
    BasicBlock *dominator = I->getParent();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    SmallVector<BasicBlock *, EXPECTED_BLOCK_NUMS> blocks;
    L->getExitBlocks(blocks);
    for(BasicBlock *BB : blocks) {
        if (!DT->dominates(dominator, BB)) return false;
    }
    return ret;
}

bool LICMPass::isSafeToHoist(Instruction *I, Loop *L) {
    if(!(I->mayHaveSideEffects()) || completeDominator(I, L)) return true; else return false;
}

bool LICMPass::isComputedOutsideLoop(Use *U, Loop *L, LoopInfo *LI) {
    LOG_LINE("checking for outside computation");
    if(Instruction *I = dyn_cast<Instruction>(U)) {
        BasicBlock *BB = I->getParent();
        for(BasicBlock *basicBlock : L->blocks()) {
            if(BB == basicBlock) {
                return false;
                
            }
        }
        LOG_LINE(*I << "was computed outside");
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
        if(!(isa<Constant>(U) || isComputedOutsideLoop(&U, L, LI))) return false;
    }
    return ret;
}

bool LICMPass::isNotInSubLoop(Loop *L, LoopInfo *LI, BasicBlock *BB) {
    Loop *loop = LI->getLoopFor(BB);
    LOG_LINE("checking if in subloop");
    if(loop == L) return true; else return false;
}

void LICMPass::getAnalysisUsage(AnalysisUsage &AU) const {
    // Tell LLVM we need some analysis info which we use for analyzing the
    // DominatorTree.
    AU.setPreservesCFG();
    AU.addRequired<LoopInfoWrapperPass>();
    getLoopAnalysisUsage(AU);
}

bool LICMPass::runOnLoop(Loop *L, LPPassManager &LPM) {
    bool ret = false;
    BasicBlock *Header = L->getHeader();
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for(BasicBlock *BB : L->blocks()) {
        if (DT->dominates(Header, BB)) {
            LOG_LINE("The loop header dominates this Basic Block");
            if(isNotInSubLoop(L, LI, BB)) {
                LOG_LINE("The basic block is not inside a subloop");
                BasicBlock::iterator iter = BB->begin();
                while(!(iter == BB->end())) {

                    Instruction *I = dyn_cast<Instruction>(iter);
                    iter++;
                    if(I) {
                    LOG_LINE("The instruction " << *I <<" is being checked for invariance and hoist safety");
                        if(isInvariant(I, L, LI) && isSafeToHoist(I, L)) {
                            LOG_LINE("The instruction " << *I <<" is invariant, safe to hoist and marked for movement");
                            BasicBlock *firstBlock = L->getLoopPreheader();
                            Instruction *preHIns = firstBlock->getTerminator();
                            I->moveBefore(preHIns);
                            ret = true;
                            LOG_LINE("The instruction " << *I << "was in a hoist attempt");
                        }
                    }
                }
            }
        }
    }
    
    return ret;
}


char LICMPass::ID = 0;
RegisterPass<LICMPass> X("coco-licm", "LLVM LICM Pass");
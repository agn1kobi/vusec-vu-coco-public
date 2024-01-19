/**
 * ADCE implementation
 */

#define DEBUG_TYPE "ADCEpass"
#define EXPECTED_BLOCK_NUMS 16
#include "utils.h"

using namespace std;

namespace {
    class ADCEPass : public FunctionPass {
    public:
        static char ID;
        ADCEPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    private:
        df_iterator_default_set<BasicBlock* > reachableBlocks;
        SmallVector<Instruction *, EXPECTED_BLOCK_NUMS> worklist;
        SmallVector<Instruction *, EXPECTED_BLOCK_NUMS> deletelist;
        SmallVector<Instruction *, EXPECTED_BLOCK_NUMS> keysToRemove;
        bool isTriviallyLive(Instruction *instruction);
    };
}

bool ADCEPass::isTriviallyLive(Instruction *instruction) {
    if(instruction->isTerminator() || isa<CallInst>(instruction) || isa<StoreInst>(instruction) || instruction->mayHaveSideEffects() || (isa<LoadInst>(instruction) && dyn_cast<LoadInst>(instruction)->isVolatile())) return true;
    else return false;
}

bool ADCEPass::runOnFunction(Function &F) {
    bool ret = false;
    DenseMap<Instruction *, bool> liveSet;
    for(BasicBlock *basicBlock : depth_first_ext(&F, reachableBlocks)) {
        for(Instruction &ins : *basicBlock) {
            Instruction *instruction = &ins;
            if(isTriviallyLive(instruction)) {
                LOG_LINE("instruction" << *instruction << "has been marked as trivially live");
                worklist.push_back(instruction);
                liveSet.insert(make_pair(instruction, true));
            }
            else if(instruction->use_empty()){
                LOG_LINE("instruction" << *instruction << "is trivially dead and has to be deleted");
                deletelist.push_back(instruction);
                liveSet.insert(make_pair(instruction, false));
                ret = true;
            }
        }
    }
    while(!deletelist.empty()) {
        Instruction *instruction = deletelist.pop_back_val();
        BasicBlock *basicBlock = instruction->getParent();
        if(reachableBlocks.count(basicBlock) > 0) {
            for(Use &use: instruction->operands()) {
                if(Instruction *ins = dyn_cast<Instruction>(use)) {
                    if(liveSet.count(ins) == 0) {
                        LOG_LINE("instruction" << *ins << "has been marked dead because is being used by a dead instructions and set to delete");
                        deletelist.push_back(ins);
                        liveSet.insert(make_pair(ins, false));
                    }
                    else if(liveSet[ins] == false) {
                        //do nothing
                    }
                }
            }
        }
    }
    for (auto &entry : liveSet) {
        if (!entry.second) {
            keysToRemove.push_back(entry.first);
        }
    }
    for (auto &key : keysToRemove) {
        key->dropAllReferences();
    }
    for (auto &key : keysToRemove) {
        key->eraseFromParent();
        liveSet.erase(key);
    }
    keysToRemove.clear();
    while(!worklist.empty()) {
        Instruction *instruction = worklist.pop_back_val();
        BasicBlock *basicBlock = instruction->getParent();
        if(reachableBlocks.count(basicBlock) > 0) {
            for(Use &use: instruction->operands()) {
                if(Instruction *ins = dyn_cast<Instruction>(use)) {
                    if(liveSet.count(ins) == 0) {
                    LOG_LINE("instruction" << *ins << "has been marked live and put in liveSet");
                    worklist.push_back(ins);
                    liveSet.insert(make_pair(ins, true));
                    }
                    else if(liveSet[ins]) {
                        //do nothing
                    }
                }
            }
        }
    }
    for (auto &entry : liveSet) {
        if (!entry.second) {
            keysToRemove.push_back(entry.first);
        }
    }
    for (auto &key : keysToRemove) {
        key->dropAllReferences();
    }
    for (auto &key : keysToRemove) {
        key->eraseFromParent();
        liveSet.erase(key);
    }
    return ret;
}

char ADCEPass::ID = 0;
static RegisterPass<ADCEPass> X("coco-adce", "LLVM ADCE pass");
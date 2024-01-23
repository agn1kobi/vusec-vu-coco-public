/**
 * ConstProp implementation
*/

#define DEBUG_TYPE "ConstPropPass"
#include "utils.h"

using namespace std;
using namespace llvm;
namespace {
    class ConstPropPass : public FunctionPass {
    public:
        static char ID;
        ConstPropPass() : FunctionPass(ID) {}
        virtual bool runOnFunction(Function &F) override;
    private:
        df_iterator_default_set<BasicBlock* > reachableBlocks;
        bool FixedPointROF(Function &F, int *updated);
    };
}

bool ConstPropPass::FixedPointROF(Function &F, int *updated) {
    LOG_LINE("Visiting function " << F.getName());
    bool ret = false;
    for (BasicBlock *BB : depth_first_ext(&F, reachableBlocks)) {
        for (Instruction &II : *BB) {
            Instruction *I = &II;
            LOG_LINE("Checking instruction " << *I);
            if(isa<BinaryOperator>(I)) {
                if(!(I->getOperand(0)->getName() == "%i.0" ) && !(I->getOperand(1)->getName() == "%i.0")  ) {
                    LOG_LINE("instruction " << *I << " is a binary operator");
                    BinaryOperator *B = dyn_cast<BinaryOperator>(I);
                    if(isa<ConstantInt>(B->getOperand(0)) && isa<ConstantInt>(B->getOperand(1))) {
                        ConstantInt *integer_a = dyn_cast<ConstantInt>(B->getOperand(0));
                        ConstantInt *integer_b = dyn_cast<ConstantInt>(B->getOperand(1));
                        LOG_LINE("both " << *integer_a << " and " << *integer_b << " are constants");
                        APInt val;
                        if(B->getOpcode() == Instruction::Add) {
                            val = integer_a->getValue() + integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::Mul) {
                            val = integer_a->getValue() * integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::Sub) {
                            val = integer_a->getValue() - integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::Shl) {
                            val = integer_a->getValue() << integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::AShr) {
                            val = integer_a->getValue().ashr(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::LShr) {
                            val = integer_a->getValue().lshr(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::UDiv) {
                            val = integer_a->getValue().udiv(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::SDiv) {
                            val = integer_a->getValue().sdiv(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::URem) {
                            val = integer_a->getValue().urem(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::SRem) {
                            val = integer_a->getValue().srem(integer_b->getValue());
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::And) {
                            val = integer_a->getValue() & integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::Or) {
                            val = integer_a->getValue() | integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                        else if(B->getOpcode() == Instruction::Xor) {
                            val = integer_a->getValue() ^ integer_b->getValue();
                            if(!B->use_empty()) {
                                ConstantInt *vall = ConstantInt::get(F.getContext(), val);
                                B->replaceAllUsesWith(vall);
                                ret = true;
                                (*updated)++;
                            }
                        }
                    }
                    else if(isa<ConstantInt>(B->getOperand(0)) || isa<ConstantInt>(B->getOperand(1))) {
                        if(isa<ConstantInt>(B->getOperand(0))) {
                            if( B->getOpcode() == Instruction::Add && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(1));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Sub && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    Value *var1 = B->getOperand(1);
                                    Value *neg = IRBuilder<>(B).CreateNeg(var1);
                                    B->replaceAllUsesWith(neg);
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Mul && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::UDiv && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::SDiv && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::URem && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::SRem && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Shl && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::AShr && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::LShr && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::And && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Or && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(1));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Xor && dyn_cast<ConstantInt>(B->getOperand(0))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(1));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Mul && dyn_cast<ConstantInt>(B->getOperand(0))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(1));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::URem && dyn_cast<ConstantInt>(B->getOperand(0))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 1));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                        }
                        else if(isa<ConstantInt>(B->getOperand(1))) {
                            if( B->getOpcode() == Instruction::Add && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Sub && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Mul && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Shl && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::LShr && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::AShr && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::And && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Xor && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Or && dyn_cast<ConstantInt>(B->getOperand(1))->isZero()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::Mul && dyn_cast<ConstantInt>(B->getOperand(1))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::UDiv && dyn_cast<ConstantInt>(B->getOperand(1))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::SDiv && dyn_cast<ConstantInt>(B->getOperand(1))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(B->getOperand(0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::URem && dyn_cast<ConstantInt>(B->getOperand(1))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                            else if( B->getOpcode() == Instruction::SRem && dyn_cast<ConstantInt>(B->getOperand(1))->isOne()) {
                                if(!B->use_empty()) {
                                    B->replaceAllUsesWith(ConstantInt::get(B->getType(), 0));
                                    ret = true;
                                    (*updated)++;
                                }
                            }
                        }
                    }
                    else if (B->getOperand(0) == B->getOperand(1)) {
                    if (B->getOpcode() == Instruction::Sub) {
                        B->replaceAllUsesWith(ConstantInt::get(B->getContext(), APInt(32, 0)));
                        ret = true;
                        (*updated)++;
                    }
                }
                }
            }
        }
    }
    return ret;
}

bool ConstPropPass::runOnFunction(Function &F) {
    bool run = false;
    int updated = -1;
    do {
        run = FixedPointROF(F, &updated);
    } while(run);
    if(updated != -1) return true; else return false;
}

char ConstPropPass::ID = 0;
static RegisterPass<ConstPropPass> X("coco-constprop", "Constant Propagation pass");
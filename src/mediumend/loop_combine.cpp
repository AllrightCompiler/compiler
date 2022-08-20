#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Instruction;
using ir::Loop;
using ir::Reg;

using std::unordered_set;
using std::unordered_map;

struct if_cond{
  BasicBlock *cond_bb;
  BasicBlock *true_bb;
  BasicBlock *false_bb;
  BasicBlock *target;
  Reg val;
};

void if_combine(Function *func){
  unordered_map<BasicBlock *, if_cond> bb2ifcond;
  for(auto &bb : func->bbs){
    TypeCase(br, ir::insns::Branch *, bb->insns.back().get()){
      auto &t_succ = br->true_target->succ;
      if(t_succ.size() != 1){
        continue;
      }
      auto next_bb = *t_succ.begin();
      if(br->false_target != next_bb){
        if(br->false_target->succ.size() != 1){
          continue;
        }
        auto false_next_bb = *br->false_target->succ.begin();
        if(false_next_bb != next_bb){
          continue;
        }
      }
      if_cond cond;
      cond.cond_bb = bb.get();
      cond.true_bb = br->true_target;
      cond.false_bb = br->false_target;
      cond.target = next_bb;
      cond.val = br->val;
      bb2ifcond[bb.get()] = cond;
    }
  }
  for(auto &each : bb2ifcond){
    auto &cond = each.second;
    if(bb2ifcond.count(cond.target) == 0){
      continue;
    }
    auto next = bb2ifcond.at(cond.target);
    if(cond.val != next.val){
      continue;
    }
    bool check = true;
    for(auto &inst : next.true_bb->insns){
      auto uses = inst->use();
      for(auto use : uses){
        if(!func->has_param(use)){
          auto def_bb = func->def_list.at(use)->bb;
          if(def_bb == next.cond_bb){
            check = false;
          }
        }
      }
    }
    if(next.false_bb != next.target){
      for(auto &inst : next.false_bb->insns){
        auto uses = inst->use();
        for(auto use : uses){
          if(!func->has_param(use)){
            auto def_bb = func->def_list.at(use)->bb;
            if(def_bb == next.cond_bb){
              check = false;
            }
          }
        }
      }
    }
    if(next.cond_bb->insns.size() != 1){
      continue;
    }
    if(!check){
      continue;
    }
    each.second.true_bb->pop_back();
    for(auto &inst : next.true_bb->insns){
      inst->bb = each.second.true_bb;
    }
    each.second.true_bb->insns.splice(each.second.true_bb->insns.end(), next.true_bb->insns);
  }
}

void if_combine(ir::Program *prog){
  for(auto &[_, func] : prog->functions){
    if_combine(&func);
  }
}

};
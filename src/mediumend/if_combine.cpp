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

static Function * cur_func = nullptr;

struct if_cond{
  BasicBlock *cond_bb;
  BasicBlock *true_bb;
  BasicBlock *false_bb;
  BasicBlock *target;
  Reg val;
};

static unordered_set<Instruction *> get_effective_use(Instruction *inst,
                                               BasicBlock *bb) {
  unordered_set<Instruction *> ret;
  unordered_set<Instruction *> visited;
  vector<Instruction *> stack;
  stack.push_back(inst);
  while (stack.size()) {
    inst = stack.back();
    stack.pop_back();
    if (visited.count(inst) || inst->bb != bb) {
      continue;
    }
    visited.insert(inst);
    TypeCase(output, ir::insns::Output *, inst) {
      auto uses = cur_func->use_list[output->dst];
      for (auto each : uses) {
        stack.push_back(each);
      }
    }
    TypeCase(store, ir::insns::Store *, inst) { ret.insert(store); }
    TypeCase(memdef, ir::insns::MemDef *, inst) { ret.insert(memdef); }
    TypeCase(call, ir::insns::Call *, inst) { ret.insert(call); }
  }
  return ret;
}

static bool check_common_var(BasicBlock *b1, BasicBlock *b2, BasicBlock *mid_bb, BasicBlock *fb1, BasicBlock *fb2) {
  unordered_map<Reg, Reg> phi_map_true;
  unordered_map<Reg, Reg> phi_map_false;
  BasicBlock *false_bb = fb1;
  if(fb1 == mid_bb){
    false_bb = fb1->idom;
  }
  for(auto &inst : mid_bb->insns){
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      phi_map_true[phi->dst] = phi->incoming.at(b1);
      phi_map_false[phi->dst] = phi->incoming.at(false_bb);
      for(auto use : cur_func->use_list[phi->dst]){
        if(use->bb != b2 && use->bb != fb2){
          return false;
        }
      }
    } else {
      TypeCase(br, ir::insns::Branch *, inst.get()){
      } else {
        return false;
      }
    }
  }
  for(auto &reg2reg : phi_map_true){
    auto use_list = cur_func->use_list[reg2reg.first];
    for(auto each : use_list){
      if(each->bb == b2){
        each->change_use(reg2reg.first, reg2reg.second);
      }
    }
  }
  for(auto &reg2reg : phi_map_false){
    auto use_list = cur_func->use_list[reg2reg.first];
    for(auto each : use_list){
      if(each->bb == fb2){
        each->change_use(reg2reg.first, reg2reg.second);
      }
    }
  }
  return true;
}

bool if_combine(Function *func){
  cur_func = func;
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
    bool check = check_common_var(each.second.true_bb, next.true_bb, next.cond_bb, each.second.false_bb, next.false_bb);
    if(!check){
      continue;
    }
    BasicBlock * false_prev = each.second.false_bb;
    BasicBlock * false_prev_2 = next.false_bb;
    if(false_prev == next.cond_bb){
      false_prev = false_prev->idom;
    }
    if(false_prev_2 == next.target){
      false_prev_2 = false_prev_2->idom;
    }
    for(auto &inst : each.second.target->insns){
      inst->bb = next.target; 
      TypeCase(phi, ir::insns::Phi *, inst.get()){
        phi->incoming[next.true_bb] = phi->incoming.at(each.second.true_bb);
        phi->incoming[next.false_bb] = phi->incoming.at(each.second.false_bb);
      }
    }
    false_prev->change_succ(each.second.target, next.false_bb);
    next.false_bb->change_prev(false_prev_2, false_prev);

    each.second.target->prev.erase(false_prev);
    each.second.target->prev.erase(each.second.true_bb);

    each.second.true_bb->change_succ(each.second.target, next.true_bb);
    next.true_bb->change_prev(next.target, each.second.true_bb);
    for(auto &inst : next.cond_bb->insns){
      inst->remove_use_def();
    }
    for(auto iter = func->bbs.begin(); iter != func->bbs.end(); ++iter){
      if(iter->get() == next.cond_bb){
        func->bbs.erase(iter);
        break;
      }
    }
    return true;
  }
  return false;
}

void if_combine(ir::Program *prog){
  for(auto &[_, func] : prog->functions){
    while(if_combine(&func)){}
  }
}

};
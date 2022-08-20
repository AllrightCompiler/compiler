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

void fuse_if(if_cond cond_1, if_cond cond_2) {
  auto b1 = cond_1.true_bb;
  auto b2 = cond_2.true_bb;
  auto mid_bb = b2->idom;
  unordered_map<Reg, Reg> phi_map;
  for (auto &inst : mid_bb->insns) {
    TypeCase(phi, ir::insns::Phi *, inst.get()) {
      phi_map[phi->dst] = phi->incoming.at(b1);
    }
  }

  unordered_map<Reg, Reg> reorder_map;
  BasicBlock *succ;
  for (auto each : b2->succ) {
    if (each == b2) {
      continue;
    }
    succ = each;
  }
  for (auto &inst : b2->insns) {
    inst->bb = b1;
    TypeCase(phi, ir::insns::Phi *, inst.get()) {
      if (!phi_map.count(phi->incoming.at(mid_bb))) {
        continue;
      }
      auto raw = phi_map.at(phi->incoming.at(mid_bb));
      auto dst = phi->dst;
      auto use_list = cur_func->use_list[dst];
      for(auto each : use_list){
        if(each->bb == b2){
          each->change_use(dst, raw);
        }
      }
      // check here
      reorder_map[raw] = phi->incoming.at(b2);
    }
  }
  for (auto &inst : b1->insns) {
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      auto uses = phi->use();
      for (auto each : uses) {
        if (reorder_map.count(each)) {
          inst->change_use(each, reorder_map.at(each));
        }
      }
    } else {
      break;
    }
  }
  auto iter = mid_bb->insns.begin();
  auto idom_prev = b1->idom;
  unordered_set<Reg> phi_defs;
  for (; iter != mid_bb->insns.end();) {
    TypeCase(phi, ir::insns::Phi *, iter->get()){
      auto uses = phi->use();
      for (auto each : uses) {
        if (reorder_map.count(each)) {
          phi->change_use(each, reorder_map.at(each));
        }
      }
      phi_defs.insert(phi->dst);
      iter++;
    } else {
      TypeCase(term, ir::insns::Terminator *, iter->get()){
        break;
      }
      bool move = true;
      auto uses = iter->get()->use();
      for (auto each : uses) {
        if (cur_func->def_list.count(each) && cur_func->def_list.at(each)->bb == mid_bb) {
          move = false;
        }
      }
      if(!move){
        iter++;
        continue;
      }
      auto inst = iter->release();
      inst->remove_use_def();
      inst->bb = idom_prev;
      idom_prev->insert_before_ter(inst);
      inst->add_use_def();
      iter = mid_bb->insns.erase(iter);
    }
  }
  iter = succ->insns.begin();
  for (; iter != succ->insns.end();) {
    TypeCase(phi, ir::insns::Phi *, iter->get()) {
      auto mid_reg = phi->incoming.at(mid_bb);
      if(cur_func->def_list.count(mid_reg) && cur_func->def_list.at(mid_reg)->bb == mid_bb){
        copy_propagation(cur_func->use_list, phi->dst, mid_reg);
        iter->get()->remove_use_def();
        iter = succ->insns.erase(iter);
        continue;
      }
      for (auto prev : b1->prev) {
        phi->incoming[prev] = phi->incoming.at(mid_bb);
      }
      phi->incoming[b1] = phi->incoming.at(b2);
      phi->bb = mid_bb;
      phi->incoming.erase(mid_bb);
      phi->incoming.erase(b2);
      phi->remove_use_def();
      phi->add_use_def();
    }
    else {
      break;
    }
    iter++;
  }
  mid_bb->insns.splice(mid_bb->insns.begin(), succ->insns,
                      succ->insns.begin(), iter);
  iter = b2->insns.begin();
  for (; iter != b2->insns.end(); iter++) {
    TypeCase(phi, ir::insns::Phi *, iter->get()) {
      auto change_reg = phi->incoming.at(b2);
      phi->incoming.erase(b2);
      phi->incoming[b1] = change_reg;
      auto raw_reg = phi->incoming.at(b2->idom);
      phi->incoming.erase(b2->idom);
      for (auto each : b1->prev) {
        if (each != b1) {
          phi->incoming[each] = raw_reg;
        }
      }
      phi->remove_use_def();
      phi->add_use_def();
    }
    else {
      break;
    }
  }
  auto br = b1->insns.back().release();
  b1->insns.pop_back();
  b1->insns.splice(b1->insns.begin(), b2->insns, b2->insns.begin(), iter);
  b1->insns.splice(b1->insns.end(), b2->insns, iter, b2->insns.end());
  b1->insns.back()->remove_use_def();
  b1->insns.pop_back();
  b1->insns.emplace_back(br);
  mid_bb->change_succ(b2, succ);
  succ->change_prev(b2, mid_bb);
  for (auto iter = cur_func->bbs.begin(); iter != cur_func->bbs.end();) {
    if (iter->get() == b2) {
      for (auto &inst : iter->get()->insns) {
        inst->remove_use_def();
      }
      iter = cur_func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
}

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

static bool check_common_var(BasicBlock *b1, BasicBlock *b2, BasicBlock *mid_bb) {
  unordered_map<Reg, Reg> phi_map;
  for(auto &inst : mid_bb->insns){
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      phi_map[phi->dst] = phi->incoming.at(b1);
    } else {
      TypeCase(br, ir::insns::Branch *, inst.get()){
      } else {
        return false;
      }
    }
  }
  for(auto &reg2reg : phi_map){
    auto use_list = cur_func->use_list[reg2reg.first];
    for(auto each : use_list){
      if(each->bb == b2){
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
      if(br->false_target != next_bb){
        continue;
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
    bool check = check_common_var(each.second.true_bb, next.true_bb, next.cond_bb);
    fuse_if(each.second, next);
    
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
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Instruction;
using ir::Loop;
using ir::Reg;

using std::unordered_map;

static Function *cur_func = nullptr;

struct LoopCond {
  bool is_const_bound;
  Reg start;
  Reg step;
  Reg end;
  Reg var;
  BinaryOp cmp_op;
  BinaryOp update_op;
};

bool is_cmp(ir::insns::Binary *binary) {
  switch (binary->op) {
  case BinaryOp::Eq:
  case BinaryOp::Geq:
  case BinaryOp::Gt:
  case BinaryOp::Leq:
  case BinaryOp::Lt:
  case BinaryOp::Neq:
    return true;
  default:
    return false;
  }
}

unordered_set<Instruction *> get_effective_use(Instruction *inst,
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
    TypeCase(memdef, ir::insns::MemDef *, inst) { ret.insert(memdef); }
    TypeCase(call, ir::insns::Call *, inst) { ret.insert(call); }
    TypeCase(memuse, ir::insns::MemUse *, inst) { ret.insert(memuse); }
  }
  return ret;
}

LoopCond get_loop_cond(Loop *loop, unordered_set<BasicBlock *> loop_bb) {
  LoopCond ret;
  ret.is_const_bound = false;
  if (loop_bb.size() != 1) {
    return ret;
  }
  unordered_set<Reg> defs;
  BasicBlock *header = loop->header;
  for (auto &inst : header->insns) {
    TypeCase(output, ir::insns::Output *, inst.get()) {
      defs.insert(output->dst);
    }
  }
  auto &inst = header->insns.back();
  TypeCase(br, ir::insns::Branch *, inst.get()) {
    if (!cur_func->def_list.count(br->val)) {
      return ret;
    }
    auto cond_inst = cur_func->def_list.at(br->val);
    TypeCase(binary, ir::insns::Binary *, cond_inst) {
      if (!is_cmp(binary)) {
        return ret;
      }
      if (defs.count(binary->src1) == defs.count(binary->src2)) {
        return ret;
      }
      ret.cmp_op = binary->op;
      Reg var;
      if (defs.count(binary->src1)) {
        var = binary->src1;
        ret.end = binary->src2;
      } else {
        var = binary->src2;
        ret.end = binary->src1;
      }
      auto update_inst = cur_func->def_list.at(var);
      TypeCase(update, ir::insns::Binary *, update_inst) {
        ret.update_op = update->op;
        if (defs.count(update->src1) == defs.count(update->src2)) {
          return ret;
        }
        Reg phi_reg;
        if (defs.count(update->src1)) {
          phi_reg = update->src1;
          ret.step = update->src2;
        } else {
          phi_reg = update->src2;
          ret.step = update->src1;
        }
        auto phi_inst = cur_func->def_list.at(phi_reg);
        Reg start;
        bool set_start = false;
        TypeCase(phi, ir::insns::Phi *, phi_inst) {
          ret.var = phi->dst;
          for (auto &in : phi->incoming) {
            if (in.first != header) {
              if (set_start) {
                if (start != in.second) {
                  return ret;
                }
              } else {
                set_start = true;
                start = in.second;
              }
            }
          }
          ret.start = start;
          ret.is_const_bound = true;
        }
        else {
          return ret;
        }
      }
      else {
        return ret;
      }
    }
    else {
      return ret;
    }
  }
  return ret;
}

void fuse_loops(Loop *to, Loop *from, LoopCond cond_1, LoopCond cond_2) {
  auto b1 = to->header;
  auto b2 = from->header;
  auto mid_bb = b2->idom;
  unordered_map<Reg, Reg> phi_map;
  for (auto &inst : mid_bb->insns) {
    TypeCase(phi, ir::insns::Phi *, inst.get()) {
      phi_map[phi->dst] = phi->incoming.at(b1);
    }
  }
  copy_propagation(cur_func->use_list, cond_2.var, cond_1.var);

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
    auto uses = inst->use();
    for (auto each : uses) {
      if (each == cond_2.var) {
        inst->change_use(each, cond_1.var);
      }
    }
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

bool check_common_var(BasicBlock *b1, BasicBlock *b2, BasicBlock *mid_bb) {
  unordered_map<Reg, Reg> common_map;
  unordered_map<Reg, Reg> used_map;
  unordered_set<Reg> common_set;
  for(auto &inst : b1->insns){
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      common_map[phi->dst] = phi->dst;
      common_map[phi->incoming.at(b1)] = phi->dst;
    }
    TypeCase(memdef, ir::insns::MemDef *, inst.get()){
      auto effect_inst = get_effective_use(memdef, b1);
      for(auto each_use : effect_inst){
        TypeCase(memuse, ir::insns::MemUse *, each_use){
          if(memuse->load_src != memdef->store_dst || memuse->dep != memdef->dep){
            return false;
          }
        } else TypeCase(use_memdef, ir::insns::MemDef *, each_use){ 
          if(memdef != use_memdef){
            return false;
          }
        } else {
          return false;
        }
      }
    }
    TypeCase(call, ir::insns::Call *, inst.get()){
      return false;
    }
    TypeCase(output, ir::insns::Output *, inst.get()){
      auto uses = inst->use();
      for(auto each : uses){
        if(common_map.count(each)){
          used_map[output->dst] = common_map.at(each);
        }
      }
    }
  }
  for(auto &inst : mid_bb->insns){
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      // 使用、改变，不能同时满足
      common_map[phi->dst] = common_map.at(phi->incoming.at(b1));
      for(auto each : cur_func->use_list[phi->dst]){
        TypeCase(use_phi, ir::insns::Phi *, each){
        } else {
          return false;
        }
      }
    }
    TypeCase(call, ir::insns::Call *, inst.get()){
      return false;
    }
  }
  for(auto &inst : b2->insns){
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      if(common_map.count(phi->incoming.at(mid_bb))){
        common_map[phi->dst] = common_map.at(phi->incoming.at(mid_bb));
        common_map[phi->incoming.at(b2)] = common_map.at(phi->incoming.at(mid_bb));
        common_set.insert(phi->incoming.at(b2));
      }
    }
    TypeCase(memdef, ir::insns::MemDef *, inst.get()){
      auto effect_inst = get_effective_use(memdef, b2);
      for(auto each_use : effect_inst){
        TypeCase(memuse, ir::insns::MemUse *, each_use){
          if(memuse->load_src != memdef->store_dst || memuse->dep != memdef->dep){
            return false;
          }
        } else TypeCase(use_memdef, ir::insns::MemDef *, each_use){ 
          if(memdef != use_memdef){
            return false;
          }
        } else {
          return false;
        }
      }
    }
    TypeCase(call, ir::insns::Call *, inst.get()){
      return false;
    }
    TypeCase(output, ir::insns::Output *, inst.get()){
      auto uses = inst->use();
      for(auto each : uses){
        if(common_map.count(each)){
          used_map[output->dst] = common_map.at(each);
        }
      }
    }
  }
  unordered_set<Reg> common_b1_set;
  for(auto each : common_set){
    TypeCase(phi, ir::insns::Phi *, cur_func->def_list.at(common_map.at(each))){
      common_b1_set.insert(phi->incoming.at(b1));
    } else {
      return false;
    }
  }
  common_set.merge(common_b1_set);
  for(auto each : common_set){
    auto inst = cur_func->def_list.at(each);
    TypeCase(memdef, ir::insns::MemDef *, inst){
      vector<Reg> stack;
      stack.push_back(memdef->store_val);
      while(stack.size()){
        auto cur_reg = stack.back();
        stack.pop_back();
        auto def_pos = cur_func->def_list.at(cur_reg);
        TypeCase(binary, ir::insns::Binary *, def_pos){
          if(binary->op == BinaryOp::Add || binary->op == BinaryOp::Mul){
            if(used_map.count(binary->src1) != common_map.count(binary->src2)){
              if(used_map.count(binary->src1)){
                stack.push_back(binary->src1);
              } else {
                stack.push_back(binary->src2);
              }
            } else {
              return false;
            }
          } else {
            return false;
          }
        } else TypeCase(memuse, ir::insns::MemUse *, def_pos){
          if(memuse->load_src != memdef->store_dst){
            return false;
          }
        } else {
          return false;
        }
      }
    } else {
      return false;
    }
  }
  for(auto &inst : mid_bb->insns){
    bool is_phi = false;
    bool is_const = false;
    TypeCase(phi, ir::insns::Phi *, inst.get()){
      // 使用、改变，不能同时满足
      common_map[phi->dst] = common_map.at(phi->incoming.at(b1));
      for(auto each : cur_func->use_list[phi->dst]){
        TypeCase(use_phi, ir::insns::Phi *, each){
          is_phi = true;
        } else {
          is_const = true;
        }
      }
      if(is_const){
        auto use_list = cur_func->use_list[phi->dst];
        for(auto each : use_list){
          if(each->bb == b2){
            each->change_use(phi->dst, phi->incoming.at(b1));
          }
        }
      }
    }
  }
  return true;
}

void loop_fusion(Function *func) {
  cur_func = func;
  func->loop_analysis();
  unordered_map<Loop *, unordered_set<BasicBlock *>> loop_bbs;
  unordered_map<Loop *, LoopCond> loop_conds;
  for (auto &loop : func->loops) {
    vector<BasicBlock *> stack;
    stack.push_back(loop->header);
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      if (loop_bbs[loop.get()].count(bb)) {
        continue;
      }
      loop_bbs[loop.get()].insert(bb);
      for (auto &succ : bb->succ) {
        if (succ->loop == loop.get()) {
          stack.push_back(succ);
        }
      }
    }
  }
  for (auto &loop : func->loops) {
    if (loop->no_inner) {
      loop_conds[loop.get()] =
          get_loop_cond(loop.get(), loop_bbs.at(loop.get()));
    }
  }
  bool fused = false;
  for (int i = 1; i < func->loops.size(); i++) {
    if (fused) {
      break;
    }
    if ((!func->loops[i - 1]->no_inner) || (!func->loops[i]->no_inner)) {
      continue;
    }
    auto cond_1 = loop_conds[func->loops[i].get()];
    auto cond_2 = loop_conds[func->loops[i - 1].get()];
    if (cond_1.is_const_bound == false || cond_2.is_const_bound == false) {
      continue;
    }
    if (cond_1.cmp_op == cond_2.cmp_op && cond_1.end == cond_2.end &&
        cond_1.start == cond_2.start && cond_1.step == cond_2.step &&
        cond_1.update_op == cond_2.update_op) {
      auto loop_1 = func->loops[i].get();
      auto loop_2 = func->loops[i - 1].get();
      if (loop_1->outer != loop_2->outer) {
        continue;
      }
      auto mid_bb = loop_2->header->idom;
      if (!loop_1->header->succ.count(mid_bb)) {
        continue;
      }
      unordered_set<Reg> mid_phi_defs;
      unordered_set<Reg> loop_1_defs;
      unordered_set<Reg> loop_2_uses;
      // 添加判断机制
      auto b1 = loop_1->header;
      auto b2 = loop_2->header;
      bool check = check_common_var(b1, b2, mid_bb);
      if(!check){
        continue;
      }
      fuse_loops(loop_1, loop_2, cond_1, cond_2);
      fused = true;
    }
  }
}

void check_func_bb_inst(Function * func){
  for(auto &bb : func->bbs){
    for(auto &inst : bb->insns){
      assert(inst->bb == bb.get());
    }
  }
}

void loop_fusion(ir::Program *prog) {
  for (auto &each : prog->functions) {
    loop_fusion(&each.second);
    check_func_bb_inst(&each.second);
  }
}

} // namespace mediumend
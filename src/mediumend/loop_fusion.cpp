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
    TypeCase(store, ir::insns::Store *, inst) { ret.insert(store); }
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

void fuse_loops(Loop *to, Loop *from, LoopCond cond_1, LoopCond cond_2,
                unordered_map<Reg, Reg> &phi_map) {
  auto b1 = to->header;
  auto b2 = from->header;
  auto mid_bb = b2->idom;
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
      copy_propagation(cur_func->use_list, dst, raw);
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
  for (; iter != mid_bb->insns.end();) {
    TypeCase(phi, ir::insns::Phi *, iter->get()){
      auto uses = phi->use();
      for (auto each : uses) {
        if (reorder_map.count(each)) {
          phi->change_use(each, reorder_map.at(each));
        }
      }
      iter++;
    } else {
      TypeCase(term, ir::insns::Terminator *, iter->get()){
        break;
      }
      auto inst = iter->release();
      inst->bb = idom_prev;
      idom_prev->insert_before_ter(inst);
      iter = mid_bb->insns.erase(iter);
    }
  }
  iter = succ->insns.begin();
  for (; iter != succ->insns.end();) {
    TypeCase(phi, ir::insns::Phi *, iter->get()) {
      auto mid_reg = phi->incoming.at(mid_bb);
      if(cur_func->def_list.count(mid_reg) && cur_func->def_list.at(mid_reg)->bb == mid_bb){
        copy_propagation(cur_func->use_list, phi->dst, mid_reg);
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

      bool check = true;
      // 添加判断机制
      auto b1 = loop_1->header;
      auto b2 = loop_2->header;
      for(auto &inst : mid_bb->insns){
        bool is_phi = false;
        bool is_const = false;
        TypeCase(phi, ir::insns::Phi *, inst.get()){
          // 使用、改变，不能同时满足
          for(auto each : func->use_list[phi->dst]){
            TypeCase(use_phi, ir::insns::Phi *, each){
              is_phi = true;
            } else {
              is_const = true;
            }
          }
        }
        if(is_const && is_phi){
          check = false;
          break;
        }
      }
      if (!check) {
        continue;
      }
      unordered_map<Reg, Reg> phi_map;
      for (auto &inst : mid_bb->insns) {
        TypeCase(phi, ir::insns::Phi *, inst.get()) {
          phi_map[phi->dst] = phi->incoming.at(loop_1->header);
        }
      }
      copy_propagation(func->use_list, cond_2.var, cond_1.var);
      fuse_loops(loop_1, loop_2, cond_1, cond_2, phi_map);
      fused = true;
    }
  }
}

void loop_fusion(ir::Program *prog) {
  for (auto &each : prog->functions) {
    loop_fusion(&each.second);
  }
}

} // namespace mediumend
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;
using ir::Loop;

// Interchange loop:
// - one exit
// - one back edge to entry (exit_prev)
// - cond_var appear twice (phi, update)
struct InterchangeLoopInfo {
  bool valid;
  BasicBlock *entry;
  BasicBlock *exit_prev;
  BasicBlock *exit;
  ir::insns::Phi *ind_phi;
  ir::insns::Binary *ind_upd;
  ir::insns::Binary *ind_cond;
  ir::insns::Branch *ind_br;
};

struct TightlyNestedLoopInfo {
  bool valid;
  InterchangeLoopInfo inner, outer;

  TightlyNestedLoopInfo(InterchangeLoopInfo _inner, InterchangeLoopInfo _outer) : valid(false), inner(_inner), outer(_outer) {
    if (!inner.valid || !outer.valid) return;
    // check condition for tightly nested loops
    if (inner.entry->idom != outer.entry) return;
    if (outer.exit_prev != inner.exit) return;
    if (outer.entry->insns.size() != 2) return; // phi + br
    if (outer.entry->insns.front().get() != outer.ind_phi) return;
    auto &insns = outer.exit_prev->insns;
    if (insns.size() != 3) return; // upd + cmp + br
    auto find_inst = [&insns](ir::Instruction *inst) {
      for (auto &insn : insns) {
        if (insn.get() == inst) return true;
      }
      return false;
    };
    if (!find_inst(outer.ind_upd)) return;
    if (!find_inst(outer.ind_cond)) return;
    if (!find_inst(outer.ind_br)) return;
    valid = true;
  }

  bool profitable(Loop *inner_loop) { // detect a[j][i] and no a[i][j]
    auto in_loop = [inner_loop](BasicBlock *b) {
      Loop *l = b->loop;
      while (l != nullptr && l != inner_loop) {
        l = l->outer;
      }
      return l == inner_loop;
    };
    vector<BasicBlock *> stack;
    unordered_set<BasicBlock *> loop_bbs;
    stack.push_back(inner_loop->header);
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      if (in_loop(bb)) {
        loop_bbs.insert(bb);
        for (auto each : bb->dom) {
          stack.push_back(each);
        }
      }
    }
    bool order_found = false, reverse_found = false;
    Reg reg_i = outer.ind_phi->dst, reg_j = inner.ind_phi->dst;
    for (auto &bb : loop_bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(gep, ir::insns::GetElementPtr *, insn.get()) {
          // 1. gep(i, j)
          if (gep->indices.size() == 2) {
            if (gep->indices[0] == reg_i && gep->indices[1] == reg_j) order_found = true;
            if (gep->indices[0] == reg_j && gep->indices[1] == reg_i) reverse_found = true;
          }
          // 2. gep(i * n + j)
          if (gep->indices.size() == 1) {
            if (!gep->bb->func->has_param(gep->indices[0])) {
              TypeCase(binary_sum, ir::insns::Binary *, gep->bb->func->def_list.at(gep->indices[0])) {
                if (binary_sum->op == BinaryOp::Add) {
                  if (binary_sum->src1 == reg_i || binary_sum->src1 == reg_j) {
                    std::swap(binary_sum->src1, binary_sum->src2);
                  }
                  if (!binary_sum->bb->func->has_param(binary_sum->src1)) {
                    TypeCase(binary_mul, ir::insns::Binary *, binary_sum->bb->func->def_list.at(binary_sum->src1)) {
                      if (binary_mul->op == BinaryOp::Mul) {
                        if (binary_sum->src2 == reg_i) { // j * n + i
                          if (binary_mul->src1 == reg_j || binary_mul->src2 == reg_j) reverse_found = true;
                        }
                        if (binary_sum->src2 == reg_j) { // i * n + j
                          if (binary_mul->src1 == reg_i || binary_mul->src2 == reg_i) order_found = true;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    return !order_found && reverse_found;
  }

};

InterchangeLoopInfo get_interchange_info(Loop *loop) {
  auto in_loop = [loop](BasicBlock *b) {
    Loop *l = b->loop;
    while (l != nullptr && l != loop) {
      l = l->outer;
    }
    return l == loop;
  };
  InterchangeLoopInfo info;
  info.valid = false;
  info.entry = loop->header;
  vector<BasicBlock *> stack;
  unordered_set<BasicBlock *> loop_bbs;
  stack.push_back(loop->header);
  while (stack.size()) {
    auto bb = stack.back();
    stack.pop_back();
    if (in_loop(bb)) {
      loop_bbs.insert(bb);
      for (auto each : bb->dom) {
        stack.push_back(each);
      }
    }
  }
  info.exit = nullptr;
  for (auto bb : loop_bbs) {
    for (auto succ_bb : bb->succ) {
      if (!in_loop(succ_bb)) {
        if (info.exit != nullptr && succ_bb != info.exit) {
          return info; // multiple exit
        }
        info.exit = succ_bb;
      }
    }
  }
  if (info.exit == nullptr) return info; // dead loop
  info.exit_prev = nullptr;
  for (auto bb : info.exit->prev) {
    if (in_loop(bb)) {
      if (info.exit_prev == nullptr) info.exit_prev = bb;
      else return info; // multiple exit_prev
    }
  }
  TypeCase(br_cond, ir::insns::Branch *, info.exit_prev->insns.back().get()) {
    info.ind_br = br_cond;
    TypeCase(binary_cond, ir::insns::Binary *, br_cond->bb->func->def_list.at(br_cond->val)) {
      info.ind_cond = binary_cond;
      if (!binary_cond->bb->func->has_param(binary_cond->src2) && in_loop(binary_cond->bb->func->def_list.at(binary_cond->src2)->bb)) return info; // end_reg should be region constant
      if (binary_cond->bb->func->has_param(binary_cond->src1)) return info;
      TypeCase(binary_update, ir::insns::Binary *, binary_cond->bb->func->def_list.at(binary_cond->src1)) {
        info.ind_upd = binary_update;
        if (binary_update->dst.type != Int) return info;
        Reg reg_i = binary_update->src1, reg_c = binary_update->src2;
        if (binary_update->bb->func->has_param(reg_i) || binary_update->bb->func->has_param(reg_c)) return info;
        TypeCase(dummy, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_i)) {
          std::swap(reg_i, reg_c);
        }
        TypeCase(inst_phi, ir::insns::Phi *, binary_update->bb->func->def_list.at(reg_i)) {
          if (inst_phi->incoming.size() != 2) return info;
          if (inst_phi->bb != info.entry) return info;
          info.ind_phi = inst_phi;
        } else return info; // no phi inst
      } else return info; // update not binary
    } else return info; // cond not binary
  } else assert(false);
  info.valid = true;
  return info;
}

void loop_interchange(TightlyNestedLoopInfo nested_info) {
  debug(std::cerr) << "loop interchange " << nested_info.outer.entry->func->name << ":" << nested_info.outer.entry->label << std::endl;
  auto find_insn = [](ir::Instruction *inst) -> std::unique_ptr<ir::Instruction>& {
    for (auto &insn : inst->bb->insns) {
      if (insn.get() == inst) return insn;
    }
    assert(false);
  };
  Reg reg_i = nested_info.outer.ind_phi->dst, reg_j = nested_info.inner.ind_phi->dst;
  // swap two phi
  auto &outer_phi = find_insn(nested_info.outer.ind_phi);
  auto &inner_phi = find_insn(nested_info.inner.ind_phi);
  outer_phi->remove_use_def();
  inner_phi->remove_use_def();
  auto o_phi = outer_phi.release();
  auto i_phi = inner_phi.release();
  std::swap(o_phi->bb, i_phi->bb);
  outer_phi.reset(i_phi);
  inner_phi.reset(o_phi);
  outer_phi->add_use_def();
  inner_phi->add_use_def();
  // modify phi's incoming bb
  BasicBlock *into_bb = nullptr;
  TypeCase(new_inner_phi, ir::insns::Phi *, inner_phi.get()) {
    unordered_map<BasicBlock *, Reg> new_incoming;
    for (auto pair : new_inner_phi->incoming) {
      if (pair.first == nested_info.outer.exit_prev) {
        new_incoming[nested_info.inner.exit_prev] = pair.second;
      } else {
        into_bb = pair.first;
        new_incoming[nested_info.outer.entry] = pair.second;
      }
    }
    new_inner_phi->incoming = new_incoming;
  } else assert(false);
  assert(into_bb != nullptr);
  TypeCase(new_outer_phi, ir::insns::Phi *, outer_phi.get()) {
    unordered_map<BasicBlock *, Reg> new_incoming;
    for (auto &pair : new_outer_phi->incoming) {
      if (pair.first == nested_info.inner.exit_prev) {
        new_incoming[nested_info.outer.exit_prev] = pair.second;
      } else if (pair.first == nested_info.outer.entry) {
        new_incoming[into_bb] = pair.second;
      } else assert(false);
    }
    new_outer_phi->incoming = new_incoming;
  } else assert(false);
  // new br val, upd dst
  auto func = nested_info.outer.entry->func;
  nested_info.outer.ind_br->change_use(nested_info.outer.ind_br->val, func->new_reg(Int));
  nested_info.inner.ind_br->change_use(nested_info.inner.ind_br->val, func->new_reg(Int));
  auto new_outer_ind_upd = new ir::insns::Binary(func->new_reg(Int), nested_info.inner.ind_upd->op, nested_info.inner.ind_upd->src1, nested_info.inner.ind_upd->src2);
  auto new_inner_ind_upd = new ir::insns::Binary(func->new_reg(Int), nested_info.outer.ind_upd->op, nested_info.outer.ind_upd->src1, nested_info.outer.ind_upd->src2);
  auto new_outer_ind_cond = new ir::insns::Binary(nested_info.outer.ind_br->val, nested_info.inner.ind_cond->op, nested_info.inner.ind_cond->src1, nested_info.inner.ind_cond->src2);
  auto new_inner_ind_cond = new ir::insns::Binary(nested_info.inner.ind_br->val, nested_info.outer.ind_cond->op, nested_info.outer.ind_cond->src1, nested_info.outer.ind_cond->src2);
  new_outer_ind_cond->change_use(reg_j, new_outer_ind_upd->dst);
  new_inner_ind_cond->change_use(reg_i, new_inner_ind_upd->dst);
  // insert new insts
  nested_info.outer.exit_prev->insert_before_ter(new_outer_ind_upd);
  nested_info.outer.exit_prev->insert_before_ter(new_outer_ind_cond);
  new_outer_ind_upd->add_use_def();
  new_outer_ind_cond->add_use_def();
  nested_info.inner.exit_prev->insert_before_ter(new_inner_ind_upd);
  nested_info.inner.exit_prev->insert_before_ter(new_inner_ind_cond);
  new_inner_ind_upd->add_use_def();
  new_inner_ind_cond->add_use_def();
  // change phi incoming reg
  outer_phi->change_use(nested_info.inner.ind_upd->dst, new_outer_ind_upd->dst);
  inner_phi->change_use(nested_info.outer.ind_upd->dst, new_inner_ind_upd->dst);
}

void loop_interchange(ir::Function *func) {
  func->cfg->build();
  func->loop_analysis();
  for (auto &loop_ptr : func->loops) {
    Loop *loop = loop_ptr.get();
    if (loop->outer == nullptr) continue;
    if (loop->outer->outer != nullptr) continue;
    // only consider interchange outermost and second outermost loops
    auto outer_info = get_interchange_info(loop->outer);
    auto inner_info = get_interchange_info(loop);
    auto nested_info = TightlyNestedLoopInfo(inner_info, outer_info);
    if (!nested_info.valid) continue;
    if (nested_info.profitable(loop)) {
      loop_interchange(nested_info);
    }
  }
}

void loop_interchange(ir::Program *prog) {
  for(auto &func : prog->functions) {
    loop_interchange(&func.second);
  }
  ir_validation(prog);
}

}
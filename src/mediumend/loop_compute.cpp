#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;
using ir::Loop;

// Compute loop:
// - one exit
// - one into edge to entry (into_entry)
// - single loop bb (entry)
// - only one phi at entry (cond_var)
// - i = x, i < n, i++
struct ComputeLoopInfo {
  bool valid;
  Reg start_reg; // reg start
  int step; // const step
  Reg end_reg; // reg end
  BinaryOp bop;
  BasicBlock *into_entry;
  BasicBlock *entry;
  BasicBlock *exit;
  vector<std::pair<Reg, ir::insns::Binary *> > acc_regs; // init, update
  ir::insns::Binary *cond_cmp;
  ir::insns::Binary *binary_upd;

  bool check() {
    acc_regs.clear();
    for (auto &insn : entry->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        continue;
      } else TypeCase(binary, ir::insns::Binary *, insn.get()) {
        if (binary == cond_cmp) continue;
        if (binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub && binary->op != BinaryOp::Shl && binary->op != BinaryOp::Shr) return false;
        if (!entry->func->has_param(binary->src2) && entry->func->def_list.at(binary->src2)->bb == entry) return false;
        if (entry->func->has_param(binary->src1)) return false;
        TypeCase(phi, ir::insns::Phi *, entry->func->def_list.at(binary->src1)) {
          if (phi->bb != entry) return false;
          if (phi->incoming.size() != 2) return false;
          Reg init = Reg(Int, -1);
          for (auto pair : phi->incoming) {
            if (pair.first == entry) continue;
            else init = pair.second;
          }
          assert(init.id != -1);
          acc_regs.push_back(std::make_pair(init, binary));
        }
      } else TypeCase(br, ir::insns::Branch *, insn.get()) {
        continue;
      } else {
        return false;
      }
    }
    assert(entry->insns.size() == acc_regs.size() * 2 + 2);
    return true;
  }
};

ComputeLoopInfo get_compute_info(Loop *loop, BasicBlock *exit) {
  ComputeLoopInfo info;
  info.valid = false;
  info.entry = loop->header;
  info.exit = exit;
  info.into_entry = nullptr;
  for (auto bb : info.entry->prev) {
    if (bb->loop != loop) {
      if (info.into_entry == nullptr) info.into_entry = bb;
      else return info; // multiple into edge
    }
  }
  TypeCase(br_cond, ir::insns::Branch *, info.entry->insns.back().get()) { // br entry, exit
    TypeCase(binary_cond, ir::insns::Binary *, br_cond->bb->func->def_list.at(br_cond->val)) { // i < n
      if (binary_cond->dst.type != Int) return info;
      if (binary_cond->op != BinaryOp::Lt) return info;
      if (!binary_cond->bb->func->has_param(binary_cond->src2) && (binary_cond->bb->func->def_list.at(binary_cond->src2)->bb->loop == loop)) return info; // end_reg should be region constant
      info.end_reg = binary_cond->src2;
      info.cond_cmp = binary_cond;
      if (binary_cond->bb->func->has_param(binary_cond->src1)) return info;
      TypeCase(binary_update, ir::insns::Binary *, binary_cond->bb->func->def_list.at(binary_cond->src1)) { // i = i + c
        if (binary_update->dst.type != Int) return info;
        if (binary_update->op != BinaryOp::Add) return info; // only consider i + c
        if (binary_update->bb->func->use_list.at(binary_update->dst).size() > 2) return info; // updated i only used in cond_cmp & phi
        Reg reg_i = binary_update->src1, reg_c = binary_update->src2;
        if (binary_update->bb->func->has_param(reg_i) || binary_update->bb->func->has_param(reg_c)) return info;
        TypeCase(dummy, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_i)) {
          std::swap(reg_i, reg_c);
        }
        TypeCase(update_imm, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_c)) { // c
          assert(update_imm->imm.type == Int);
          if (update_imm->imm.iv != 1) return info; // only consider c = 1
          info.step = update_imm->imm.iv;
        } else return info; // step not const
        TypeCase(inst_phi, ir::insns::Phi *, binary_update->bb->func->def_list.at(reg_i)) {
          if (inst_phi->incoming.size() != 2) return info;
          if (inst_phi->bb != info.entry) return info;
          Reg reg_i_init;
          for (auto pair : inst_phi->incoming) {
            if (pair.second != binary_update->dst) { // find init
              reg_i_init = pair.second;
              assert(pair.first->loop != loop);
            }
          }
          info.start_reg = reg_i_init;
//           info.entry_ind_phi = inst_phi;
        } else return info; // no phi inst
        info.binary_upd = binary_update;
      } else return info; // update not binary
    } else return info; // cond not binary
  } else assert(false);
  info.valid = true;
  return info;
}

void loop_compute(ir::Function *func, ComputeLoopInfo &info) {
  vector<ir::Instruction *> inst_to_del;
  vector<ir::Instruction *> inst_to_add;
  Reg range_reg = func->new_reg(Int);
  inst_to_add.push_back(new ir::insns::Binary(range_reg, BinaryOp::Sub, info.end_reg, info.start_reg));
  for (auto &insn : info.entry->insns) {
    TypeCase(phi, ir::insns::Phi *, insn.get()) {
      inst_to_del.push_back(insn.get());
    } else if (insn.get() == info.cond_cmp || insn.get() == info.binary_upd) {
      inst_to_del.push_back(insn.get());
    } else TypeCase(binary, ir::insns::Binary *, insn.get()) {
      auto new_inst = new ir::insns::Binary(func->new_reg(Int), BinaryOp::Mul, binary->src2, range_reg);
      inst_to_add.push_back(new_inst);
      binary->change_use(binary->src2, new_inst->dst);
    } else TypeCase(br, ir::insns::Branch *, insn.get()) {
      auto jmp = new ir::insns::Jump(info.exit);
      jmp->bb = br->bb;
      br->remove_use_def();
      insn.reset(jmp);
      jmp->add_use_def();
    } else assert(false);
  }
  info.entry->succ.erase(info.entry);
  info.entry->prev.erase(info.entry);
  for (auto pair : info.acc_regs) {
    pair.second->change_use(pair.second->src1, pair.first);
  }
  for (auto inst : inst_to_del) {
    inst->remove_use_def();
    info.entry->remove(inst);
  }
  for (auto it = inst_to_add.rbegin(); it != inst_to_add.rend(); it++) {
    info.entry->push_front(*it);
  }
}

void loop_compute(ir::Function *func) {
  func->cfg->build();
  func->loop_analysis();
  for (auto &loop_ptr : func->loops) {
    Loop *loop = loop_ptr.get();
    if (!loop->no_inner) { // only perform in most inner loop
      continue;
    }
    unordered_set<BasicBlock *> loop_bbs; // collect loop bbs
    vector<BasicBlock *> stack;
    stack.push_back(loop->header);
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      if (bb->loop == loop) {
        loop_bbs.insert(bb);
        for (auto each : bb->dom) {
          stack.push_back(each);
        }
      }
    }
    if (loop_bbs.size() > 1) continue; // only consider single block
    BasicBlock *exit_bb = nullptr;
    bool check = true; // check loop has only one exit
    for (auto bb : loop_bbs) {
      if (!check) break;
      for (auto succ_bb : bb->succ) {
        if (succ_bb->loop == nullptr || succ_bb->loop != loop) {
          if (exit_bb != nullptr && succ_bb != exit_bb) {
            check = false;
            break;
          }
          exit_bb = succ_bb;
        }
      }
    }
    if (exit_bb == nullptr || !check) { // dead loop or multiple exit
      continue;
    }
    auto loop_info = get_compute_info(loop, exit_bb);
    if (!loop_info.valid) continue;
    if (loop_info.check()) {
      loop_compute(func, loop_info);
    }
  }
}

void loop_compute(ir::Program *prog) {
  for(auto &func : prog->functions) {
    loop_compute(&func.second);
  }
  ir_validation(prog);
}

}
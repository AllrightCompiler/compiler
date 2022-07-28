#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Instruction;
using ir::Reg;
using ir::Loop;

static const int UNROLL_CNT = 4;
static int map_curid;
static unordered_map<Reg, Reg> reg_map[2]; // contains only regs inside loop, two maps representing now and prev
static unordered_map<BasicBlock*, BasicBlock*> bb_map[2]; // contains only bbs inside loop, two maps representing now and prev

// Simple loop: one exit, one back edge to entry (and only one exit_prev), cond_var appear twice (phi, update)
struct SimpleLoopInfo {
  int loop_type; // 0: nothing special; 1: i = c0, i </<= c1, i++; 2: i = c0, i </<= n, i++
  int start; // const start
  int step; // const step
  int end; // const end, available when type = 1
  Reg end_reg;
  Reg new_end_reg;
  BinaryOp cond_op;
  int inst_cnt;
  BasicBlock *exit_prev;
  BasicBlock *exit;
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

SimpleLoopInfo get_loop_info(Loop *loop, const unordered_set<BasicBlock *> &loop_bbs, BasicBlock *exit) {
  SimpleLoopInfo info;
  info.loop_type = 0;
  info.inst_cnt = 0;
  for (auto bb : loop_bbs) {
    info.inst_cnt += bb->insns.size();
  }
  BasicBlock *entry = loop->header;
  BasicBlock *exit_prev = nullptr;
  for (auto bb : exit->prev) {
    if (bb->loop != nullptr && bb->loop == loop) {
      if (exit_prev == nullptr) exit_prev = bb;
      else return info; // multiple exit_prev
    }
  }
  info.exit_prev = exit_prev;
  for (auto bb : entry->prev) {
    if (bb->loop != nullptr && bb->loop == loop) {
      if (bb != exit_prev) return info; // multiple back edge
    }
  }
  int type = 2;
  Instruction *inst_cond = exit_prev->insns.back().get();
  TypeCase(br_cond, ir::insns::Branch *, inst_cond) {
    TypeCase(binary_cond, ir::insns::Binary *, br_cond->bb->func->def_list.at(br_cond->val)) {
      if (!is_cmp(binary_cond)) return info;
      if (binary_cond->dst.type != Int) return info;
      if (binary_cond->op != BinaryOp::Lt && binary_cond->op != BinaryOp::Leq) return info; // TODO: only consider i < n or i <= n now
      if (binary_cond->bb->func->has_param(binary_cond->src2)) info.end_reg = binary_cond->src2;
      else TypeCase(end_imm, ir::insns::LoadImm *, binary_cond->bb->func->def_list.at(binary_cond->src2)) {
        info.end = end_imm->imm.iv;
        info.end_reg = binary_cond->src2;
        type = 1;
      } else info.end_reg = binary_cond->src2;
      info.cond_op = binary_cond->op;
      if (binary_cond->bb->func->has_param(binary_cond->src1)) return info;
      TypeCase(binary_update, ir::insns::Binary *, binary_cond->bb->func->def_list.at(binary_cond->src1)) {
        if (binary_update->dst.type != Int) return info;
        if (binary_update->op != BinaryOp::Add) return info; // TODO: only consider i + c now
        Reg reg_i = binary_update->src1, reg_c = binary_update->src2;
        if (binary_update->bb->func->has_param(reg_i) || binary_update->bb->func->has_param(reg_c)) return info;
        TypeCase(dummy, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_i)) {
          std::swap(reg_i, reg_c);
        }
        TypeCase(update_imm, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_c)) {
          if (!update_imm->imm.isValue(1)) return info; // TODO: only consider c = 1 now
          info.step = update_imm->imm.iv;
        } else return info;
        TypeCase(inst_phi, ir::insns::Phi *, binary_update->bb->func->def_list.at(reg_i)) {
          if (inst_phi->bb != entry) return info;
          if (inst_phi->incoming.size() != 2) return info;
          Reg reg_i_init;
          for (auto pair : inst_phi->incoming) {
            if (pair.second != binary_update->dst) {
              reg_i_init = pair.second;
              assert(pair.first->loop == nullptr || pair.first->loop != loop);
            }
          }
          if (binary_update->bb->func->has_param(reg_i_init)) return info;
          TypeCase(init_imm, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_i_init)) {
            info.start = init_imm->imm.iv;
          } else return info; // i init is not imm
        } else return info; // no phi inst
      } else return info; // update not binary inst
    } else return info; // cond not binary
  } else assert(false);
  info.loop_type = type;
  return info;
}

Instruction *copy_inst(SimpleLoopInfo info, Instruction *inst, BasicBlock *entry, BasicBlock *exit) {
  auto reg_map_if = [=](Reg reg) {
    if (reg_map[map_curid].count(reg)) return reg_map[map_curid].at(reg);
    else return reg;
  };
  auto bb_map_to = [=](BasicBlock *bb) {
    if (bb == exit) return exit;
    if (bb == entry && info.loop_type != 3) return entry;
    return bb_map[map_curid].at(bb);
  };
  TypeCase(binary, ir::insns::Binary *, inst) {
    auto new_inst = new ir::insns::Binary(reg_map_if(binary->dst), binary->op, reg_map_if(binary->src1), reg_map_if(binary->src2));
    return new_inst;
  } else TypeCase(unary, ir::insns::Unary *, inst) {
    auto new_inst = new ir::insns::Unary(reg_map_if(unary->dst), unary->op, reg_map_if(unary->src));
    return new_inst;
  } else TypeCase(convert, ir::insns::Convert *, inst) {
    auto new_inst = new ir::insns::Convert(reg_map_if(convert->dst), reg_map_if(convert->src));
    return new_inst;
  } else TypeCase(load, ir::insns::Load *, inst) {
    auto new_inst = new ir::insns::Load(reg_map_if(load->dst), reg_map_if(load->addr));
    return new_inst;
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    auto new_inst = new ir::insns::LoadAddr(reg_map_if(loadaddr->dst), loadaddr->var_name);
    return new_inst;
  } else TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    auto new_inst = new ir::insns::LoadImm(reg_map_if(loadimm->dst), loadimm->imm);
    return new_inst;
  } else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
    auto indices = gep->indices;
    for(auto &index : indices){
      index = reg_map_if(index);
    }
    auto new_inst = new ir::insns::GetElementPtr(reg_map_if(gep->dst), gep->type, reg_map_if(gep->base), indices);
    return new_inst;
  } else TypeCase(call, ir::insns::Call *, inst) {
    auto args = call->args;
    for(auto &arg : args){
      arg = reg_map_if(arg);
    }
    auto new_inst = new ir::insns::Call(reg_map_if(call->dst), call->func, args);
    return new_inst;
  } else TypeCase(store, ir::insns::Store *, inst) {
    auto new_inst = new ir::insns::Store(reg_map_if(store->addr), reg_map_if(store->val));
    return new_inst;
  } else TypeCase(br, ir::insns::Branch *, inst) {
    auto new_inst = new ir::insns::Branch(reg_map_if(br->val), bb_map_to(br->true_target), bb_map_to(br->false_target));
    return new_inst;
  } else TypeCase(jmp, ir::insns::Jump *, inst) {
    auto new_inst = new ir::insns::Jump(bb_map_to(jmp->target));
    return new_inst;
  } else TypeCase(ret, ir::insns::Return *, inst) {
    auto ret_val = ret->val;
    if (ret_val.has_value()) ret_val.value() = reg_map_if(ret_val.value());
    auto new_inst = new ir::insns::Return(ret_val);
  } else TypeCase(phi, ir::insns::Phi *, inst) {
    auto new_inst = new ir::insns::Phi(reg_map_if(phi->dst), phi->array_ssa);
    if (inst->bb == entry) { // new entry
      for (auto pair : phi->incoming) {
        auto income_bb = pair.first;
        if (income_bb->loop == nullptr || income_bb->loop != entry->loop) { // not in loop
          continue;
        }
        auto mapped_bb = bb_map[!map_curid].at(income_bb);
        auto mapped_reg = reg_map[!map_curid].at(pair.second);
        new_inst->incoming[mapped_bb] = mapped_reg;
        if (info.loop_type == 3) { // in remaining loop
          mapped_bb = bb_map[map_curid].at(income_bb);
          mapped_reg = reg_map[map_curid].at(pair.second);
          new_inst->incoming[mapped_bb] = mapped_reg;
        }
      }
    } else {
      for (auto pair : phi->incoming) {
        auto mapped_bb = bb_map[map_curid].at(pair.first);
        auto mapped_reg = reg_map[map_curid].at(pair.second);
        new_inst->incoming[mapped_bb] = mapped_reg;
      }
    }
    return new_inst;
  } else assert(false);
}

void copy_bb(SimpleLoopInfo info, bool last_turn, BasicBlock *bb, BasicBlock *new_bb, BasicBlock *entry, BasicBlock *exit) {
  // map prev & succ
  for (auto prev_bb : bb->prev) {
    if (bb == entry) continue; // new_entry's prev has been done
    new_bb->prev.insert(bb_map[map_curid].at(prev_bb));
  }
  for (auto succ_bb : bb->succ) {
    if (succ_bb == entry) {
      if (info.loop_type == 0 || (info.loop_type == 2 && last_turn)) {
        new_bb->succ.insert(succ_bb);
        succ_bb->prev.insert(new_bb);
      }
    } else if (succ_bb == exit) {
      if (info.loop_type == 0 || last_turn) {
        new_bb->succ.insert(succ_bb);
        succ_bb->prev.insert(new_bb);
      }
    } else {
      new_bb->succ.insert(bb_map[map_curid].at(succ_bb));
    }
  }
  // copy insts
  for (auto &insn : bb->insns) {
    new_bb->push_back(copy_inst(info, insn.get(), entry, exit));
  }
}

void loop_unroll(ir::Function *func, Loop *loop, SimpleLoopInfo info, const unordered_set<BasicBlock *> &loop_bbs, const int unroll_cnt) {
  map_curid = 0;
  bb_map[0].clear();
  bb_map[1].clear();
  reg_map[0].clear();
  reg_map[1].clear();
  BasicBlock *exit_bb = info.exit;
  for (auto bb : loop_bbs) {
    bb_map[map_curid][bb] = bb;
    for (auto &insn : bb->insns) {
      TypeCase(output, ir::insns::Output *, insn.get()) {
        reg_map[map_curid][output->dst] = output->dst;
      }
    }
  }
  if (info.loop_type == 0) {
    // Add phi for regs defined in loop but used outside
    for (auto bb : loop_bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          Reg reg = output->dst, newreg;
          if (!bb->func->use_list.count(reg)) continue;
          bool flag = false;
          vector<Instruction *> insts_to_change_use;
          for (auto i : bb->func->use_list.at(reg)) {
            if (i->bb->loop != nullptr && i->bb->loop == loop) continue; // in loop
            TypeCase(phi, ir::insns::Phi *, i) {
              if (i->bb == exit_bb) continue;
            }
            if (!flag) { // create phi
              newreg = func->new_reg(reg.type);
              auto phi_i = new ir::insns::Phi(newreg);
              for (auto exit_prev : exit_bb->prev) {
                assert(exit_prev->loop != nullptr && exit_prev->loop == loop);
                phi_i->incoming[exit_prev] = reg;
              }
              exit_bb->push_front(phi_i);
            }
            flag = true;
            insts_to_change_use.push_back(i);
          }
          for (auto i : insts_to_change_use) {
            i->change_use(reg, newreg);
          }
        }
      }
    }
  }
  unordered_set<BasicBlock *> back_paths; // bbs in original loop that go back to entry
  for (auto entry_prev_bb : loop->header->prev) {
    if (entry_prev_bb->loop != nullptr && entry_prev_bb->loop == loop) { // bb in loop
      back_paths.insert(entry_prev_bb);
    }
  }
  unordered_set<BasicBlock *> exit_paths; // bbs in original loop that go to exit
  for (auto exit_prev_bb : exit_bb->prev) {
    if (exit_prev_bb->loop != nullptr && exit_prev_bb->loop == loop) { // bb in loop
      exit_paths.insert(exit_prev_bb);
    }
  }
  unordered_set<BasicBlock *> loop0_to_entry; // bb in loop0 -> loop1-entry, inserted after for loop to make loop0 clean
  BasicBlock *loop1_entry = nullptr;
  for (int l = 1; l < unroll_cnt; l++) {
    map_curid ^= 1;
    // 1. Create new bbs
    for (auto bb : loop_bbs) {
      BasicBlock *new_bb = new BasicBlock;
      new_bb->func = func;
      new_bb->loop = loop; // just for convenience
      func->bbs.emplace_back(new_bb);
      new_bb->label = "unroll_" + std::to_string(l) + "_" + bb->label;
      bb_map[map_curid][bb] = new_bb;
    }
    // 2. Connect entry with prev loop
    auto entry = loop->header;
    auto new_entry = bb_map[map_curid][loop->header];
    if (l == 1) loop1_entry = new_entry;
    for (auto entry_prev_bb : entry->prev) {
      if (entry_prev_bb->loop == nullptr || entry_prev_bb->loop != loop) {
        continue; // bb not in loop
      }
      if (l == 1) { // delayed to make loop0 clean
        loop0_to_entry.insert(bb_map[!map_curid][entry_prev_bb]);
      } else {
        bb_map[!map_curid][entry_prev_bb]->succ.erase(entry);
        bb_map[!map_curid][entry_prev_bb]->succ.insert(new_entry);
        auto ter_inst = bb_map[!map_curid][entry_prev_bb]->insns.back().get();
        TypeCase(jmp, ir::insns::Jump *, ter_inst) {
          assert(jmp->target == entry);
          jmp->target = new_entry;
        } else TypeCase(br, ir::insns::Branch *, ter_inst) {
          assert(br->true_target == entry || br->false_target == entry);
          if (info.loop_type == 0) {
            if (br->true_target == entry)
              br->true_target = new_entry;
            if (br->false_target == entry)
              br->false_target = new_entry;
          } else {
            bb_map[!map_curid][entry_prev_bb]->insns.back()->remove_use_def();
            bb_map[!map_curid][entry_prev_bb]->insns.back().reset(new ir::insns::Jump(new_entry));
            bb_map[!map_curid][entry_prev_bb]->insns.back()->add_use_def();
          }
        } else assert(false);
      }
      new_entry->prev.insert(bb_map[!map_curid][entry_prev_bb]);
    }
    // 3. Fill new bbs
    // First create all new regs
    for (auto bb : loop_bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          Reg reg = bb->func->new_reg(output->dst.type);
          reg_map[map_curid][output->dst] = reg;
        }
      }
    }
    for (auto bb : loop_bbs) {
      copy_bb(info, l == unroll_cnt - 1, bb, bb_map[map_curid][bb], loop->header, exit_bb);
    }
    // 4. Modify entry's phi (Delayed), exit's phi
    if (info.loop_type == 0) {
      for (auto &insn : exit_bb->insns) {
        TypeCase(phi, ir::insns::Phi *, insn.get()) {
          phi->remove_use_def();
          for (auto bb : exit_paths) {
            Reg reg = phi->incoming.at(bb);
            if (reg_map[map_curid].count(reg)) reg = reg_map[map_curid].at(reg);
            phi->incoming[bb_map[map_curid].at(bb)] = reg;
          }
          phi->add_use_def();
        } else break;
      }
    }
  }
  if (info.loop_type == 2) { // remaining loop
    map_curid ^= 1;
    // 1. Create new bbs
    for (auto bb : loop_bbs) {
      BasicBlock *new_bb = new BasicBlock;
      new_bb->func = func;
      new_bb->loop = loop; // just for convenience
      func->bbs.emplace_back(new_bb);
      new_bb->label = "unroll_" + std::to_string(unroll_cnt) + "_" + bb->label;
      bb_map[map_curid][bb] = new_bb;
    }
    // 2. Connect entry with prev loop
    auto entry = loop->header;
    auto new_entry = bb_map[map_curid][loop->header];
    auto old_exit_prev = bb_map[!map_curid].at(info.exit_prev);
    auto new_exit_prev = bb_map[map_curid].at(info.exit_prev);
    new_entry->prev.insert(old_exit_prev);
    new_entry->prev.insert(new_exit_prev);
    old_exit_prev->succ.erase(exit_bb);
    old_exit_prev->succ.insert(new_entry);
    auto old_ter_inst = old_exit_prev->insns.back().get();
    TypeCase(br, ir::insns::Branch *, old_ter_inst) {
      if (br->true_target == exit_bb) br->true_target = new_entry;
      else if (br->false_target == exit_bb) br->false_target = new_entry;
      else assert(false);
    } else assert(false);
    // 3. Fill new bbs
    // First create all new regs
    for (auto bb : loop_bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          Reg reg = bb->func->new_reg(output->dst.type);
          reg_map[map_curid][output->dst] = reg;
        }
      }
    }
    info.loop_type = 3; // for convenience, tricky
    for (auto bb : loop_bbs) {
      copy_bb(info, true, bb, bb_map[map_curid][bb], loop->header, exit_bb);
    }
    info.loop_type = 2;
    // 4. Modify exit's phi
    for (auto &insn : exit_bb->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        for (auto bb : exit_paths) {
          Reg reg = phi->incoming.at(bb);
          if (reg_map[map_curid].count(reg)) reg = reg_map[map_curid].at(reg);
          phi->incoming[bb_map[map_curid].at(bb)] = reg;
        }
        phi->add_use_def();
      } else break;
    }
    // 5. Restore loop end
    Instruction *last_br = bb_map[map_curid].at(info.exit_prev)->insns.back().get();
    last_br->change_use(info.new_end_reg, info.end_reg);
  }
  // Modify Loop0 entry's phi & prev
  if (info.loop_type != 0) { // delete phi & prev
    // delete loop0 entry's prev in loop0
    for (auto bb : back_paths) {
      loop->header->prev.erase(bb);
    }
    // delete exit's prev in loop0
    assert(exit_paths.size() == 1);
    for (auto bb : exit_paths) { // only one exit_prev
      exit_bb->prev.erase(bb);
    }
    // modify exit's phi
    for (auto &insn : exit_bb->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        for (auto bb : exit_paths) { // only one exit_prev
          phi->incoming.erase(bb);
        }
        phi->add_use_def();
      } else break;
    }
  }
  if (info.loop_type == 1) {
    // delete last bb's Br
    bb_map[map_curid].at(info.exit_prev)->insns.back()->remove_use_def();
    bb_map[map_curid].at(info.exit_prev)->insns.back().reset(new ir::insns::Jump(exit_bb));
    bb_map[map_curid].at(info.exit_prev)->insns.back()->add_use_def();
    for (auto &insn : loop->header->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        for (auto bb : back_paths) {
          phi->incoming.erase(bb);
        }
        phi->add_use_def();
      } else break;
    }
  } else {
    if (info.loop_type == 2) map_curid ^= 1; // correct map_cur_id
    vector<BasicBlock *> entry_prev_bb_to_insert;
    for (auto bb : back_paths) {
      entry_prev_bb_to_insert.push_back(bb_map[map_curid].at(bb));
    }
    for (auto bb : entry_prev_bb_to_insert) {
      loop->header->prev.insert(bb);
    }
    for (auto &insn : loop->header->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        for (auto bb : back_paths) {
          Reg reg = phi->incoming.at(bb);
          if (reg_map[map_curid].count(reg)) reg = reg_map[map_curid].at(reg);
          phi->incoming.erase(bb);
          phi->incoming[bb_map[map_curid].at(bb)] = reg;
        }
        phi->add_use_def();
      } else break;
    }
    if (info.loop_type == 2) map_curid ^= 1;
  }
  // loop0 -> loop1 entry
  for (auto bb : loop0_to_entry) {
    bb->succ.erase(loop->header);
    bb->succ.insert(loop1_entry);
    auto ter_inst = bb->insns.back().get();
    TypeCase(jmp, ir::insns::Jump *, ter_inst) {
      assert(jmp->target == loop->header);
      jmp->target = loop1_entry;
    } else TypeCase(br, ir::insns::Branch *, ter_inst) {
      assert(br->true_target == loop->header || br->false_target == loop->header);
      if (info.loop_type == 0) {
        if (br->true_target == loop->header)
          br->true_target = loop1_entry;
        if (br->false_target == loop->header)
          br->false_target = loop1_entry;
      } else {
        bb->insns.back()->remove_use_def();
        bb->insns.back().reset(new ir::insns::Jump(loop1_entry));
        bb->insns.back()->add_use_def();
      }
    } else assert(false);
  }
  if (info.loop_type != 0) {
    // Modify regs defined in loop but used outside
    for (auto bb : loop_bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          Reg reg = output->dst, newreg = reg_map[map_curid].at(reg);
          if (!bb->func->use_list.count(reg)) continue;
          vector<Instruction *> insts_to_change_use;
          for (auto i : bb->func->use_list.at(reg)) {
            if (i->bb->loop != nullptr && i->bb->loop == loop) continue; // in loop
            insts_to_change_use.push_back(i);
          }
          for (auto i : insts_to_change_use) {
            i->change_use(reg, newreg);
          }
        }
      }
    }
  }
}

void loop_unroll(ir::Function *func) {
  func->cfg->build();
  func->loop_analysis();
  for (auto &loop_ptr : func->loops) {
    Loop *loop = loop_ptr.get();
    if (!loop->no_inner) { // only perform on deepest loop
      continue;
    }
    unordered_set<BasicBlock *> loop_bbs; // add bbs into loop_bbs
    vector<BasicBlock *> stack;
    stack.push_back(loop->header);
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      if (bb->loop != loop) {
        continue;
      }
      loop_bbs.insert(bb);
      for (auto each : bb->dom) {
        stack.push_back(each);
      }
    }
    BasicBlock *exit_bb = nullptr;
    bool check = true; // check loop has only one exit (the exit bb is outside of loop)
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
    // Unroll this loop
    auto loop_info = get_loop_info(loop, loop_bbs, exit_bb);
    loop_info.exit = exit_bb;
    int unroll_cnt = UNROLL_CNT;
    if (loop_info.loop_type == 1) {
      int full_cnt;
      if (loop_info.cond_op == BinaryOp::Lt) {
        full_cnt = (loop_info.end - loop_info.start + loop_info.step - 1) / loop_info.step;
        if (loop_info.start >= loop_info.end) full_cnt = 0;
      } else if (loop_info.cond_op == BinaryOp::Leq) {
        full_cnt = (loop_info.end - loop_info.start) / loop_info.step + 1;
        if (loop_info.start > loop_info.end) full_cnt = 0;
      } else assert(false);
      assert(full_cnt >= 0);
      if (full_cnt == 0 || full_cnt == 1) continue;
      if (full_cnt < 50 && full_cnt * loop_info.inst_cnt <= 500) {
        unroll_cnt = full_cnt; // fully unroll
      } else loop_info.loop_type = 2;
    }
    if (loop_info.loop_type == 2) { // Decrease end by unroll * step, therefore erase branches jump out in middle
      Instruction *last_br = loop_info.exit_prev->insns.back().get();
      Reg new_end = func->new_reg(Int);
      Reg imm = func->new_reg(Int);
      auto loadimm_offset = new ir::insns::LoadImm(imm, ConstValue(loop_info.step * unroll_cnt));
      auto binary_new_end = new ir::insns::Binary(new_end, BinaryOp::Sub, loop_info.end_reg, imm);
      if (func->has_param(loop_info.end_reg)) {
        func->bbs.front()->push_front(binary_new_end);
        func->bbs.front()->push_front(loadimm_offset);
      } else {
        Instruction *inst_end = func->def_list.at(loop_info.end_reg);
        inst_end->bb->insert_after_inst(inst_end, binary_new_end);
        inst_end->bb->insert_after_inst(inst_end, loadimm_offset);
      }
      last_br->change_use(loop_info.end_reg, new_end);
      loop_info.new_end_reg = new_end; // save for later use
    }
    loop_unroll(func, loop, loop_info, loop_bbs, unroll_cnt);
  }
}

void loop_unroll(ir::Program *prog) {
  for(auto &func : prog->functions) {
    loop_unroll(&func.second);
  }
}

}
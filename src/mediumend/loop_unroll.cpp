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

Instruction *copy_inst(Instruction *inst, BasicBlock *entry, BasicBlock *exit) {
  auto reg_map_if = [=](Reg reg) {
    if (reg_map[map_curid].count(reg)) return reg_map[map_curid].at(reg);
    else return reg;
  };
  auto bb_map_to = [=](BasicBlock *bb) {
    if (bb == entry) return entry;
    if (bb == exit) return exit;
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

void copy_bb(BasicBlock *bb, BasicBlock *new_bb, BasicBlock *entry, BasicBlock *exit) {
  // map prev & succ
  for (auto prev_bb : bb->prev) {
    if (bb == entry) continue; // new_entry's prev has been done
    new_bb->prev.insert(bb_map[map_curid].at(prev_bb));
  }
  for (auto succ_bb : bb->succ) {
    if (succ_bb == exit || succ_bb == entry) {
      new_bb->succ.insert(succ_bb);
      succ_bb->prev.insert(new_bb);
    } else {
      new_bb->succ.insert(bb_map[map_curid].at(succ_bb));
    }
  }
  // copy insts
  for (auto &insn : bb->insns) {
    new_bb->push_back(copy_inst(insn.get(), entry, exit));
  }
}

void loop_unroll(ir::Function * func) {
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
    map_curid = 0;
    bb_map[0].clear();
    bb_map[1].clear();
    reg_map[0].clear();
    reg_map[1].clear();
    for (auto bb : loop_bbs) {
      bb_map[map_curid][bb] = bb;
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          reg_map[map_curid][output->dst] = output->dst;
        }
      }
    }
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
    for (int l = 1; l < UNROLL_CNT; l++) {
      map_curid ^= 1;
      // 1. Create new bbs
      for (auto bb : loop_bbs) {
        BasicBlock* new_bb = new BasicBlock;
        new_bb->func = func;
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
            if (br->true_target == entry)
              br->true_target = new_entry;
            if (br->false_target == entry)
              br->false_target = new_entry;
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
        copy_bb(bb, bb_map[map_curid][bb], loop->header, exit_bb);
      }
      // 4. Modify entry's phi (Delayed), exit's phi
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
    // Modify Loop0 entry's phi & prev
    vector<BasicBlock *> entry_prev_bb_to_insert;
    for (auto entry_prev_bb : loop->header->prev) {
      if (entry_prev_bb->loop == nullptr || entry_prev_bb->loop != loop) {
        continue; // bb not in loop
      }
      entry_prev_bb_to_insert.push_back(bb_map[map_curid].at(entry_prev_bb));
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
        if (br->true_target == loop->header)
          br->true_target = loop1_entry;
        if (br->false_target == loop->header)
          br->false_target = loop1_entry;
      } else assert(false);
    }
  }
}

void loop_unroll(ir::Program *prog) {
  for(auto &func : prog->functions) {
    loop_unroll(&func.second);
  }
}

}
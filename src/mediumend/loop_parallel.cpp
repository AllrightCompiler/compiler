#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;
using ir::Loop;
using ir::BasicBlock;
using ir::Instruction;

static const int THREAD_NUM = 4;

// Parallel loop:
// - one exit
// - one into edge to entry (into_entry)
// - one back edge to entry (exit_prev)
// - cond_var appear twice (phi, update), updated value only used by phi & cmp (array index only contain simple i)
// - only one phi at entry (cond_var)
// - no call (stack changed)
// - i = x, i < n, i += c
struct ParallelLoopInfo {
  bool valid;
  Reg start_reg; // reg start
  int step; // const step
  Reg end_reg; // reg end
  BasicBlock *into_entry;
  BasicBlock *entry;
  BasicBlock *exit_prev;
  BasicBlock *exit;
  ir::insns::Phi *entry_ind_phi;
  ir::insns::Binary *cond_cmp;
  ir::insns::Binary *binary_upd;

  bool profitable(Loop *loop) {
    if (loop->no_inner) return false; // don't parallel single layer loop
    Reg ind_var = entry_ind_phi->dst;
    for (auto use_i : entry->func->use_list.at(ind_var)) {
      if (use_i == binary_upd) continue;
      TypeCase(dummy, ir::insns::GetElementPtr *, use_i) {
        // i only used for array index
      } else return false;
    }
    return true;
  }
};

ParallelLoopInfo get_parallel_info(Loop *loop, const unordered_set<BasicBlock *> &loop_bbs, BasicBlock *exit) {
  auto in_loop = [=](BasicBlock *b) {
    Loop *l = b->loop;
    while (l != nullptr && l != loop) {
      l = l->outer;
    }
    return l == loop;
  };
  ParallelLoopInfo info;
  info.valid = false;
  info.entry = loop->header;
  info.exit = exit;
  info.exit_prev = nullptr;
  for (auto bb : exit->prev) {
    if (in_loop(bb)) {
      if (info.exit_prev == nullptr) info.exit_prev = bb;
      else return info; // multiple exit_prev
    }
  }
  info.into_entry = nullptr;
  for (auto bb : info.entry->prev) {
    if (in_loop(bb)) {
      if (bb != info.exit_prev) return info; // multiple back edge
    } else {
      if (info.into_entry == nullptr) info.into_entry = bb;
      else return info; // multiple into edge
    }
  }
  int cnt_phi = 0;
  for (auto &insn : info.entry->insns) {
    TypeCase(dummy, ir::insns::Phi *, insn.get()) {
      cnt_phi++;
    } else break;
  }
  if (cnt_phi > 1) return info; // multiple phi at entry
  for (auto &bb : loop_bbs) {
    for (auto &insn : bb->insns) {
      TypeCase(call, ir::insns::Call *, insn.get()) {
        return info; // call in loop
      } else TypeCase(output, ir::insns::Output *, insn.get()) {
        bool flag = true; // all use in loop
        if (output->bb->func->use_list.count(output->dst)) {
          for (auto use : output->bb->func->use_list.at(output->dst)) {
            flag &= in_loop(use->bb);
          }
        }
        if (!flag) return info;
      }
    }
  }
  TypeCase(br_cond, ir::insns::Branch *, info.exit_prev->insns.back().get()) { // br entry, exit
    TypeCase(binary_cond, ir::insns::Binary *, br_cond->bb->func->def_list.at(br_cond->val)) { // i < n
      if (binary_cond->dst.type != Int) return info;
      if (binary_cond->op != BinaryOp::Lt) return info; // TODO: only consider i < n now
      if (!binary_cond->bb->func->has_param(binary_cond->src2) && in_loop(binary_cond->bb->func->def_list.at(binary_cond->src2)->bb)) return info; // end_reg should be region constant
      info.end_reg = binary_cond->src2;
      info.cond_cmp = binary_cond;
      if (binary_cond->bb->func->has_param(binary_cond->src1)) return info;
      TypeCase(binary_update, ir::insns::Binary *, binary_cond->bb->func->def_list.at(binary_cond->src1)) { // i = i + c
        if (binary_update->dst.type != Int) return info;
        if (binary_update->op != BinaryOp::Add) return info; // TODO: only consider i + c now
        if (binary_update->bb->func->use_list.at(binary_update->dst).size() > 2) return info; // updated i only used in cond_cmp & phi
        Reg reg_i = binary_update->src1, reg_c = binary_update->src2;
        if (binary_update->bb->func->has_param(reg_i) || binary_update->bb->func->has_param(reg_c)) return info;
        TypeCase(dummy, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_i)) {
          std::swap(reg_i, reg_c);
        }
        TypeCase(update_imm, ir::insns::LoadImm *, binary_update->bb->func->def_list.at(reg_c)) { // c
          assert(update_imm->imm.type == Int);
          if (update_imm->imm.iv != 1) return info; // TODO: only consider c = 1 now
          info.step = update_imm->imm.iv;
        } else return info; // step not const
        TypeCase(inst_phi, ir::insns::Phi *, binary_update->bb->func->def_list.at(reg_i)) {
          if (inst_phi->incoming.size() != 2) return info;
          if (inst_phi->bb != info.entry) return info;
          Reg reg_i_init;
          for (auto pair : inst_phi->incoming) {
            if (pair.second != binary_update->dst) { // find init
              reg_i_init = pair.second;
              assert(!in_loop(pair.first));
            }
          }
          info.start_reg = reg_i_init;
          info.entry_ind_phi = inst_phi;
        } else return info; // no phi inst
        info.binary_upd = binary_update;
      } else return info; // update not binary
    } else return info; // cond not binary
  } else assert(false);
  info.valid = true;
  return info;
}

static Instruction *copy_inst(ParallelLoopInfo info, Instruction *inst,
                              const unordered_map<BasicBlock *, BasicBlock *> &bb_map, const unordered_map<Reg, Reg> &reg_map) {
  auto reg_map_if = [=](Reg reg) {
    if (reg_map.count(reg)) return reg_map.at(reg);
    else return reg;
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
    auto new_inst = new ir::insns::Branch(reg_map_if(br->val), bb_map.at(br->true_target), bb_map.at(br->false_target));
    return new_inst;
  } else TypeCase(jmp, ir::insns::Jump *, inst) {
    auto new_inst = new ir::insns::Jump(bb_map.at(jmp->target));
    return new_inst;
  } else TypeCase(ret, ir::insns::Return *, inst) {
    auto ret_val = ret->val;
    if (ret_val.has_value()) ret_val.value() = reg_map_if(ret_val.value());
    auto new_inst = new ir::insns::Return(ret_val);
  } else TypeCase(phi, ir::insns::Phi *, inst) {
    auto new_inst = new ir::insns::Phi(reg_map_if(phi->dst), phi->array_ssa);
    for (auto pair : phi->incoming) {
      auto mapped_bb = bb_map.at(pair.first);
      auto mapped_reg = reg_map_if(pair.second);
      new_inst->incoming[mapped_bb] = mapped_reg;
    }
    return new_inst;
  } else assert(false);
}

static void copy_bb(ParallelLoopInfo info, BasicBlock *bb, BasicBlock *new_bb,
                    const unordered_map<BasicBlock *, BasicBlock *> &bb_map, const unordered_map<Reg, Reg> &reg_map) {
  // map prev & succ
  for (auto prev_bb : bb->prev) {
    new_bb->prev.insert(bb_map.at(prev_bb));
  }
  for (auto succ_bb : bb->succ) {
    new_bb->succ.insert(bb_map.at(succ_bb));
  }
  // copy insts
  for (auto &insn : bb->insns) {
    new_bb->push_back(copy_inst(info, insn.get(), bb_map, reg_map));
  }
}

void loop_parallel(ir::Function *func, ParallelLoopInfo &info, Loop *loop, const unordered_set<BasicBlock *> &loop_bbs) {
  debug(std::cerr) << "loop parallel " << func->name << ":" << loop->header->label << std::endl;
  BasicBlock *entry = info.entry;
  BasicBlock *exit = info.exit;
  BasicBlock *into_entry = info.into_entry;
  BasicBlock *exit_prev = info.exit_prev;
  BasicBlock *new_into_entry = new BasicBlock; // determine whether to go parallel
  new_into_entry->func = func;
  new_into_entry->label = "parallel_check_" + entry->label;
  func->bbs.emplace_back(new_into_entry);
  BasicBlock *parallel_entry = new BasicBlock; // into_entry for all parallel loop, for distributing control flow
  BasicBlock *parallel_entries[THREAD_NUM]; // into_entry for parallel loop
  BasicBlock *parallel_exits[THREAD_NUM]; // exit for parallel loop
  parallel_entry->func = func;
  parallel_entry->label = "parallel_entry_" + entry->label;
  for (int i = 0; i < THREAD_NUM; i++) {
    parallel_entries[i] = new BasicBlock;
    parallel_exits[i] = new BasicBlock;
    parallel_entries[i]->func = parallel_exits[i]->func = func;
    parallel_entries[i]->label = "parallel_entry_" + std::to_string(i) + "_" + entry->label;
    parallel_exits[i]->label = "parallel_exit_" + std::to_string(i) + "_" + entry->label;
  }
  func->bbs.emplace_back(parallel_entry);
  for (int i = 0; i < THREAD_NUM; i++) {
    func->bbs.emplace_back(parallel_entries[i]);
    func->bbs.emplace_back(parallel_exits[i]);
  }
  // setup new_into_entry
  {
    new_into_entry->prev.insert(into_entry);
    new_into_entry->succ.insert(parallel_entry);
    new_into_entry->succ.insert(entry);
    Reg parallel_cond = func->new_reg(Int);
    new_into_entry->push_back(new ir::insns::LoadImm(parallel_cond, ConstValue(1))); // TODO: check condition for parallel
    auto br = new ir::insns::Branch(parallel_cond, parallel_entry, entry);
    new_into_entry->push_back(br);
  }
  // setup parallel_entry
  {
    parallel_entry->prev.insert(new_into_entry);
    for (int i = 0; i < THREAD_NUM; i++) {
      parallel_entry->succ.insert(parallel_entries[i]);
    }
    Reg tid = func->new_reg(Int);
    auto syscall = new ir::insns::Call(tid, "__create_threads", {});
    parallel_entry->push_back(syscall);
    std::map<int, BasicBlock *> targets;
    for (int i = 0; i < THREAD_NUM - 1; i++) {
      targets[i] = parallel_entries[i];
    }
    auto jmp = new ir::insns::Switch(tid, targets, parallel_entries[THREAD_NUM - 1]);
    parallel_entry->push_back(jmp);
  }
  Reg new_start, new_end; // store the last
  unordered_map<Reg, Reg> reg_map[THREAD_NUM];
  for (int tid = 0; tid < THREAD_NUM; tid++) {
    unordered_map<BasicBlock *, BasicBlock *> bb_map;
    for (auto bb : loop_bbs) {
      BasicBlock *new_bb = new BasicBlock;
      new_bb->func = func;
      new_bb->label = "parallel_" + std::to_string(tid) + "_" + bb->label;
      func->bbs.emplace_back(new_bb);
      bb_map[bb] = new_bb;
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          Reg new_reg = func->new_reg(output->dst.type);
          reg_map[tid][output->dst] = new_reg;
        }
      }
    }
    // setup parallel_entry_tid
    Reg imm_tid = func->new_reg(Int);
    {
      parallel_entries[tid]->prev.insert(parallel_entry);
      parallel_entries[tid]->succ.insert(bb_map.at(entry));
      Reg range_len = func->new_reg(Int);
      auto range_sub = new ir::insns::Binary(range_len, BinaryOp::Sub, info.end_reg, info.start_reg);
      parallel_entries[tid]->push_back(range_sub);
      auto loadimm_tid = new ir::insns::LoadImm(imm_tid, ConstValue(tid));
      parallel_entries[tid]->push_back(loadimm_tid);
      Reg tid_plus_one = func->new_reg(Int);
      auto tid_inc = new ir::insns::LoadImm(tid_plus_one, ConstValue(tid + 1));
      parallel_entries[tid]->push_back(tid_inc);
      Reg imm_TN = func->new_reg(Int);
      auto loadimm_TN = new ir::insns::LoadImm(imm_TN, ConstValue(THREAD_NUM));
      parallel_entries[tid]->push_back(loadimm_TN);
      Reg mul_l = func->new_reg(Int), mul_r = func->new_reg(Int);
      auto l_mul = new ir::insns::Binary(mul_l, BinaryOp::Mul, range_len, imm_tid);
      auto r_mul = new ir::insns::Binary(mul_r, BinaryOp::Mul, range_len, tid_plus_one);
      parallel_entries[tid]->push_back(l_mul);
      parallel_entries[tid]->push_back(r_mul);
      Reg l = func->new_reg(Int), r = func->new_reg(Int);
      auto l_div = new ir::insns::Binary(l, BinaryOp::Div, mul_l, imm_TN); // l = [(end - start) * tid] / THREAD_NUM
      auto r_div = new ir::insns::Binary(r, BinaryOp::Div, mul_r, imm_TN); // r = [(end - start) * (tid + 1)] / THREAD_NUM
      parallel_entries[tid]->push_back(l_div);
      parallel_entries[tid]->push_back(r_div);
      new_start = func->new_reg(Int), new_end = func->new_reg(Int);
      auto new_start_add = new ir::insns::Binary(new_start, BinaryOp::Add, info.start_reg, l);
      auto new_end_add = new ir::insns::Binary(new_end, BinaryOp::Add, info.start_reg, r);
      parallel_entries[tid]->push_back(new_start_add);
      parallel_entries[tid]->push_back(new_end_add);
      info.entry_ind_phi->change_use(info.start_reg, new_start);
      info.cond_cmp->change_use(info.end_reg, new_end);
      auto jmp = new ir::insns::Jump(bb_map.at(entry));
      parallel_entries[tid]->push_back(jmp);
    }
    // setup parallel_exit_tid
    {
      parallel_exits[tid]->prev.insert(bb_map.at(exit_prev));
      parallel_exits[tid]->succ.insert(exit);
      auto syscall = new ir::insns::Call(func->new_reg(Int), "__join_threads", {imm_tid});
      auto jmp = new ir::insns::Jump(exit);
      parallel_exits[tid]->push_back(syscall);
      parallel_exits[tid]->push_back(jmp);
    }
    bb_map[into_entry] = parallel_entries[tid];
    bb_map[exit] = parallel_exits[tid];
    for (auto bb : loop_bbs) {
      copy_bb(info, bb, bb_map.at(bb), bb_map, reg_map[tid]);
    }
    // change back original loop's start & end
    info.entry_ind_phi->change_use(new_start, info.start_reg);
    info.cond_cmp->change_use(new_end, info.end_reg);
  }
  // into_entry -> entry  ==>  into_entry -> new_into_entry -> entry
  {
    // modify into_entry's succ & terinst from entry to new_into_entry
    into_entry->succ.erase(entry);
    into_entry->succ.insert(new_into_entry);
    TypeCase(into_entry_br, ir::insns::Branch *, into_entry->insns.back().get()) {
      if (into_entry_br->true_target == entry) {
        into_entry_br->true_target = new_into_entry;
      } else if (into_entry_br->false_target == entry) {
        into_entry_br->false_target = new_into_entry;
      } else assert(false);
    } else TypeCase(into_entry_jmp, ir::insns::Jump *, into_entry->insns.back().get()) {
      assert(into_entry_jmp->target == entry);
      into_entry_jmp->target = new_into_entry;
    } else assert(false);
    // modify entry's prev & phi from into_entry to new_into_entry
    entry->prev.erase(into_entry);
    entry->prev.insert(new_into_entry);
    for (auto &insn : entry->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        phi->incoming[new_into_entry] = phi->incoming.at(into_entry);
        phi->incoming.erase(into_entry);
        phi->add_use_def();
      } else break;
    }
  }
  // modify exit
  {
    // add new exit from parallel_exit
    for (int i = 0; i < THREAD_NUM; i++) {
      exit->prev.insert(parallel_exits[i]);
    }
    for (auto &insn : exit->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        phi->remove_use_def();
        Reg new_reg = phi->incoming.at(exit_prev);
        for (int i = 0; i < THREAD_NUM; i++) {
          if (reg_map[i].count(new_reg)) new_reg = reg_map[i].at(new_reg);
          phi->incoming[parallel_exits[i]] = new_reg;
        }
        phi->add_use_def();
      } else break;
    }
  }
}

void loop_parallel(ir::Function *func) {
  func->cfg->build();
  func->loop_analysis();
  for (auto &loop_ptr : func->loops) {
    Loop *loop = loop_ptr.get();
    if (loop->outer != nullptr) { // only perform on outermost loop
      continue;
    }
    unordered_set<BasicBlock *> loop_bbs; // collect loop bbs (recursive)
    auto in_loop = [=](BasicBlock *b) {
      Loop *l = b->loop;
      while (l != nullptr && l != loop) {
        l = l->outer;
      }
      return l == loop;
    };
    vector<BasicBlock *> stack;
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
    BasicBlock *exit_bb = nullptr;
    bool check = true; // check loop has only one exit
    for (auto bb : loop_bbs) {
      if (!check) break;
      for (auto succ_bb : bb->succ) {
        if (!in_loop(succ_bb)) {
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
    // Parallel this loop
    auto loop_info = get_parallel_info(loop, loop_bbs, exit_bb);
    if (!loop_info.valid) continue;
    if (loop_info.profitable(loop)) {
      loop_parallel(func, loop_info, loop, loop_bbs);
    }
  }
}

void loop_parallel(ir::Program *prog) {
  for(auto &func : prog->functions) {
    loop_parallel(&func.second);
  }
  ir_validation(prog);
}

}
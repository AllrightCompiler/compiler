#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <iostream>
#include <variant>

namespace mediumend {

using ir::Function;
using ir::BasicBlock;
using ir::Reg;
using ir::Instruction;
using std::list;
using std::unique_ptr;
using mediumend::CFG;

static ir::Program *program = nullptr;

// try to swap imm to src2 (only for ops with commutativity)
void swapBinaryOprands(ir::insns::Binary *binary) {
  switch (binary->op) {
    case BinaryOp::Add:
    case BinaryOp::Mul:
    case BinaryOp::Eq:
    case BinaryOp::Neq:
      std::swap(binary->src1, binary->src2);
      break;
    case BinaryOp::Lt:
      binary->op = BinaryOp::Gt;
      std::swap(binary->src1, binary->src2);
      break;
    case BinaryOp::Gt:
      binary->op = BinaryOp::Lt;
      std::swap(binary->src1, binary->src2);
      break;
    case BinaryOp::Leq:
      binary->op = BinaryOp::Geq;
      std::swap(binary->src1, binary->src2);
      break;
    case BinaryOp::Geq:
      binary->op = BinaryOp::Leq;
      std::swap(binary->src1, binary->src2);
      break;
    default:
      break;
  }
}

// try to simplify binary
// return const if simplified to const
// return Reg if simplified to reg
std::optional< std::variant<ConstValue, Reg> > simplifyBinary(unordered_map<Reg, ConstValue> &constMap, ir::insns::Binary *binary) {
  Reg src1 = binary->src1, src2 = binary->src2;
  if (constMap.count(src1) && constMap.count(src2)) {
    return const_compute(binary, constMap[src1], constMap[src2]);
  }
  if (!constMap.count(src1) && !constMap.count(src2)) {
    if (src1 == src2) {
      switch (binary->op) {
        case BinaryOp::Sub:
        case BinaryOp::Mod:
        case BinaryOp::Neq:
        case BinaryOp::Lt:
        case BinaryOp::Gt:
          return ConstValue(0);
        case BinaryOp::Div:
        case BinaryOp::Eq:
        case BinaryOp::Leq:
        case BinaryOp::Geq:
          return ConstValue(1);
        default:
          break;
      }
    }
    return std::nullopt;
  }
  // 1 reg + 1 imm
  switch (binary->op) {
    case BinaryOp::Add:
      assert(!constMap.count(src1) && constMap.count(src2));
      if (constMap[src2].isValue(0)) return src1;
      break;
    case BinaryOp::Sub:
      if (constMap.count(src2) && constMap[src2].isValue(0)) return src1;
      break;
    case BinaryOp::Mul:
      assert(!constMap.count(src1) && constMap.count(src2));
      if (constMap[src2].isValue(0)) return ConstValue(0);
      if (constMap[src2].isValue(1)) return src1;
      break;
    case BinaryOp::Div:
      if (constMap.count(src2)) {
        if (constMap[src2].isValue(1)) return src1;
      }
      break;
    case BinaryOp::Mod:
      if (constMap.count(src2)) {
        if (constMap[src2].isValue(1)) return ConstValue(0);
      }
      break;
    default:
      break;
  }
  return std::nullopt;
}

// lookup Value Number of inst, insert when not found
Reg vn_get(unordered_map<Instruction *, Reg> &hashTable,
          unordered_set<Instruction *> &vnSet,
          unordered_map<Reg, ConstValue> &constMap,
          Instruction *inst) {
  if (hashTable.find(inst) != hashTable.end()) return hashTable[inst];
  TypeCase(phi, ir::insns::Phi *, inst) {
    bool find = false;
    for (auto i : vnSet) {
      TypeCase(phi_i, ir::insns::Phi *, i) {
        bool same = true;
        if (phi->incoming.size() != phi_i->incoming.size()) continue;
        for (auto income : phi->incoming) {
          if (phi_i->incoming.count(income.first)) {
            same &= (income.second == phi_i->incoming[income.first]);
          } else {
            same = false;
          }
        }
        if (same) {
          hashTable[inst] = phi_i->dst;
          find = true;
        }
      }
    }
    if (!find) {
      hashTable[inst] = phi->dst;
      vnSet.insert(inst);
    }
  } else TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    bool find = false;
    for (auto i : vnSet) {
      TypeCase(loadimm_i, ir::insns::LoadImm *, i) {
        if (loadimm_i->imm == loadimm->imm) {
          hashTable[inst] = loadimm_i->dst;
          find = true;
        }
      }
    }
    if (!find) {
      hashTable[inst] = loadimm->dst;
      vnSet.insert(inst);
      constMap[hashTable[inst]] = loadimm->imm;
    }
  } else TypeCase(binary, ir::insns::Binary *, inst) {
    bool find = false;
    for (auto i : vnSet) {
      TypeCase(binary_i, ir::insns::Binary *, i) {
        if (binary->op == binary_i->op && binary->src1 == binary_i->src1 && binary->src2 == binary_i->src2) {
          hashTable[inst] = binary_i->dst;
          find = true;
        }
        // check commutativity
        if (binary->op == BinaryOp::Add || 
            binary->op == BinaryOp::Mul || 
            binary->op == BinaryOp::Eq || 
            binary->op == BinaryOp::Neq) {
          if (binary->src1 == binary_i->src2 && binary->src2 == binary_i->src1) {
            hashTable[inst] = binary_i->dst;
            find = true;
          }
        }
        // check +imm and -imm
        if (binary->src1 == binary_i->src1 && constMap.count(binary->src2) && constMap.count(binary_i->src2)) {
          if ((binary->op == BinaryOp::Add && binary_i->op == BinaryOp::Sub) || (binary->op == BinaryOp::Sub && binary_i->op == BinaryOp::Add)) {
            ConstValue c1 = constMap[binary->src2], c2 = constMap[binary_i->src2];
            if (c1.isOpposite(c2)) {
              hashTable[inst] = binary_i->dst;
              find = true;
            }
          }
        }
        // check Lt & Gt, Leq & Geq
        if (binary->src1 == binary_i->src2 && binary->src2 == binary_i->src1) {
          switch (binary->op) {
            case BinaryOp::Lt:
              if (binary_i->op == BinaryOp::Gt) {
                hashTable[inst] = binary_i->dst;
                find = true;
              }
              break;
            case BinaryOp::Gt:
               if (binary_i->op == BinaryOp::Lt) {
                hashTable[inst] = binary_i->dst;
                find = true;
              }
              break;
            case BinaryOp::Leq:
               if (binary_i->op == BinaryOp::Geq) {
                hashTable[inst] = binary_i->dst;
                find = true;
              }
              break;
            case BinaryOp::Geq:
               if (binary_i->op == BinaryOp::Leq) {
                hashTable[inst] = binary_i->dst;
                find = true;
              }
              break;
            default:
              break;
          }
        }
      }
    }
    if (!find) {
      hashTable[inst] = binary->dst;
      vnSet.insert(inst);
    }
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    bool find = false;
    for (auto i : vnSet) {
      TypeCase(loadaddr_i, ir::insns::LoadAddr *, i) {
        if (loadaddr_i->var_name == loadaddr->var_name) {
          hashTable[inst] = loadaddr_i->dst;
          find = true;
        }
      }
    }
    if (!find) {
      hashTable[inst] = loadaddr->dst;
      vnSet.insert(inst);
    }
  } else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
    bool find = false;
    for (auto i : vnSet) {
      TypeCase(gep_i, ir::insns::GetElementPtr *, i) {
        bool same = (gep->base == gep_i->base);
        if (gep->indices.size() != gep_i->indices.size()) continue;
        int n_idx = gep->indices.size();
        for (int i = 0; i < n_idx; i++)
          same &= (gep->indices[i] == gep_i->indices[i]);
        if (same) {
          hashTable[inst] = gep_i->dst;
          find = true;
        }
      }
    }
    if (!find) {
      hashTable[inst] = gep->dst;
      vnSet.insert(inst);
    }
  } else TypeCase(other, ir::insns::Output *, inst) {
    hashTable[inst] = other->dst;
    vnSet.insert(inst);
  } else assert(false);
  return hashTable[inst];
}

// reg_dst = reg_src
// change all reg_dst -> reg_src
void copy_propagation(unordered_map<Reg, list<Instruction *> > &use_list, Reg dst, Reg src) {
  while (use_list[dst].size() > 0) {
    auto inst = use_list[dst].front();
    inst->change_use(dst, src);
  }
}

// Global Value Numbering
void gvn(Function *f) {
  f->cfg->build();
  f->cfg->compute_dom();
  // cfg dom, domby, idom, domlevel set
  f->cfg->compute_rpo();
  // rpo set

  // use reg as value number
  // reg -> inst : f->def_list[reg]
  unordered_map<Instruction *, Reg> hashTable;
  unordered_set<Instruction *> vnSet; // set of all value in hashTable
  unordered_map<Reg, ConstValue> constMap;
  for (auto bb : f->cfg->rpo) {
    for (auto &insn : bb->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        unordered_map <Reg, Reg> regsToChange;
        for (auto income : phi->incoming) {
          Reg new_reg = income.second;
          if (!f->has_param(new_reg)) {
            new_reg = vn_get(hashTable, vnSet, constMap, f->def_list.at(income.second));
          }
          if (income.second != new_reg) {
            regsToChange[income.second] = new_reg;
          }
        }
        for (auto reg_pair : regsToChange) {
          phi->change_use(reg_pair.first, reg_pair.second);
        }
        // check meaningless
        bool meaningless = true;
        Reg first = (*(phi->incoming.begin())).second;
        for (auto income : phi->incoming) {
          meaningless &= (first == income.second);
        }
        if (meaningless) { // all incoming same
          copy_propagation(f->use_list, phi->dst, first);
        }
      }
      TypeCase(loadimm, ir::insns::LoadImm *, insn.get()) {
        Reg new_reg = vn_get(hashTable, vnSet, constMap, loadimm);
        if (new_reg != loadimm->dst) {
          copy_propagation(f->use_list, loadimm->dst, new_reg);
        }
      }
      TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
        Reg new_reg = vn_get(hashTable, vnSet, constMap, loadaddr);
        if (new_reg != loadaddr->dst) {
          copy_propagation(f->use_list, loadaddr->dst, new_reg);
        }
      }
      TypeCase(gep, ir::insns::GetElementPtr *, insn.get()) {
        Reg new_reg = vn_get(hashTable, vnSet, constMap, gep);
        if (new_reg != gep->dst) {
          copy_propagation(f->use_list, gep->dst, new_reg);
        }
      }
      TypeCase(binary, ir::insns::Binary *, insn.get()) {
        Reg new_reg1 = binary->src1;
        if (!f->has_param(binary->src1)) {
          new_reg1 = vn_get(hashTable, vnSet, constMap, f->def_list.at(binary->src1));
        }
        Reg new_reg2 = binary->src2;
        if (!f->has_param(binary->src2)) {
          new_reg2 = vn_get(hashTable, vnSet, constMap, f->def_list.at(binary->src2));
        }
        if (binary->src1 != new_reg1) binary->change_use(binary->src1, new_reg1);
        if (binary->src2 != new_reg2) binary->change_use(binary->src2, new_reg2);
        // try to simplify binary Instruction
        Reg src1 = binary->src1, src2 = binary->src2;
        if (constMap.count(src1) && !constMap.count(src2)) {
          swapBinaryOprands(binary);
          src1 = binary->src1, src2 = binary->src2;
        }
        auto ret = simplifyBinary(constMap, binary);
        if (ret.has_value()) {
          if (ret.value().index() == 0) {
            auto constval = std::get<ConstValue>(ret.value());
            bool find = false;
            for (auto pair : constMap) {
              if (pair.second == constval) {
                copy_propagation(f->use_list, binary->dst, pair.first);
                find = true;
                break;
              }
            }
            if (!find) { // replace binary with loadimm
              auto new_ins = new ir::insns::LoadImm(binary->dst, constval);
              new_ins->bb = binary->bb;
              binary->remove_use_def();
              insn.reset(new_ins);
              new_ins->add_use_def();
              hashTable[insn.get()] = binary->dst;
              vnSet.insert(insn.get());
              constMap[binary->dst] = constval;
            }
          } else {
            auto reg = std::get<Reg>(ret.value());
            copy_propagation(f->use_list, binary->dst, reg);
          }
        }
      }
      // TODO: more inst types
    }
  }
}

// Least Common Ancestor (on dom tree)
BasicBlock *find_lca(BasicBlock *a, BasicBlock *b) {
  if (a == nullptr) return b;
  while (a->domlevel > b->domlevel) a = a->idom;
  while (b->domlevel > a->domlevel) b = b->idom;
  while (a != b) {
    a = a->idom;
    b = b->idom;
  }
  return a;
}

void get_pinned_pred(unordered_set<Instruction *> &visited,
                  unordered_map<Instruction *, unordered_set<Instruction *> > &pred,
                  Instruction *inst);

void update_pred(unordered_set<Instruction *> &visited,
                unordered_map<Instruction *, unordered_set<Instruction *> > &pred,
                Instruction *inst, Reg reg_use) {
  if (inst->bb->func->has_param(reg_use)) return;
  Instruction *inst_use = inst->bb->func->def_list.at(reg_use);
  get_pinned_pred(visited, pred, inst_use);
  if (pred.count(inst_use)) {
    for (auto i : pred[inst_use])
      pred[inst].insert(i);
  }
}

// pinned inst (load, impure call) can not be moved, therefore we need pred of pinned insts for position limit
void get_pinned_pred(unordered_set<Instruction *> &visited,
                  unordered_map<Instruction *, unordered_set<Instruction *> > &pred,
                  Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(load, ir::insns::Load *, inst) {
    update_pred(visited, pred, inst, load->addr);
    for (auto i : load->bb->func->use_list[load->dst]) {
      pred[i].insert(load);
    }
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    for (auto i : loadaddr->bb->func->use_list[loadaddr->dst]) {
      pred[i].insert(loadaddr);
    }
  } else TypeCase(store, ir::insns::Store *, inst) {
    update_pred(visited, pred, inst, store->addr);
    update_pred(visited, pred, inst, store->val);
  } else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
    update_pred(visited, pred, inst, gep->base);
    for (auto idx : gep->indices) {
      update_pred(visited, pred, inst, idx);
    }
  } else TypeCase(convert, ir::insns::Convert *, inst) {
    update_pred(visited, pred, inst, convert->src);
  } else TypeCase(call, ir::insns::Call *, inst) {
    for (auto arg : call->args) {
      update_pred(visited, pred, inst, arg);
    }
    if (!program->functions.count(call->func) || !program->functions[call->func].is_pure()) { // lib func or impure
      for (auto i : call->bb->func->use_list[call->dst]) {
        pred[i].insert(call);
      }
    }
  } else TypeCase(unary, ir::insns::Unary *, inst) {
    update_pred(visited, pred, inst, unary->src);
  } else TypeCase(binary, ir::insns::Binary *, inst) {
    update_pred(visited, pred, inst, binary->src1);
    update_pred(visited, pred, inst, binary->src2);
  } else TypeCase(phi, ir::insns::Phi *, inst) {
    for (auto [_, reg] : phi->incoming) {
      update_pred(visited, pred, inst, reg);
    }
  }
}

// 3. Schedule (select basic blocks for) all instructions
// early, based on existing control and data depend-
// ences. We place instructions in the first block
// where they are dominated by their inputs. This
// schedule has a lot of speculative code, with ex-
// tremely long live ranges.
void schedule_early(unordered_set<ir::Instruction *> &visited,
                    unordered_map<ir::Instruction *, BasicBlock *> &placement,
                    list<unique_ptr<BasicBlock>> &bbs,
                    const unordered_map<Reg, Instruction *> &def_list,
                    ir::BasicBlock *root_bb, ir::Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(binary, ir::insns::Binary *, inst) {
    placement[binary] = root_bb;
    BasicBlock *place1, *place2;
    if (inst->bb->func->has_param(binary->src1)) { // is function param
      place1 = inst->bb->func->bbs.front().get();
    } else {
      ir::Instruction *i1 = def_list.at(binary->src1);
      schedule_early(visited, placement, bbs, def_list, root_bb, i1);
      place1 = placement[i1];
    }
    if (inst->bb->func->has_param(binary->src2)) { // is function param
      place2 = inst->bb->func->bbs.front().get();
    } else {
      ir::Instruction *i2 = def_list.at(binary->src2);
      schedule_early(visited, placement, bbs, def_list, root_bb, i2);
      place2 = placement[i2];
    }
    if (place1->domlevel > placement[binary]->domlevel) {
      placement[binary] = place1;
    }
    if (place2->domlevel > placement[binary]->domlevel) {
      placement[binary] = place2;
    }
  } else TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    placement[loadimm] = root_bb;
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    placement[loadaddr] = root_bb;
  } else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
    placement[gep] = root_bb;
    BasicBlock *place;
    if (inst->bb->func->has_param(gep->base)) {
      place = inst->bb->func->bbs.front().get();
    } else {
      schedule_early(visited, placement, bbs, def_list, root_bb, def_list.at(gep->base));
      place = placement[def_list.at(gep->base)];
    }
    if (place->domlevel > placement[gep]->domlevel) {
      placement[gep] = place;
    }
    for (auto reg : gep->indices) {
      if (inst->bb->func->has_param(reg)) {
        place = inst->bb->func->bbs.front().get();
      } else {
        schedule_early(visited, placement, bbs, def_list, root_bb, def_list.at(reg));
        place = placement[def_list.at(reg)];
      }
      if (place->domlevel > placement[gep]->domlevel) {
        placement[gep] = place;
      }
    }
  } else TypeCase(call, ir::insns::Call *, inst) {
    if (program->functions.count(call->func) && program->functions[call->func].is_pure()) {
      placement[call] = root_bb;
      BasicBlock *place;
      for (auto arg : call->args) {
        if (inst->bb->func->has_param(arg)) {
          place = inst->bb->func->bbs.front().get();
        } else {
          schedule_early(visited, placement, bbs, def_list, root_bb, def_list.at(arg));
          place = placement[def_list.at(arg)];
        }
        if (place->domlevel > placement[call]->domlevel) {
          placement[call] = place;
        }
      }
    } else {
      placement[call] = inst->bb;
    }
  } else {
    placement[inst] = inst->bb;
  }
  // TODO: more inst types
}

// 4. Schedule all instructions late. We place instruc-
// tions in the last block where they dominate all
// their uses.
// 5. Between the early schedule and the late schedule
// we have a safe range to place computations. We
// choose the block that is in the shallowest loop nest
// possible, and then is as control dependent as possible.
void schedule_late(unordered_set<ir::Instruction *> &visited,
                  unordered_map<ir::Instruction *, BasicBlock *> &placement,
                  const CFG *cfg,
                  list<unique_ptr<BasicBlock>> &bbs,
                  const unordered_map<Reg, list<Instruction *>> &use_list,
                  const unordered_map<Instruction*, unordered_set<Instruction *>> &pred,
                  ir::Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(phi, ir::insns::Phi *, inst) {
    return;
  } else TypeCase(load, ir::insns::Load *, inst) {
    if (use_list.count(load->dst)) {
      for (auto i : use_list.at(load->dst)) {
        schedule_late(visited, placement, cfg, bbs, use_list, pred, i);
      }
    }
    return;
  } else TypeCase(call, ir::insns::Call *, inst) {
    if (use_list.count(call->dst)) {
      for (auto i : use_list.at(call->dst)) {
        schedule_late(visited, placement, cfg, bbs, use_list, pred, i);
      }
    }
    return;
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    if (use_list.count(loadaddr->dst)) {
      for (auto i : use_list.at(loadaddr->dst)) {
        schedule_late(visited, placement, cfg, bbs, use_list, pred, i);
      }
    }
    return;
  } else TypeCase(output, ir::insns::Output *, inst) {
    // Find latest legal block for instruction
    BasicBlock *lca = nullptr, *use;
    if(use_list.count(output->dst)) {
      for (auto i : use_list.at(output->dst)) {
        schedule_late(visited, placement, cfg, bbs, use_list, pred, i);
        TypeCase(phi, ir::insns::Phi *, i) {
          for (auto pair : phi->incoming) {
            if (pair.second == output->dst) {
              use = pair.first;
            }
          }
        } else {
          use = placement[i];
        }
        lca = find_lca(lca, use);
      }
    }
    
    // Pick final position
    if (lca == nullptr) return; // no use
    BasicBlock *best = lca;
    if (lca != placement[inst]) {
      do {
        lca = lca->idom;
        if (lca->get_loop_level() < best->get_loop_level()) {
          best = lca;
        }
      } while (lca != placement[inst]);
    }
    inst->bb->remove(inst);
    if (pred.count(inst)) {
      best->insert_after(pred.at(inst), inst);
    } else {
      best->insert_after_phi(inst);
    }
    placement[inst] = best;
  }
}

// Global Code Motion
void gcm(Function *f) {
  f->cfg->loop_analysis();

  vector<ir::Instruction *> all_insts;

  for (auto &bb : f->bbs)
    for (auto &insn : bb->insns)
      all_insts.push_back(insn.get());
  
  unordered_set<ir::Instruction *> visited;
  unordered_map<ir::Instruction *, BasicBlock *> placement;
  unordered_map<Instruction*, unordered_set<Instruction *>> pred;
  for (auto inst : all_insts) {
    get_pinned_pred(visited, pred, inst);
  }
  visited.clear();
  for (auto inst : all_insts) {
    schedule_early(visited, placement, f->bbs, f->def_list, f->bbs.front().get(), inst);
  }
  visited.clear();
  for (auto inst : all_insts) {
    schedule_late(visited, placement, f->cfg, f->bbs, f->use_list, pred, inst);
  }
}

void gvn_gcm(ir::Program *prog) {
  program = prog;
  for(auto &func : prog->functions){
    func.second.cfg->remove_unreachable_bb();
    // std::cout << "func: " << func.first << std::endl;
    gvn(&func.second);
    gcm(&func.second);
  }
}

}


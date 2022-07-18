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
using std::tuple;
using std::vector;
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
std::optional< std::variant<ConstValue, Reg> > simplifyBinary(const unordered_map<Reg, ConstValue> &constMap, ir::insns::Binary *binary) {
  Reg src1 = binary->src1, src2 = binary->src2;
  if (constMap.count(src1) && constMap.count(src2)) {
    return const_compute(binary, constMap.at(src1), constMap.at(src2));
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
      if (constMap.at(src2).isValue(0)) return src1;
      break;
    case BinaryOp::Sub:
      if (constMap.count(src2) && constMap.at(src2).isValue(0)) return src1;
      break;
    case BinaryOp::Mul:
      assert(!constMap.count(src1) && constMap.count(src2));
      if (constMap.at(src2).isValue(0)) return ConstValue(0);
      if (constMap.at(src2).isValue(1)) return src1;
      break;
    case BinaryOp::Div:
      if (constMap.count(src2)) {
        if (constMap.at(src2).isValue(1)) return src1;
      }
      break;
    case BinaryOp::Mod:
      if (constMap.count(src2)) {
        if (constMap.at(src2).isValue(1)) return ConstValue(0);
      }
      break;
    default:
      break;
  }
  return std::nullopt;
}

// use reg as value number
static unordered_map<ConstValue, Reg> hashTable_loadimm;
static unordered_map<tuple<BinaryOp, Reg, Reg>, Reg> hashTable_binary;
static unordered_map<std::string, Reg> hashTable_loadaddr;
static unordered_map<tuple<Reg, vector<Reg> >, Reg> hashTable_gep;
static unordered_map<tuple<std::string, vector<Reg> >, Reg> hashTable_call;
static unordered_map<Reg, ConstValue> constMap;
static unordered_map<ConstValue, Reg> rConstMap;
static unordered_map<Instruction *, Reg> hashTable;

// lookup Value Number of inst, insert when not found
Reg vn_get(Instruction *inst) {
  if (hashTable.find(inst) != hashTable.end()) return hashTable[inst];
  TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    if (hashTable_loadimm.count(loadimm->imm)) {
      hashTable[inst] = hashTable_loadimm[loadimm->imm];
    } else {
      hashTable[inst] = loadimm->dst;
      hashTable_loadimm[loadimm->imm] = loadimm->dst;
      constMap[loadimm->dst] = loadimm->imm;
      rConstMap[loadimm->imm] = loadimm->dst;
    }
  } else TypeCase(binary, ir::insns::Binary *, inst) {
    bool find = false;
    auto check_binary = [&](BinaryOp op, Reg r1, Reg r2) {
      if (hashTable_binary.count(std::make_tuple(op, r1, r2))) {
        hashTable[inst] = hashTable_binary[std::make_tuple(op, r1, r2)];
        find = true;
      }
    };
    check_binary(binary->op, binary->src1, binary->src2);
    // check commutativity
    if (binary->op == BinaryOp::Add || 
        binary->op == BinaryOp::Mul || 
        binary->op == BinaryOp::Eq || 
        binary->op == BinaryOp::Neq) {
      check_binary(binary->op, binary->src2, binary->src1);
    }
    // check +imm and -imm
    if (constMap.count(binary->src2) && rConstMap.count(constMap.at(binary->src2).getOpposite())) {
      if (binary->op == BinaryOp::Add) {
        check_binary(BinaryOp::Sub, binary->src1, rConstMap.at(constMap.at(binary->src2).getOpposite()));
      }
      if (binary->op == BinaryOp::Sub) {
        check_binary(BinaryOp::Add, binary->src1, rConstMap.at(constMap.at(binary->src2).getOpposite()));
      }
    }
    // check Lt & Gt, Leq & Geq
    switch (binary->op) {
      case BinaryOp::Lt:
        check_binary(BinaryOp::Gt, binary->src2, binary->src1);
        break;
      case BinaryOp::Gt:
        check_binary(BinaryOp::Lt, binary->src2, binary->src1);
        break;
      case BinaryOp::Leq:
        check_binary(BinaryOp::Geq, binary->src2, binary->src1);
        break;
      case BinaryOp::Geq:
        check_binary(BinaryOp::Leq, binary->src2, binary->src1);
        break;
      default:
        break;
    }
    if (!find) {
      hashTable[inst] = binary->dst;
      hashTable_binary[std::make_tuple(binary->op, binary->src1, binary->src2)] = binary->dst;
    }
  } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
    if (hashTable_loadaddr.count(loadaddr->var_name)) {
      hashTable[inst] = hashTable_loadaddr[loadaddr->var_name];
    } else {
      hashTable[inst] = loadaddr->dst;
      hashTable_loadaddr[loadaddr->var_name] = loadaddr->dst;
    }
  } else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
    if (hashTable_gep.count(std::make_tuple(gep->base, gep->indices))) {
      hashTable[inst] = hashTable_gep[make_tuple(gep->base, gep->indices)];
    } else {
      hashTable[inst] = gep->dst;
      hashTable_gep[make_tuple(gep->base, gep->indices)] = gep->dst;
    }
  } else TypeCase(call, ir::insns::Call *, inst) {
    if (program->functions.count(call->func) && program->functions.at(call->func).is_pure()) {
      if (hashTable_call.count(std::make_tuple(call->func, call->args))) {
        hashTable[inst] = hashTable_call[make_tuple(call->func, call->args)];
      } else {
        hashTable[inst] = call->dst;
        hashTable_call[make_tuple(call->func, call->args)] = call->dst;
      }
    } else {
      hashTable[inst] = call->dst;
    }
  } else TypeCase(other, ir::insns::Output *, inst) {
    hashTable[inst] = other->dst;
  } else assert(false);
  return hashTable[inst];
}

// reg_dst = reg_src
// change all reg_dst -> reg_src
void copy_propagation(unordered_map<Reg, unordered_set<Instruction *> > &use_list, Reg dst, Reg src) {
  while (use_list[dst].size() > 0) {
    auto inst = *use_list[dst].begin();
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

  for (auto bb : f->cfg->rpo) {
    unordered_map<Reg, Reg> loadMap; // only in same bb
    for (auto &insn : bb->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        unordered_map <Reg, Reg> regsToChange;
        for (auto income : phi->incoming) {
          Reg new_reg = income.second;
          if (!f->has_param(new_reg)) {
            new_reg = vn_get(f->def_list.at(income.second));
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
        Reg new_reg = vn_get(loadimm);
        if (new_reg != loadimm->dst) {
          copy_propagation(f->use_list, loadimm->dst, new_reg);
        }
      }
      TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
        Reg new_reg = vn_get(loadaddr);
        if (new_reg != loadaddr->dst) {
          copy_propagation(f->use_list, loadaddr->dst, new_reg);
        }
      }
      TypeCase(gep, ir::insns::GetElementPtr *, insn.get()) {
        Reg new_base = gep->base;
        if (!f->has_param(gep->base)) {
          new_base = vn_get(f->def_list.at(gep->base));
        }
        if (new_base != gep->base) gep->change_use(gep->base, new_base);
        for (auto idx : gep->indices) {
          Reg new_idx = idx;
          if (!f->has_param(idx)) {
            new_idx = vn_get(f->def_list.at(idx));
          }
          if (new_idx != idx) gep->change_use(idx, new_idx);
        }
        Reg new_reg = vn_get(gep);
        if (new_reg != gep->dst) {
          copy_propagation(f->use_list, gep->dst, new_reg);
        }
      }
      TypeCase(binary, ir::insns::Binary *, insn.get()) {
        Reg new_reg1 = binary->src1;
        if (!f->has_param(binary->src1)) {
          new_reg1 = vn_get(f->def_list.at(binary->src1));
        }
        Reg new_reg2 = binary->src2;
        if (!f->has_param(binary->src2)) {
          new_reg2 = vn_get(f->def_list.at(binary->src2));
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
            if (rConstMap.count(constval)) {
              copy_propagation(f->use_list, binary->dst, rConstMap.at(constval));
            } else { // replace binary with loadimm
              auto new_ins = new ir::insns::LoadImm(binary->dst, constval);
              new_ins->bb = binary->bb;
              binary->remove_use_def();
              insn.reset(new_ins);
              new_ins->add_use_def();
              hashTable[insn.get()] = binary->dst;
              hashTable_loadimm[constval] = binary->dst;
              constMap[binary->dst] = constval;
              rConstMap[constval] = binary->dst;
            }
          } else {
            auto reg = std::get<Reg>(ret.value());
            copy_propagation(f->use_list, binary->dst, reg);
          }
        }
      }
      TypeCase(call, ir::insns::Call *, insn.get()) {
        for (auto arg : call->args) {
          Reg new_arg = arg;
          if (!f->has_param(arg)) {
            new_arg = vn_get(f->def_list.at(arg));
          }
          if (new_arg != arg) call->change_use(arg, new_arg);
        }
        Reg new_reg = vn_get(call);
        if (new_reg != call->dst) {
          copy_propagation(f->use_list, call->dst, new_reg);
        }
      }
      TypeCase(load, ir::insns::Load *, insn.get()) {
        Reg new_addr = load->addr;
        if (!f->has_param(load->addr)) {
          new_addr = vn_get(f->def_list.at(load->addr));
        }
        if (load->addr != new_addr) load->change_use(load->addr, new_addr);
        if (loadMap.count(load->addr)) {
          copy_propagation(f->use_list, load->dst, loadMap.at(load->addr));
        } else {
          loadMap[load->addr] = load->dst;
        }
      }
      TypeCase(store, ir::insns::Store *, insn.get()) {
        loadMap.clear();
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

// pinned inst (phi, load, impure call) can not be moved
bool is_pinned(Instruction *inst) {
  TypeCase(load, ir::insns::Load *, inst) return true;
  TypeCase(phi, ir::insns::Phi *, inst) return true;
  TypeCase(call, ir::insns::Call *, inst) {
    if (program->functions.count(call->func) && program->functions.at(call->func).is_pure()) {
      return false;
    } else {
      return true;
    }
  }
  return false;
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
  if (inst->bb != placement[inst]) {
    inst->bb->remove(inst);
    placement[inst]->insert_before_ter(inst);
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
                  const unordered_map<Reg, unordered_set<Instruction *>> &use_list,
                  ir::Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(output, ir::insns::Output *, inst) {
    if (is_pinned(inst)) return;
    // Find latest legal block for instruction
    BasicBlock *lca = nullptr, *use;
    if(use_list.count(output->dst)) {
      for (auto i : use_list.at(output->dst)) {
        schedule_late(visited, placement, cfg, bbs, use_list, i);
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
    if (inst->bb != best) {
      inst->bb->remove(inst);
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
  for (auto inst : all_insts) {
    schedule_early(visited, placement, f->bbs, f->def_list, f->bbs.front().get(), inst);
  }
  visited.clear();
  for (auto inst : all_insts) {
    schedule_late(visited, placement, f->cfg, f->bbs, f->use_list, inst);
  }
}

void gvn_gcm(ir::Program *prog) {
  program = prog;
  for(auto &func : prog->functions){
    hashTable_loadimm.clear();
    hashTable_binary.clear();
    hashTable_loadaddr.clear();
    hashTable_gep.clear();
    hashTable_call.clear();
    constMap.clear();
    rConstMap.clear();
    hashTable.clear();
    func.second.cfg->remove_unreachable_bb();
    gvn(&func.second);
    gcm(&func.second);
  }
}

}


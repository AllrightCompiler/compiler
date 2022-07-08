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
        Reg first = (*phi_i->incoming.begin()).second;
        for (auto income : phi_i->incoming) {
          same &= (first == income.second);
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

// Least Common Ancestor (on dom tree)
BasicBlock *find_lca(const unordered_map<BasicBlock *, int> &domlevel,
                    const unordered_map<BasicBlock *, BasicBlock *> &idom,
                    BasicBlock *a, BasicBlock *b) {
  if (a == nullptr) return b;
  while (domlevel.at(a) > domlevel.at(b)) a = idom.at(a);
  while (domlevel.at(b) > domlevel.at(a)) b = idom.at(b);
  while (a != b) {
    a = idom.at(a);
    b = idom.at(b);
  }
  return a;
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
                    const unordered_map<BasicBlock *, int> &domlevel,
                    ir::BasicBlock *root_bb, ir::Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(binary, ir::insns::Binary *, inst) {
    placement[binary] = root_bb;
    ir::Instruction *i1 = def_list.at(binary->src1);
    ir::Instruction *i2 = def_list.at(binary->src2);
    schedule_early(visited, placement, bbs, def_list, domlevel, root_bb, i1);
    schedule_early(visited, placement, bbs, def_list, domlevel, root_bb, i2);
    if (domlevel.at(placement[i1]) > domlevel.at(placement[binary])) {
      placement[binary] = placement[i1];
    }
    if (domlevel.at(placement[i2]) > domlevel.at(placement[binary])) {
      placement[binary] = placement[i2];
    }
  } else TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    placement[loadimm] = root_bb;
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
                  ir::Instruction *inst) {
  if (visited.count(inst)) return;
  visited.insert(inst);
  TypeCase(phi, ir::insns::Phi *, inst) {
    return;
  } else TypeCase(output, ir::insns::Output *, inst) {
    // Find latest legal block for instruction
    BasicBlock *lca = nullptr, *use;
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
      lca = find_lca(cfg->domlevel, cfg->idom, lca, use);
    }
    // Pick final position
    if (lca == nullptr) return; // no use
    BasicBlock *best = lca;
    if (lca != placement[inst]) {
      do {
        lca = cfg->idom.at(lca);
        if (cfg->get_loop_level(lca) < cfg->get_loop_level(best)) {
          best = lca;
        }
      } while (lca != placement[inst]);
    }
    inst->bb->remove(inst);
    best->insert_after_phi(inst);
    placement[inst] = best;
  }
}

void gvn_gcm(Function *f) {
  // Global Value Numbering

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
          Reg new_reg = vn_get(hashTable, vnSet, constMap, f->def_list[income.second]);
          if (!(income.second == new_reg)) {
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
      TypeCase(binary, ir::insns::Binary *, insn.get()) {
        Reg new_reg1 = vn_get(hashTable, vnSet, constMap, f->def_list[binary->src1]);
        Reg new_reg2 = vn_get(hashTable, vnSet, constMap, f->def_list[binary->src2]);
        if (!(binary->src1 == new_reg1)) binary->change_use(binary->src1, new_reg1);
        if (!(binary->src2 == new_reg2)) binary->change_use(binary->src2, new_reg2);
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

  // Global Code Motion

  f->cfg->loop_analysis();

  vector<ir::Instruction *> all_insts;

  for (auto &bb : f->bbs)
    for (auto &insn : bb->insns)
      all_insts.push_back(insn.get());
  
  unordered_set<ir::Instruction *> visited;
  unordered_map<ir::Instruction *, BasicBlock *> placement;
  for (auto inst : all_insts) {
    schedule_early(visited, placement, f->bbs, f->def_list, f->cfg->domlevel, f->bbs.front().get(), inst);
  }
  visited.clear();
  for (auto inst : all_insts) {
    schedule_late(visited, placement, f->cfg, f->bbs, f->use_list, inst);
  }
}

void gvn_gcm(ir::Program *prog) {
  for(auto &func : prog->functions){
    // std::cout << "func: " << func.first << std::endl;
    gvn_gcm(&func.second);
  }
}

}


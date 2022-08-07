#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::Reg;
using ir::BasicBlock;
using std::make_pair;
using std::make_tuple;

static bool changed;

struct ValueRange {
  unordered_map<BinaryOp, unordered_set<Reg> > bounds;

  bool empty() const { return bounds.size() == 0; }

  void update(BinaryOp op, Reg val, bool change_flag) {
    if (!bounds.count(op)) {
      bounds[op] = unordered_set<Reg>();
    }
    if (!bounds.at(op).count(val)) {
      bounds.at(op).insert(val);
      if (change_flag) changed = true;
    }
  }

  void intersect_l(const unordered_map<BinaryOp, unordered_set<Reg> > &_bds) {
    if (bounds.count(BinaryOp::Lt)) {
      if (!_bds.count(BinaryOp::Lt)) bounds.erase(BinaryOp::Lt);
      else {
        auto &st = bounds.at(BinaryOp::Lt);
        auto &st2 = _bds.at(BinaryOp::Lt);
        for (auto it = st.begin(); it != st.end(); ) {
          if (!st2.count(*it)) it = st.erase(it);
          else it++;
        }
        if (st.size() == 0) bounds.erase(BinaryOp::Lt);
      }
    }
    if (bounds.count(BinaryOp::Leq)) {
      if (!_bds.count(BinaryOp::Leq) && !_bds.count(BinaryOp::Lt)) bounds.erase(BinaryOp::Leq);
      else {
        auto &st = bounds.at(BinaryOp::Leq);
        unordered_set<Reg> st2;
        if (_bds.count(BinaryOp::Leq)) {
          st2.insert(_bds.at(BinaryOp::Leq).begin(), _bds.at(BinaryOp::Leq).end());
        }
        if (_bds.count(BinaryOp::Lt)) {
          st2.insert(_bds.at(BinaryOp::Lt).begin(), _bds.at(BinaryOp::Lt).end());
        }
        for (auto it = st.begin(); it != st.end(); ) {
          if (!st2.count(*it)) it = st.erase(it);
          else it++;
        }
        if (st.size() == 0) bounds.erase(BinaryOp::Leq);
      }
    }
  }

  void intersect_g(const unordered_map<BinaryOp, unordered_set<Reg> > &_bds) {
    if (bounds.count(BinaryOp::Gt)) {
      if (!_bds.count(BinaryOp::Gt)) bounds.erase(BinaryOp::Gt);
      else {
        auto &st = bounds.at(BinaryOp::Gt);
        auto &st2 = _bds.at(BinaryOp::Gt);
        for (auto it = st.begin(); it != st.end(); ) {
          if (!st2.count(*it)) it = st.erase(it);
          else it++;
        }
        if (st.size() == 0) bounds.erase(BinaryOp::Gt);
      }
    }
    if (bounds.count(BinaryOp::Geq)) {
      if (!_bds.count(BinaryOp::Geq) && !_bds.count(BinaryOp::Gt)) bounds.erase(BinaryOp::Geq);
      else {
        auto &st = bounds.at(BinaryOp::Geq);
        unordered_set<Reg> st2;
        if (_bds.count(BinaryOp::Geq)) {
          st2.insert(_bds.at(BinaryOp::Geq).begin(), _bds.at(BinaryOp::Geq).end());
        }
        if (_bds.count(BinaryOp::Gt)) {
          st2.insert(_bds.at(BinaryOp::Gt).begin(), _bds.at(BinaryOp::Gt).end());
        }
        for (auto it = st.begin(); it != st.end(); ) {
          if (!st2.count(*it)) it = st.erase(it);
          else it++;
        }
        if (st.size() == 0) bounds.erase(BinaryOp::Geq);
      }
    }
  }

  void intersect_inc_indvar(const unordered_map<BinaryOp, unordered_set<Reg> > &_bds) {
    if (bounds.count(BinaryOp::Gt)) bounds.erase(BinaryOp::Gt);
    if (bounds.count(BinaryOp::Geq)) bounds.erase(BinaryOp::Geq);
    if (_bds.count(BinaryOp::Gt)) bounds[BinaryOp::Gt] = _bds.at(BinaryOp::Gt);
    if (_bds.count(BinaryOp::Geq) || _bds.count(BinaryOp::Gt)) {
      bounds[BinaryOp::Geq] = unordered_set<Reg>();
    }
    if (_bds.count(BinaryOp::Geq)) {
      bounds[BinaryOp::Geq].insert(_bds.at(BinaryOp::Geq).begin(), _bds.at(BinaryOp::Geq).end());
    }
    if (_bds.count(BinaryOp::Gt)) {
      bounds[BinaryOp::Geq].insert(_bds.at(BinaryOp::Gt).begin(), _bds.at(BinaryOp::Gt).end());
    }
    intersect_l(_bds);
  }

  void intersect_dec_indvar(const unordered_map<BinaryOp, unordered_set<Reg> > &_bds) {
    if (bounds.count(BinaryOp::Lt)) bounds.erase(BinaryOp::Lt);
    if (bounds.count(BinaryOp::Leq)) bounds.erase(BinaryOp::Leq);
    if (_bds.count(BinaryOp::Lt)) bounds[BinaryOp::Lt] = _bds.at(BinaryOp::Lt);
    if (_bds.count(BinaryOp::Leq) || _bds.count(BinaryOp::Lt)) {
      bounds[BinaryOp::Leq] = unordered_set<Reg>();
    }
    if (_bds.count(BinaryOp::Leq)) {
      bounds[BinaryOp::Leq].insert(_bds.at(BinaryOp::Leq).begin(), _bds.at(BinaryOp::Leq).end());
    }
    if (_bds.count(BinaryOp::Lt)) {
      bounds[BinaryOp::Leq].insert(_bds.at(BinaryOp::Lt).begin(), _bds.at(BinaryOp::Lt).end());
    }
    intersect_g(_bds);
  }

  void intersect(const ValueRange &range, Reg self) {
    // dec loop inductive var
    if (bounds.count(BinaryOp::Lt) && bounds.at(BinaryOp::Lt).count(self)) {
      intersect_dec_indvar(range.bounds);
      return;
    } else if (range.bounds.count(BinaryOp::Lt) && range.bounds.at(BinaryOp::Lt).count(self)) {
      unordered_map<BinaryOp, unordered_set<Reg> > bds = bounds;
      intersect_dec_indvar(bds);
      return;
    }
    // inc loop inductive var
    if (bounds.count(BinaryOp::Gt) && bounds.at(BinaryOp::Gt).count(self)) {
      intersect_inc_indvar(range.bounds);
      return;
    } else if (range.bounds.count(BinaryOp::Gt) && range.bounds.at(BinaryOp::Gt).count(self)) {
      unordered_map<BinaryOp, unordered_set<Reg> > bds = bounds;
      intersect_inc_indvar(bds);
      return;
    }
    // intersect
    intersect_l(range.bounds);
    intersect_g(range.bounds);
  }

  void unite(const ValueRange &range) {
    if (range.bounds.count(BinaryOp::Lt)) {
      for (auto v : range.bounds.at(BinaryOp::Lt)) {
        update(BinaryOp::Lt, v, false);
      }
    }
    if (range.bounds.count(BinaryOp::Gt)) {
      for (auto v : range.bounds.at(BinaryOp::Gt)) {
        update(BinaryOp::Gt, v, false);
      }
    }
    if (range.bounds.count(BinaryOp::Leq)) {
      for (auto v : range.bounds.at(BinaryOp::Leq)) {
        update(BinaryOp::Leq, v, false);
      }
    }
    if (range.bounds.count(BinaryOp::Geq)) {
      for (auto v : range.bounds.at(BinaryOp::Geq)) {
        update(BinaryOp::Geq, v, false);
      }
    }
  }

  unordered_set<Reg> get_regs(BinaryOp op) {
    unordered_set<Reg> ret;
    if (bounds.count(op)) {
      ret.insert(bounds.at(op).begin(), bounds.at(op).end());
    }
    if (op == BinaryOp::Leq) {
      if (bounds.count(BinaryOp::Lt)) {
        ret.insert(bounds.at(BinaryOp::Lt).begin(), bounds.at(BinaryOp::Lt).end());
      }
    }
    if (op == BinaryOp::Geq) {
      if (bounds.count(BinaryOp::Gt)) {
        ret.insert(bounds.at(BinaryOp::Gt).begin(), bounds.at(BinaryOp::Gt).end());
      }
    }
    return ret;
  }

};

// reg's range at bb
static unordered_map<std::pair<Reg, BasicBlock*>, ValueRange> range_set;
// reg's range at bb from bb_prev
static unordered_map<std::tuple<Reg, BasicBlock*, BasicBlock*>, ValueRange> trans_range_set;
static unordered_map<std::pair<Reg, BasicBlock*>, bool> val_set;
static unordered_map<Reg, int> constIntMap;

void update_bound(BinaryOp op, BasicBlock *bb, BasicBlock *bb_prev, Reg reg, Reg val) {
  if (bb_prev == nullptr || bb->prev.size() == 1) {
    auto pr = make_pair(reg, bb);
    if (!range_set.count(pr)) {
      range_set[pr] = ValueRange();
    }
    range_set.at(pr).update(op, val, true);
  } else {
    auto tp = make_tuple(reg, bb, bb_prev);
    if (!trans_range_set.count(tp)) {
      trans_range_set[tp] = ValueRange();
    }
    trans_range_set.at(tp).update(op, val, true);
  }
}

void update_bound(BasicBlock *bb, Reg reg, const ValueRange &range) {
  if (range.empty()) return;
  auto pr = make_pair(reg, bb);
  if (!range_set.count(pr)) {
    range_set[pr] = ValueRange();
  }
  if (range.bounds.count(BinaryOp::Lt)) {
    for (auto v : range.bounds.at(BinaryOp::Lt))
      range_set.at(pr).update(BinaryOp::Lt, v, true);
  }
  if (range.bounds.count(BinaryOp::Gt)) {
    for (auto v : range.bounds.at(BinaryOp::Gt))
      range_set.at(pr).update(BinaryOp::Gt, v, true);
  }
  if (range.bounds.count(BinaryOp::Leq)) {
    for (auto v : range.bounds.at(BinaryOp::Leq))
      range_set.at(pr).update(BinaryOp::Leq, v, true);
  }
  if (range.bounds.count(BinaryOp::Geq)) {
    for (auto v : range.bounds.at(BinaryOp::Geq))
      range_set.at(pr).update(BinaryOp::Geq, v, true);
  }
}

void update_val(BasicBlock *bb, Reg reg, bool val) {
  auto pr = make_pair(reg, bb);
  if (val_set.count(pr)) {
    assert(val_set.at(pr) == val);
  } else {
    val_set[pr] = val;
    changed = true;
  }
}

bool exist_bound(BasicBlock *bb, Reg reg) {
  auto tp = make_pair(reg, bb);
  return range_set.count(tp);
}

// get value range (union along dom tree ancestors)
ValueRange get_bound(BasicBlock *bb, Reg reg) {
  ValueRange range;
  auto pr = make_pair(reg, bb);
  if (range_set.count(pr)) range = range_set.at(pr);
  while (bb->idom != nullptr) {
    bb = bb->idom;
    pr = make_pair(reg, bb);
    if (range_set.count(pr)) {
      range.unite(range_set.at(pr));
    }
  }
  return range;
}

ValueRange get_trans_bound(BasicBlock *bb, BasicBlock *bb_prev, Reg reg) {
  auto tp = make_tuple(reg, bb, bb_prev);
  if (trans_range_set.count(tp)) return trans_range_set.at(tp);
  else return ValueRange();
}

// check transitivity recursively, return -1 for not certain
int check_cond(BasicBlock *bb, Reg reg, BinaryOp op, Reg val) {
  unordered_set<Reg> visited, workset;
  ValueRange range = get_bound(bb, reg);
  // check true
  visited = range.get_regs(op);
  if (visited.count(val)) return 1;
  workset = visited;
  while (workset.size() > 0) {
    Reg r = *(workset.begin());
    workset.erase(workset.begin());
    unordered_set<Reg> st = get_bound(bb, r).get_regs(op);
    if (st.count(val)) return 1;
    for (auto v : st) {
      if (!visited.count(v)) {
        workset.insert(v);
        visited.insert(v);
      }
    }
  }
  // check false
  switch (op) {
    case BinaryOp::Lt:
      op = BinaryOp::Geq;
      break;
    case BinaryOp::Gt:
      op = BinaryOp::Leq;
      break;
    case BinaryOp::Leq:
      op = BinaryOp::Gt;
      break;
    case BinaryOp::Geq:
      op = BinaryOp::Lt;
      break;
    default:
      assert(false);
  }
  visited = range.get_regs(op);
  if (visited.count(val)) return 0;
  workset = visited;
  while (workset.size() > 0) {
    Reg r = *(workset.begin());
    workset.erase(workset.begin());
    unordered_set<Reg> st = get_bound(bb, r).get_regs(op);
    if (st.count(val)) return 0;
    for (auto v : st) {
      if (!visited.count(v)) {
        workset.insert(v);
        visited.insert(v);
      }
    }
  }
  return -1;
}

// return -1 for not certain
int check_val(BasicBlock *bb, Reg reg) {
  auto pr = make_pair(reg, bb);
  if (val_set.count(pr)) return val_set.at(pr);
  while (bb->idom != nullptr) {
    bb = bb->idom;
    pr = make_pair(reg, bb);
    if (val_set.count(pr)) return val_set.at(pr);
  }
  return -1;
}

void value_range_analysis(ir::Function *func) {
  func->cfg->build();
  func->cfg->compute_dom();
  range_set.clear();
  trans_range_set.clear();
  val_set.clear();
  constIntMap.clear();
  // collect int constant
  for (auto &bb : func->bbs) {
    for (auto &insn : bb->insns) {
      TypeCase(loadimm, ir::insns::LoadImm *, insn.get()) {
        if (loadimm->imm.type == Int) {
          constIntMap[loadimm->dst] = loadimm->imm.iv;
        }
      }
    }
  }
  // Range Discover
  // 0. loadimm
  for (auto &bb : func->bbs) {
    for (auto &insn : bb->insns) {
      TypeCase(loadimm, ir::insns::LoadImm *, insn.get()) {
        if (loadimm->imm.type == Int) {
          update_bound(BinaryOp::Leq, bb.get(), nullptr, loadimm->dst, loadimm->dst);
          update_bound(BinaryOp::Geq, bb.get(), nullptr, loadimm->dst, loadimm->dst);
        }
      }
    }
  }
  do {
    changed = false;
    // 1. branch from cfg
    for (auto &bb : func->bbs) {
      for (auto prev_bb : bb->prev) {
        auto ter_inst = prev_bb->insns.back().get();
        TypeCase(br, ir::insns::Branch *, ter_inst) {
          if (br->true_target == br->false_target) continue;
          if (br->true_target == bb.get()) {
            if (bb->prev.size() == 1) update_val(bb.get(), br->val, true);
            if (func->has_param(br->val)) continue;
            TypeCase(binary, ir::insns::Binary *, func->def_list.at(br->val)) {
              switch (binary->op) {
                case BinaryOp::Lt:
                  update_bound(BinaryOp::Lt, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Gt, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Leq:
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Gt:
                  update_bound(BinaryOp::Gt, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Lt, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Geq:
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Eq:
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src2, binary->src1);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                default:
                  break;
              }
            }
          } else if (br->false_target == bb.get()) {
            if (bb->prev.size() == 1) update_val(bb.get(), br->val, false);
            if (func->has_param(br->val)) continue;
            TypeCase(binary, ir::insns::Binary *, func->def_list.at(br->val)) {
              switch (binary->op) {
                case BinaryOp::Lt:
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Leq:
                  update_bound(BinaryOp::Gt, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Lt, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Gt:
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Geq:
                  update_bound(BinaryOp::Lt, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Gt, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                case BinaryOp::Neq:
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src1, binary->src2);
                  update_bound(BinaryOp::Leq, bb.get(), prev_bb, binary->src2, binary->src1);
                  update_bound(BinaryOp::Geq, bb.get(), prev_bb, binary->src2, binary->src1);
                  break;
                default:
                  break;
              }
            }
          } else assert(false);
        } else continue;
      }
    }
    // 2. binary with const int
    for (auto &bb : func->bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(binary, ir::insns::Binary *, insn.get()) {
          if (binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) continue;
          if (binary->op == BinaryOp::Add && constIntMap.count(binary->src1)) {
            std::swap(binary->src1, binary->src2);
          }
          if (!constIntMap.count(binary->src2)) continue;
          ConstValue val = constIntMap.at(binary->src2);
          bool f = (val.iv > 0);
          if (binary->op == BinaryOp::Sub) f ^= 1;
          if (f) {
            update_bound(BinaryOp::Gt, bb.get(), nullptr, binary->dst, binary->src1);
            update_bound(BinaryOp::Lt, bb.get(), nullptr, binary->src1, binary->dst);
          } else {
            update_bound(BinaryOp::Lt, bb.get(), nullptr, binary->dst, binary->src1);
            update_bound(BinaryOp::Gt, bb.get(), nullptr, binary->src1, binary->dst);
          }
        }
      }
    }
    // 3. phi inst
    for (auto &bb : func->bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(phi, ir::insns::Phi *, insn.get()) {
          bool undefined = false;
          for (auto pair : phi->incoming) {
            if (!exist_bound(pair.first, pair.second) && get_trans_bound(bb.get(), pair.first, pair.second).empty()) undefined = true;
          }
          if (undefined) continue;
          ValueRange range;
          bool first = true;
          for (auto pair : phi->incoming) {
            ValueRange bound = get_bound(pair.first, pair.second);
            bound.unite(get_trans_bound(bb.get(), pair.first, pair.second));
            if (first) {
              first = false;
              range = bound;
            } else {
              range.intersect(bound, phi->dst);
            }
          }
          update_bound(bb.get(), phi->dst, range);
        } else break;
      }
    }
  } while (changed);
  // check branches
  for (auto &bb : func->bbs) {
    auto ter_inst = bb->insns.back().get();
    TypeCase(br, ir::insns::Branch *, ter_inst) {
      int val_check = check_val(bb.get(), br->val);
      if (val_check != -1) {
        auto loadimm = new ir::insns::LoadImm(func->new_reg(Int), ConstValue(val_check));
        bb->insert_before_ter(loadimm);
        loadimm->add_use_def();
        br->change_use(br->val, loadimm->dst);
      } else {
        if (func->has_param(br->val)) continue;
        TypeCase(binary, ir::insns::Binary *, func->def_list.at(br->val)) {
          if (binary->op != BinaryOp::Lt && binary->op != BinaryOp::Gt && binary->op != BinaryOp::Leq && binary->op != BinaryOp::Geq) continue;
          int val_cond = check_cond(bb.get(), binary->src1, binary->op, binary->src2);
          if (val_cond != -1) {
            auto new_ins = new ir::insns::LoadImm(binary->dst, ConstValue(val_cond));
            new_ins->bb = binary->bb;
            binary->remove_use_def();
            new_ins->add_use_def();
            for (auto &insn : binary->bb->insns) {
              if (insn.get() == binary) {
                insn.reset(new_ins);
                break;
              }
            }
          }
        }
      }
    }
  }
}

void value_range_analysis(ir::Program *prog) {
  for(auto &func : prog->functions){
    value_range_analysis(&func.second);
  }
}

} // namespace mediumend
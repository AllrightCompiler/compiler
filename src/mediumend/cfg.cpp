#include "mediumend/cfg.hpp"
#include "common/ir.hpp"

#include <cassert>

namespace mediumend {

using std::vector;

void compute_use_def_list(ir::Function *func){
  auto &bbs = func->bbs;
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto &inst : insns) {
      inst.get()->add_use_def(func->use_list, func->def_list);
    }
  }
}

CFG::CFG(ir::Function *func) : func(func) {}

void CFG::build(){
  succ.clear();
  prev.clear();
  for (auto &bb : func->bbs) {
    this->succ[bb.get()] = {};
    this->prev[bb.get()] = {};
  }
  for (auto &bb : func->bbs) {
    if (bb->insns.empty())
      continue;
    auto &ins = bb->insns.back();
    ir::insns::Jump *jmp = dynamic_cast<ir::insns::Jump *>(ins.get());
    ir::insns::Branch *brh = dynamic_cast<ir::insns::Branch *>(ins.get());
    ir::insns::Return *ret = dynamic_cast<ir::insns::Return *>(ins.get());
    auto &next = this->succ[bb.get()];
    if (jmp) {
      next.insert(jmp->target);
      prev[jmp->target].insert(bb.get());
    } else if (brh) {
      next.insert(brh->true_target);
      next.insert(brh->false_target);
      prev[brh->true_target].insert(bb.get());
      prev[brh->false_target].insert(bb.get());
    } else if (ret) {
    } else {
      assert(false);
    }
  }
}

void CFG::remove_unreachable_bb() {
  auto entry = func->bbs.front().get();
  unordered_set<BasicBlock *> remove_set;
  vector<BasicBlock *> to_remove;

  for (auto prev : this->prev) {
    if (prev.second.size() == 0 && prev.first != entry) {
      to_remove.push_back(prev.first);
    }
  }
  while (to_remove.size()) {
    auto bb = to_remove.back();
    to_remove.pop_back();
    for (auto &succ : this->succ[bb]) {
      this->prev[succ].erase(bb);
      if (this->prev[succ].size() == 0) {
        to_remove.push_back(succ);
      }
    }
    remove_set.insert(bb);
  }
  for (auto iter = func->bbs.begin(); iter != func->bbs.end();) {
    if (remove_set.find(iter->get()) != remove_set.end()) {
      this->remove_bb_in_cfg(iter->get());
      iter = func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
}

void CFG::compute_dom_level(BasicBlock *bb, int dom_level) {
  auto &domlevel = this->domlevel;
  auto &dom = this->dom;
  domlevel[bb] = dom_level;
  for (BasicBlock *succ : dom[bb]) {
    compute_dom_level(succ, dom_level + 1);
  }
}

void CFG::compute_dom() {
  dom.clear();
  domby.clear();
  idom.clear();
  domlevel.clear();
  BasicBlock *entry = func->bbs.front().get();
  domby[entry] = {entry};
  unordered_set<BasicBlock *> all_bb;
  for (auto &bb : func->bbs) {
    all_bb.insert(bb.get());
  }
  for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
    domby[iter->get()] = all_bb;
  }
  bool modify = true;
  while (modify) {
    modify = false;
    for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
      auto bb = iter->get();
      auto &domby_bb = domby[bb];
      auto &dom_bb = dom[bb];
      for (auto iter = domby_bb.begin(); iter != domby_bb.end();) {
        BasicBlock *x = *iter;
        auto &prev_bb = prev[bb];
        auto &succ_bb = succ[bb];
        if (x != bb) {
          bool find = false;
          for (auto &pre : prev_bb) {
            if (domby[pre].find(x) == domby[pre].end()) {
              modify = true;
              find = true;
              iter = domby_bb.erase(iter);
              break;
            }
          }
          if (!find) {
            ++iter;
          }
        } else {
          ++iter;
        }
      }
    }
  }

  idom[entry] = nullptr;
  for (auto &bb : func->bbs) {
    auto &domby_bb = domby[bb.get()];
    for (BasicBlock *d : domby_bb) {
      if (d != bb.get()) {
        bool all_true = true;
        for (auto &pre : domby_bb) {
          if (pre == bb.get() || pre == d ||
              domby[pre].find(d) == domby[pre].end()) {
            continue;
          }
          all_true = false;
          break;
        }
        if (all_true) {
          idom[bb.get()] = d;
          dom[d].insert(bb.get());
          break;
        }
      }
    }
  }
  compute_dom_level(entry, 0);
}

unordered_map<BasicBlock *, unordered_set<BasicBlock *>> CFG::compute_df() {
  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> df;
  auto &idom = this->idom;
  for (auto &bb : func->bbs) {
    auto &succ = this->succ[bb.get()];
    for (BasicBlock *to : succ) {
      auto &domby = this->domby[to];
      BasicBlock *x = bb.get();
      while (x == to || domby.find(x) == domby.end()) {
        df[x].insert(to);
        x = idom[x];
      }
    }
  }
  return df;
}

} // namespace mediumend
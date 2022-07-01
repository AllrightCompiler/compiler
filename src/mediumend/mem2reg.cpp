#include "mediumend/mem2reg.hpp"
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void remove_unreachable_bb(Function *func, CFG *cfg) {
  auto entry = func->bbs.front().get();
  unordered_set<BasicBlock *> remove_set;
  vector<BasicBlock *> to_remove;

  for (auto prev : cfg->prev) {
    if (prev.second.size() == 0 && prev.first != entry) {
      to_remove.push_back(prev.first);
    }
  }
  while (to_remove.size()) {
    auto bb = to_remove.back();
    to_remove.pop_back();
    for (auto &succ : cfg->succ[bb]) {
      cfg->prev[succ].erase(bb);
      if (cfg->prev[succ].size() == 0) {
        to_remove.push_back(succ);
      }
    }
    remove_set.insert(bb);
  }
  for (auto iter = func->bbs.begin(); iter != func->bbs.end();) {
    if (remove_set.find(iter->get()) != remove_set.end()) {
      iter = func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
}

static void dfs(BasicBlock *bb, int dom_level, CFG *cfg) {
  auto &domlevel = cfg->domlevel;
  auto &dom = cfg->dom;
  domlevel[bb] = dom_level;
  for (BasicBlock *ch : dom[bb]) {
    dfs(ch, dom_level + 1, cfg);
  }
}

void compute_dom(Function *func, CFG *cfg) {
  BasicBlock *entry = func->bbs.front().get();
  auto &domby = cfg->domby;
  auto &dom = cfg->dom;
  auto &prev = cfg->prev;
  auto &succ = cfg->succ;
  auto &idom = cfg->idom;
  domby[entry] = {entry};
  unordered_set<BasicBlock *> all_bb;
  for (auto &bb : func->bbs) {
    all_bb.insert(bb.get());
  }
  for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
    cfg->domby[iter->get()] = all_bb;
  }

  bool modify = true;
  while (modify) {
    modify = false;
    for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
      auto bb = iter->get();
      auto &domby_bb = domby[bb];
      auto &dom_bb = dom[bb];
      for (auto it = domby_bb.begin(); it != domby_bb.end();) {
        BasicBlock *x = *it;
        auto &prev_bb = prev[bb];
        auto &succ_bb = succ[bb];
        if (x != bb) {
          bool find = false;
          for (auto &pre : prev_bb) {
            if (domby[pre].find(x) == domby[pre].end()) {
              modify = true;
              find = true;
              it = domby_bb.erase(it);
              break;
            }
          }
          if (!find) {
            ++it;
          }
        } else {
          ++it;
        }
      }
    }
  }
  // 计算直接支配节点用于计算迭代支配边界
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
  dfs(entry, 0, cfg);
}

auto compute_df(Function *func, CFG *cfg) {
  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> df;
  auto &idom = cfg->idom;
  for (auto &bb : func->bbs) {
    auto &succ = cfg->succ[bb.get()];
    for (BasicBlock *to : succ) {
      auto &domby = cfg->domby[to];
      BasicBlock *x = bb.get();
      while (x == to || domby.find(x) == domby.end()) {
        // while x不strictly dom to
        df[x].insert(to);
        x = idom[x];
      }
    }
  }
  return df;
}

void mem2reg(Function *func) {
  CFG cfg(func);
  remove_unreachable_bb(func, &cfg);
  compute_dom(func, &cfg);
  auto df = compute_df(func, &cfg);
  unordered_map<Reg, BasicBlock *> alloc_set;
  unordered_map<Reg, vector<BasicBlock *>> defs;
  unordered_map<Reg, vector<Reg>> defs_reg;

  for (auto &bb : func->bbs) {
    for (auto &i : bb->insns) {
      TypeCase(inst, ir::insns::Alloca *, i.get()) {
        alloc_set[inst->dst] = bb.get();
        defs[inst->dst] = {};
        defs_reg[inst->dst] = {};
      }
      // 先定义后使用，此处不会出现Store到没有alloca的地址
      TypeCase(inst, ir::insns::Store *, i.get()) {
        if (alloc_set.find(inst->addr) != alloc_set.end()) {
          defs[inst->addr].push_back(bb.get());
          defs_reg[inst->addr].push_back(inst->val);
        }
      }
    }
  }

  for (auto v : alloc_set) {
    unordered_set<BasicBlock *> F;
    unordered_set<BasicBlock *> W;
    for (auto d : defs[v.first]) {
      W.insert(d);
    }
    while (W.size()) {
      auto bb = *W.begin();
      W.erase(W.begin());
      for (auto &Y : df[bb]) {
        if (F.find(Y) == F.end()) {
          Y->push_front(new ir::insns::Phi(func->new_reg(v.first.type),
                                           defs[v.first],
                                           defs_reg[v.first])); // add phi
          F.insert(Y);
          bool find = false;
          for (auto &each : defs[v.first]) {
            if (each == Y) {
              find = true;
              break;
            }
          }
          if (!find) {
            W.insert(Y);
          }
        }
      }
      F.insert(bb);
    }
  }
}

} // namespace mediumend
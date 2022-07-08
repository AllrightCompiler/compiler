#include "mediumend/cfg.hpp"
#include "common/ir.hpp"

#include <cassert>
#include<iostream>

namespace mediumend {

using std::vector;

void compute_use_def_list(ir::Program *prog){
  for(auto &each : prog->functions){
    auto func = &each.second;
    func->def_list.clear();
    func->use_list.clear();
    auto &bbs = func->bbs;
    for (auto &bb : bbs) {
      auto &insns = bb->insns;
      for (auto &inst : insns) {
        inst.get()->add_use_def();
      }
    }
  }
}

void mark_global_addr_reg(ir::Program *prog){
  for(auto &each : prog->functions){
    auto func = &each.second;
    func->global_addr.clear();
    auto &bbs = func->bbs;
    for (auto &bb : bbs) {
      auto &insns = bb->insns;
      for (auto &inst : insns) {
        TypeCase(loadaddr, ir::insns::LoadAddr *, inst.get()) {
          func->global_addr.insert(loadaddr->dst);
        }
        TypeCase(get_ptr, ir::insns::GetElementPtr *, inst.get()){
          if(func->global_addr.count(get_ptr->base)){
            func->global_addr.insert(get_ptr->dst);
          }
        }
      }
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
      for(auto suc : succ[iter->get()]){
        for(auto &inst : suc->insns){
          TypeCase(phi, ir::insns::Phi *, inst.get()){
            phi->incoming.erase(iter->get());
          } else {
            break;
          }
        }
      }
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
  for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
    auto bb = iter->get();
    auto &domby_bb = domby[bb];
    for (BasicBlock *d : domby_bb) {
      if (d != bb) {
        bool all_true = true;
        for (auto &pre : domby_bb) {
          if (pre == bb || pre == d ||
              domby[pre].find(d) == domby[pre].end()) {
            continue;
          }
          all_true = false;
          break;
        }
        if (all_true) {
          idom[bb] = d;
          dom[d].insert(bb);
          break;
        }
      }
    }
  }
  compute_dom_level(entry, 0);
}


void CFG::compute_rdom() {
  rdom.clear();
  rdomby.clear();
  ridom.clear();
  BasicBlock *entry = func->bbs.front().get();
  rdomby[entry] = {entry};

  unordered_set<BasicBlock *> all_bb;
  unordered_set<BasicBlock *> outs;
  for (auto &bb : func->bbs) {
    all_bb.insert(bb.get());
    auto &inst = bb->insns.back();
    if(auto ret = dynamic_cast<ir::insns::Return *>(inst.get())){
      outs.insert(bb.get());
    }
  }
  for (auto iter = func->bbs.begin(); iter != func->bbs.end(); iter++) {
    auto bb = iter->get();
    if(outs.count(bb)){
      rdomby[bb] = {bb};
      ridom[bb] = nullptr;
    } else {
      rdomby[bb] = all_bb;
    }
  }
  bool modify = true;
  while (modify) {
    modify = false;
    for (auto iter = func->bbs.begin(); iter != func->bbs.end(); iter++) {
      auto bb = iter->get();
      if(outs.count(bb)){
        continue;
      }
      auto &rdomby_bb = rdomby[bb];
      auto &rdom_bb = rdom[bb];
      for (auto iter = rdomby_bb.begin(); iter != rdomby_bb.end();) {
        BasicBlock *x = *iter;
        auto &succ_bb = succ[bb];
        if (x != bb) {
          bool find = false;
          for (auto &suc : succ_bb) {
            if (rdomby[suc].find(x) == rdomby[suc].end()) {
              modify = true;
              find = true;
              iter = rdomby_bb.erase(iter);
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

  for (auto &bb : func->bbs) {
    auto &rdomby_bb = rdomby[bb.get()];
    for (BasicBlock *d : rdomby_bb) {
      if (d != bb.get()) {
        bool all_true = true;
        for (auto &pre : rdomby_bb) {
          if (pre == bb.get() || pre == d ||
              rdomby[pre].find(d) == rdomby[pre].end()) {
            continue;
          }
          all_true = false;
          break;
        }
        if (all_true) {
          ridom[bb.get()] = d;
          rdom[d].insert(bb.get());
          break;
        }
      }
    }
  }
}


unordered_map<BasicBlock *, unordered_set<BasicBlock *>> CFG::compute_df() {
  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> df;
  auto &idom = this->idom;
  for (auto &bb : func->bbs) {
    auto &succ_bb = this->succ[bb.get()];
    for (BasicBlock *to : succ_bb) {
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

unordered_map<BasicBlock *, unordered_set<BasicBlock *>> CFG::compute_rdf() {
  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> rdf;
  auto &ridom = this->ridom;
  for (auto &bb : func->bbs) {
    auto &prev_bb = prev[bb.get()];
    for (BasicBlock *to : prev_bb) {
      auto &rdomby = this->rdomby[to];
      BasicBlock *x = bb.get();
      while (x == to || rdomby.find(x) == rdomby.end()) {
        rdf[x].insert(to);
        x = ridom[x];
      }
    }
  }
  return rdf;
}

void CFG::rpo_dfs(BasicBlock *bb) {
  if (visit.count(bb)) return;
  visit.insert(bb);
  for (auto next : succ[bb]) {
    rpo_dfs(next);
  }
  rpo.emplace_back(bb);
}

void CFG::compute_rpo() {
  clear_visit();
  rpo.clear();
  rpo_dfs(func->bbs.front().get());
  std::reverse(rpo.begin(), rpo.end());
}

void CFG::loop_dfs(BasicBlock *header) {
  // dfs on dom tree
  for (auto next : dom[header]) {
    loop_dfs(next);
  }
  std::vector<BasicBlock *> bbs;
  for (auto p : prev[header]) {
    if (domby[p].count(header)) { // find back edge to header
      bbs.emplace_back(p);
    }
  }
  if (!bbs.empty()) { // form 1 loop ( TODO: nested loops with same header? )
    Loop *new_loop = new Loop(header);
    while (bbs.size() > 0) {
      auto bb = bbs.back();
      bbs.pop_back();
      if (!loop.count(bb)) {
        loop[bb] = new_loop;
        if (bb != header) {
          bbs.insert(bbs.end(), prev[bb].begin(), prev[bb].end());
        }
      } else {
        Loop *inner_loop = loop[bb];
        while (inner_loop->outer) inner_loop = inner_loop->outer;
        if (inner_loop == new_loop) continue;
        inner_loop->outer = new_loop;
        bbs.insert(bbs.end(), prev[inner_loop->header].begin(), prev[inner_loop->header].end());
      }
    }
  }
}

void calc_loop_level(Loop *loop) {
  if (loop->level != -1) return;
  if (loop->outer == nullptr) loop->level = 1;
  else {
    calc_loop_level(loop->outer);
    loop->level = loop->outer->level + 1;
  }
}

void CFG::loop_analysis() {
  loop.clear();
  loop_dfs(func->bbs.front().get());
  for (auto pair : loop) {
    calc_loop_level(pair.second);
  }
}

} // namespace mediumend
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
  for (auto &bb : func->bbs) {
    bb->succ = {};
    bb->prev = {};
  }
  for (auto &bb : func->bbs) {
    if (bb->insns.empty())
      continue;
    auto &ins = bb->insns.back();
    ir::insns::Jump *jmp = dynamic_cast<ir::insns::Jump *>(ins.get());
    ir::insns::Branch *brh = dynamic_cast<ir::insns::Branch *>(ins.get());
    ir::insns::Return *ret = dynamic_cast<ir::insns::Return *>(ins.get());
    auto &next = bb->succ;
    if (jmp) {
      next.insert(jmp->target);
      jmp->target->prev.insert(bb.get());
    } else if (brh) {
      next.insert(brh->true_target);
      next.insert(brh->false_target);
      brh->true_target->prev.insert(bb.get());
      brh->false_target->prev.insert(bb.get());
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

  for (auto &bb : func->bbs) {
    if ((bb->prev.size() == 0 && bb.get() != entry) || (bb->prev.size() == 1 && *(bb->prev.begin()) == bb.get())) {
      to_remove.push_back(bb.get());
    }
  }
  while (to_remove.size()) {
    auto bb = to_remove.back();
    to_remove.pop_back();
    for (auto &succ : bb->succ) {
      succ->prev.erase(bb);
      if (succ->prev.size() == 0) {
        to_remove.push_back(succ);
      }
    }
    remove_set.insert(bb);
  }
  for (auto iter = func->bbs.begin(); iter != func->bbs.end();) {
    if (remove_set.find(iter->get()) != remove_set.end()) {
      for(auto suc : iter->get()->succ){
        for(auto &inst : suc->insns){
          TypeCase(phi, ir::insns::Phi *, inst.get()){
            phi->remove_prev(iter->get());
          } else {
            break;
          }
        }
      }
      for(auto pre: iter->get()->prev){
        pre->succ.erase(iter->get());
      }
      for(auto &inst : iter->get()->insns){
        inst->remove_use_def();
      }
      iter = func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
}

void CFG::compute_dom_level(BasicBlock *bb, int dom_level) {
  bb->domlevel = dom_level;
  for (BasicBlock *succ : bb->dom) {
    compute_dom_level(succ, dom_level + 1);
  }
}

void CFG::compute_dom() {
  func->clear_dom();
  BasicBlock *entry = func->bbs.front().get();
  entry->domby = {entry};
  unordered_set<BasicBlock *> all_bb;
  for (auto &bb : func->bbs) {
    all_bb.insert(bb.get());
  }
  for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
    iter->get()->domby = all_bb;
  }
  bool modify = true;
  while (modify) {
    modify = false;
    for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
      auto bb = iter->get();
      auto &domby_bb = bb->domby;
      auto &dom_bb = bb->dom;
      for (auto iter = domby_bb.begin(); iter != domby_bb.end();) {
        BasicBlock *x = *iter;
        auto &prev_bb = bb->prev;
        if (x != bb) {
          bool find = false;
          for (auto &pre : prev_bb) {
            if (pre->domby.find(x) == pre->domby.end()) {
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

  entry->idom = nullptr;
  for (auto iter = ++func->bbs.begin(); iter != func->bbs.end(); iter++) {
    auto bb = iter->get();
    auto &domby_bb = bb->domby;
    for (BasicBlock *d : domby_bb) {
      if (d != bb) {
        bool all_true = true;
        for (auto &pre : domby_bb) {
          if (pre == bb || pre == d ||
              pre->domby.find(d) == pre->domby.end()) {
            continue;
          }
          all_true = false;
          break;
        }
        if (all_true) {
          bb->idom = d;
          d->dom.insert(bb);
          break;
        }
      }
    }
  }
  compute_dom_level(entry, 0);
}


unordered_map<BasicBlock *, unordered_set<BasicBlock *>> CFG::compute_df() {
  unordered_map<BasicBlock *, unordered_set<BasicBlock *>> df;
  for (auto &bb : func->bbs) {
    auto &succ_bb = bb->succ;
    for (BasicBlock *to : succ_bb) {
      auto &domby = to->domby;
      BasicBlock *x = bb.get();
      while (x == to || domby.find(x) == domby.end()) {
        df[x].insert(to);
        x = x->idom;
      }
    }
  }
  return df;
}
void CFG::compute_rpo() {
  func->clear_visit();
  rpo.clear();
  func->bbs.front().get()->rpo_dfs(rpo);
  std::reverse(rpo.begin(), rpo.end());
}

void CFG::loop_analysis() {
  func->clear_visit();
  func->bbs.front().get()->loop_dfs();
  for (auto &bb : func->bbs) {
    calc_loop_level(bb->loop);
  }
}

} // namespace mediumend
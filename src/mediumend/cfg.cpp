#include "mediumend/cfg.hpp"
#include "common/ir.hpp"

#include <cassert>

namespace mediumend {

using std::vector;

vector<ir::Reg> get_inst_use_reg(ir::Instruction *inst){
  vector<ir::Reg> regs;
  TypeCase(unary, ir::insns::Unary *, inst){
    regs.push_back(unary->src);
  }
  TypeCase(binary, ir::insns::Binary *, inst){
    regs.push_back(binary->src1);
    regs.push_back(binary->src2);
  }
  TypeCase(load, ir::insns::Load *, inst){
    regs.push_back(load->addr);
  }
  TypeCase(store, ir::insns::Store *, inst){
    regs.push_back(store->addr);
    regs.push_back(store->val);
  }
  TypeCase(call, ir::insns::Call *, inst){
    for(auto &r : call->args){
      regs.push_back(r);
    }
  }
  TypeCase(phi, ir::insns::Phi *, inst){
    for(auto &r : phi->incoming){
      regs.push_back(r.second);
    }
  }
  TypeCase(convey, ir::insns::Convert *, inst){
    regs.push_back(convey->src);
  }
  TypeCase(ret, ir::insns::Return *, inst){
    if(ret->val.has_value()){
      regs.push_back(ret->val.value());
    }
  }
  TypeCase(branch, ir::insns::Branch *, inst){
    regs.push_back(branch->val);
  }
  TypeCase(ptr, ir::insns::GetElementPtr *, inst){
    regs.push_back(ptr->base);
    for(auto &r : ptr->indices){
      regs.push_back(r);
    }
  }
  return regs;
}

CFG::CFG(ir::Function *func) : func(func) {
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
    auto &succ = this->succ[bb.get()];
    if (jmp) {
      succ.insert(jmp->target);
      prev[jmp->target].insert(bb.get());
    } else if (brh) {
      succ.insert(brh->true_target);
      succ.insert(brh->false_target);
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

void CFG::compute_use_def_list(){
  auto &bbs = func->bbs;
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto &inst : insns) {
      inst.get()->addUseDef(this->use_list, this->def_list);
    }
  }
}

void CFG::remove_unused_reg(){
  auto &bbs = func->bbs;
  vector<ir::Instruction *> stack;
  unordered_set<ir::Instruction *> remove_inst_set; 
  for (auto &bb : bbs) {
    auto &insns = bb->insns;
    for (auto &inst : insns) {
      TypeCase(output, ir::insns::Output *, inst.get()){
        if(auto call = dynamic_cast<ir::insns::Call *>(output)){
          continue;
        }
        auto &reg = output->dst;
        if (reg.id && this->use_list[reg].size() == 0) {
          stack.push_back(output);
        }
      }
    }
  }
  while(!stack.empty()){
    auto inst = stack.back();
    stack.pop_back();
    auto regs = get_inst_use_reg(inst);
    inst->removeUseDef(this->use_list, this->def_list);
    for(auto reg : regs){
      if(this->use_list[reg].size() == 0){
        auto inst = def_list[reg];
        if(auto call = dynamic_cast<ir::insns::Call *>(inst)){
          continue;
        }
        stack.push_back(inst);
      }
    }
    remove_inst_set.insert(inst);
  }
  for(auto &bb : bbs){
    auto &insns = bb->insns;
    for(auto iter = insns.begin(); iter != insns.end();){
      if(remove_inst_set.find(iter->get()) != remove_inst_set.end()){
        iter = insns.erase(iter);
      }else{
        iter++;
      }
    }
  }
}

} // namespace mediumend
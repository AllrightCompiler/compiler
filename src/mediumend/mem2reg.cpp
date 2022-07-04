#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

#include <cassert>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void mem2reg(ir::Program *prog) {
  for(auto &each : prog->functions){
    Function* func = &each.second;
    CFG *cfg = func->cfg;
    auto df = cfg->compute_df();
    unordered_map<Reg, BasicBlock *> alloc_set;
    unordered_map<Reg, ScalarType> alloc2type;
    unordered_map<Reg, vector<BasicBlock *>> defs;
    unordered_map<Reg, vector<Reg>> defs_reg;
    unordered_map<Reg, Reg> alloc_map;
    unordered_map<ir::insns::Phi *, Reg> phi2mem;

    for (auto &bb : func->bbs) {
      for (auto &i : bb->insns) {
        TypeCase(inst, ir::insns::Alloca *, i.get()) {
          alloc_set[inst->dst] = bb.get();
          alloc2type[inst->dst] = inst->type.base_type;
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
    // mem2reg第一阶段，添加Phi函数
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
            Reg r = func->new_reg(alloc2type[v.first]);
            auto phi = new ir::insns::Phi(r,
                                            defs[v.first],
                                            defs_reg[v.first]);
            Y->push_front(phi); // add phi
            phi->add_use_def(func->use_list, func->def_list);
            phi2mem[phi] = v.first;
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

    // mem2reg第二阶段，寄存器重命名
    vector<BasicBlock *> stack;
    stack.push_back(func->bbs.front().get());
    while(stack.size()){
      auto bb = stack.back();
      stack.pop_back();
      for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
        TypeCase(inst, ir::insns::Alloca *, iter->get()) {
          if(!inst->type.is_array()){
            inst->remove_use_def(func->use_list, func->def_list);
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        TypeCase(inst, ir::insns::Load *, iter->get()) {
          if(alloc_set.find(inst->addr) != alloc_set.end()) {
            assert(alloc_map.find(inst->addr) != alloc_map.end());
            auto &all_use = func->use_list[inst->dst];
            while(!all_use.empty()){
              auto &uses= all_use.front();
              uses->change_use(func->use_list, inst->dst, alloc_map[inst->addr]);
            }
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        TypeCase(inst, ir::insns::Store *, iter->get()) {
          if(alloc_set.find(inst->addr) != alloc_set.end()) {
            alloc_map[inst->addr] = inst->val;
            inst->remove_use_def(func->use_list, func->def_list);
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        TypeCase(inst, ir::insns::Phi *, iter->get()) {
          alloc_map[phi2mem[inst]] = inst->dst;
        }
        iter++;
        continue;
      }
      for(auto &next : cfg->dom[bb]){
        stack.push_back(next);
      }
    }
  }
}

void simplification_phi(ir::Program *prog){
  for(auto &f : prog->functions){
    Function* func = &f.second;
    auto &prev = func->cfg->prev;
    for(auto &bb : func->bbs){
      for(auto iter = bb->insns.begin(); iter != bb->insns.end();){
        TypeCase(inst, ir::insns::Phi *, iter->get()) {
          for(auto in_iter = inst->incoming.begin(); in_iter != inst->incoming.end();){
            if(!prev[bb.get()].count(in_iter->first)){
              func->use_list[in_iter->second].remove(inst);
              in_iter = inst->incoming.erase(in_iter);
            } else {
              in_iter++;
            }
          }
          if(inst->incoming.size() == 1){
            while(func->use_list[inst->dst].size()){
              auto &uses = func->use_list[inst->dst].front();
              uses->change_use(func->use_list, inst->dst, inst->incoming.begin()->second);
            }
            iter = bb->insns.erase(iter);
          }
        } else {
          break;
        }
      }
    }
  }
}

} // namespace mediumend
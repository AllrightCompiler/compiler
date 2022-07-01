#include "mediumend/mem2reg.hpp"
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

void mem2reg(Function *func) {
  CFG cfg(func);
  cfg.remove_unreachable_bb();
  cfg.compute_use_list();
  cfg.compute_dom();
  auto df = cfg.compute_df();
  unordered_map<Reg, BasicBlock *> alloc_set;
  unordered_map<Reg, ScalarType> alloc2type;
  unordered_map<Reg, vector<BasicBlock *>> defs;
  unordered_map<Reg, vector<Reg>> defs_reg;
  unordered_map<BasicBlock *, unordered_map<Reg, Reg>> alloc_map;

  for (auto &bb : func->bbs) {
    alloc_map[bb.get()] = {};
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
          Y->push_front(new ir::insns::Phi(r,
                                           defs[v.first],
                                           defs_reg[v.first])); // add phi
          Y->insns.front()->addUse(cfg.use_list);
          alloc_map[Y][v.first] = r;
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
        inst->removeUse(cfg.use_list);
        iter = bb->insns.erase(iter);
        continue;
      }
      TypeCase(inst, ir::insns::Load *, iter->get()) {
        if(alloc_set.find(inst->addr) != alloc_set.end()) {
          assert(alloc_map[bb].find(inst->addr) != alloc_map[bb].end());
          auto &all_use = cfg.use_list[inst->dst];
          while(!all_use.empty()){
            auto &uses= all_use.front();
            uses->changeUse(cfg.use_list, inst->dst, alloc_map[bb][inst->addr]);
          }
          iter = bb->insns.erase(iter);
          continue;
        }
      }
      TypeCase(inst, ir::insns::Store *, iter->get()) {
        if(alloc_set.find(inst->addr) != alloc_set.end()) {
          alloc_map[bb][inst->addr] = inst->val;
          inst->removeUse(cfg.use_list);
          iter = bb->insns.erase(iter);
          continue;
        }
      }
      iter++;
      continue;
    }
    for(auto &next : cfg.dom[bb]){
      stack.push_back(next);
    }
  }
}

} // namespace mediumend
#include "common/ir.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;
using std::unordered_map;

unordered_map<Reg, Reg> find_base(Function *func) {
  unordered_map<Reg, Reg> reg2base;
  unordered_map<std::string, Reg> name2base;
  for (auto &bb : func->bbs) {
    for (auto &ins : bb->insns) {
      auto inst = ins.get();
      TypeCase(loadaddr, ir::insns::LoadAddr *, inst){
        if(!name2base.count(loadaddr->var_name)){
          name2base[loadaddr->var_name] = loadaddr->dst;
        }
        reg2base[loadaddr->dst] = name2base[loadaddr->var_name];
      } else TypeCase(alloca, ir::insns::Alloca *, inst){
        reg2base[alloca->dst] = alloca->dst;
      } else TypeCase(gep, ir::insns::GetElementPtr *, inst){
        reg2base[gep->base] = reg2base.at(gep->base);
      }
    }
  }
  return reg2base;
}

void array_mem2reg(ir::Program *prog) {
  for (auto &each : prog->functions) {
    Function *func = &each.second;
    CFG *cfg = func->cfg;
    cfg->remove_unreachable_bb();
    cfg->compute_dom();
    auto df = cfg->compute_df();

    unordered_map<Reg, BasicBlock *> alloc_set;
    unordered_map<Reg, ScalarType> alloc2type;
    unordered_map<Reg, unordered_set<BasicBlock *>> defs;
    unordered_map<BasicBlock *, unordered_map<Reg, Reg>> alloc_map;
    unordered_map<ir::insns::Phi *, Reg> phi2mem;

    unordered_map<Reg, Reg> reg2base;
    unordered_map<std::string, Reg> name2base;

    for (auto &bb : func->bbs) {
      for (auto &ins : bb->insns) {
        auto inst = ins.get();
        TypeCase(loadaddr, ir::insns::LoadAddr *, inst){
          if(!name2base.count(loadaddr->var_name)){
            name2base[loadaddr->var_name] = loadaddr->dst;
            alloc_set[loadaddr->dst] = bb.get();
          }
          reg2base[loadaddr->dst] = name2base[loadaddr->var_name];
        } else TypeCase(alloca, ir::insns::Alloca *, inst){
          reg2base[alloca->dst] = alloca->dst;
          alloc_set[alloca->dst] = bb.get();
        } else TypeCase(gep, ir::insns::GetElementPtr *, inst){
          reg2base[gep->dst] = reg2base.at(gep->base);
        } else TypeCase(store, ir::insns::Store *, inst) {
          if (reg2base.count(store->addr)) {
            defs[reg2base.at(store->addr)].insert(bb.get());
          }
        } else TypeCase(call, ir::insns::Call *, inst) {
          auto use = call->use();
          for(auto &use_reg : use){
            if(reg2base.count(use_reg)){
              defs[reg2base.at(use_reg)].insert(bb.get());
            }
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
          if (F.find(Y) == F.end() && Y->live_in.count(v.first)) {
            Reg r = func->new_reg(alloc2type[v.first]);
            auto phi = new ir::insns::Phi(r);
            Y->insns.emplace_front(phi); // add phi
            phi->bb = Y;
            phi2mem[phi] = v.first;
            F.insert(Y);
            if (!defs[v.first].count(Y)) {
              W.insert(Y);
            }
          }
        }
      }
    }

    // mem2reg第二阶段，寄存器重命名
    vector<BasicBlock *> stack;
    stack.push_back(func->bbs.front().get());
    func->clear_visit();
    func->bbs.front().get()->visit = true;
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
        TypeCase(inst, ir::insns::Load *, iter->get()) {
          if (reg2base.find(inst->addr) != reg2base.end()) {
            auto base = reg2base[inst->addr];
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(base)) {
              pos = pos->idom;
            }
            Reg reg;
            if (!pos) {
              assert(false);
            } else {
              reg = alloc_map[pos][base];
            }
            inst->remove_use_def();
            auto new_inst = new ir::insns::MemUse(inst->dst, reg, inst->addr);
            iter->reset(new_inst);
            new_inst->bb = bb;
            new_inst->add_use_def();
          }
        }
        TypeCase(inst, ir::insns::Store *, iter->get()) {
          if (reg2base.find(inst->addr) != reg2base.end()) {
            auto base = reg2base[inst->addr];
            inst->remove_use_def();
            Reg dst = func->new_reg(ScalarType::String);
            auto new_inst = new ir::insns::MemDef(dst, inst->addr, inst->val);
            iter->reset(new_inst);
            new_inst->bb = bb;
            new_inst->add_use_def();
            alloc_map[bb][base] = dst;
          }
        }
        TypeCase(inst, ir::insns::Call *, iter->get()) {
          auto use = inst->use();
          for(auto reg : use){
            if (reg2base.find(reg) != reg2base.end()) {
              auto base = reg2base[reg];
              Reg dst = func->new_reg(ScalarType::String);
              auto new_inst = new ir::insns::MemDef(dst, reg, inst->dst);
              iter = bb->insns.insert(iter, std::unique_ptr<ir::Instruction>(new_inst));
              new_inst->bb = bb;
              new_inst->add_use_def();
              alloc_map[bb][base] = dst;
            }
          }
        }
        TypeCase(inst, ir::insns::Phi *, iter->get()) {
          if (phi2mem.count(inst)) {
            alloc_map[bb][phi2mem.at(inst)] = inst->dst;
          }
        }
        iter++;
        continue;
      }
      for(auto &succ : bb->succ){
        for(auto &inst : succ->insns){
          TypeCase(phi, ir::insns::Phi *, inst.get()) {
            if (!phi2mem.count(phi)) {
              continue;
            }
            BasicBlock *pos = bb;
            Reg reg = phi2mem.at(phi);
            while (pos && !alloc_map[pos].count(reg)) {
              pos = pos->idom;
            }
            if (pos) {
              phi->incoming[bb] = alloc_map[pos][phi2mem.at(phi)];
            }
          }
          else {
            break;
          }
        }
      }
      for(auto dom : bb->dom){
        stack.push_back(dom);
      }
    }
    for (auto phi : phi2mem) {
      phi.first->add_use_def();
    }
    // remove_unused_phi(func);
  }
}

} // namespace mediumend
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void add_phi_reg(ir::insns::Phi *phi, unordered_set<ir::insns::Phi *> &used_phi, Function *func){
  auto used_reg = phi->use();
  used_phi.insert(phi);
  for(auto &reg : used_reg){
    if(!func->has_param(reg)){
      auto def = func->def_list.at(reg);
      TypeCase(new_phi, ir::insns::Phi *, def){
        if(!used_phi.count(new_phi)){
          add_phi_reg(new_phi, used_phi, func);
        }
      }
    }
  }
}

void remove_unused_phi(ir::Function *func){
  unordered_set<ir::insns::Phi *> used_phi;
  for(auto &bb : func->bbs){
    for(auto &inst : bb->insns){
      TypeCase(phi, ir::insns::Phi *, inst.get()){
        continue;
      }
      auto used_reg = inst->use();
      for(auto &reg : used_reg){
        if(!func->has_param(reg)){
          auto def = func->def_list.at(reg);
          TypeCase(phi, ir::insns::Phi *, def){
            add_phi_reg(phi, used_phi, func);
          }
        }
      }
    }
  }
  for(auto &bb : func->bbs){
    for(auto iter = bb->insns.begin(); iter != bb->insns.end();){
      TypeCase(phi, ir::insns::Phi *, iter->get()){
        if(!used_phi.count(phi)){
          iter->get()->remove_use_def();
          iter = bb->insns.erase(iter);
          continue;
        }
      }
      iter++;
    }
  }
}

void main_global_var_to_local(ir::Program *prog){
  mark_global_addr_reg(prog);
  unordered_set<string> used_vars; // 变量被除了main函数以外的函数使用过
  unordered_set<string> main_used_vars; // 变量被除了main函数以外的函数使用过
  unordered_map<string, ConstValue> global_const; // 全局常量
  for(auto &[name, val] : prog->global_vars){
    if(val->val.has_value()){
      global_const[name] = val->val.value();
    } else {
      if(val->type.is_array()){
        continue;
      }
      if(val->type.base_type == ScalarType::Int){
        global_const[name] = ConstValue(0);
      }
      if(val->type.base_type == ScalarType::Float){
        global_const[name] = ConstValue(0.0f);
      }
    }
  }
  for (auto &[name, func] : prog->functions) {
    unordered_map<Reg, string> reg2name;
    if (name == "main") {
      for (auto &bb : func.bbs) {
        for (auto &insn : bb->insns) {
          TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
            auto &var = prog->global_vars[loadaddr->var_name];
            if (var->type.is_array()) // 只改写非数组的全局变量
              continue;
            main_used_vars.insert(loadaddr->var_name);
            reg2name[loadaddr->dst] = loadaddr->var_name;
          }
          TypeCase(store, ir::insns::Store *, insn.get()){
            if(reg2name.count(store->addr)){
              global_const.erase(reg2name[store->addr]);
            }
          }
        }
      }
    } else {
      for (auto &bb : func.bbs) {
        for (auto &insn : bb->insns) {
          TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
            used_vars.insert(loadaddr->var_name);
            reg2name[loadaddr->dst] = loadaddr->var_name;
          }
          TypeCase(store, ir::insns::Store *, insn.get()){
            if(reg2name.count(store->addr)){
              global_const.erase(reg2name[store->addr]);
            }
          }
        }
      }
    }
  }
  auto &func = prog->functions.at("main");
  unordered_map<string, Reg>
      global_name_to_local_reg; // 全局变量到局部变量的映射
  auto &entry = func.bbs.front();
  for (auto &var_name : main_used_vars) {
    if (used_vars.count(var_name))
      continue;

    auto &var = prog->global_vars[var_name];

    Reg reg = func.new_reg(String);
    Reg val_reg = func.new_reg(var->type.base_type);

    global_name_to_local_reg[var_name] = reg;
    entry->push_front(new ir::insns::Store(reg, val_reg));
    if (var->val.has_value()) {
      entry->push_front(new ir::insns::LoadImm(val_reg, var->val.value()));
    } else {
      if (var->type.base_type == ScalarType::Float) {
        entry->push_front(new ir::insns::LoadImm(val_reg, ConstValue(0.0f)));
      } else {
        entry->push_front(new ir::insns::LoadImm(val_reg, ConstValue(0)));
      }
    }
    entry->push_front(new ir::insns::Alloca(reg, var->type));
    prog->global_vars.erase(var_name);
  }
  for (auto &bb : func.bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      TypeCase(loadaddr, ir::insns::LoadAddr *, iter->get()) {
        if (!prog->global_vars.count(loadaddr->var_name)) {
          copy_propagation(func.use_list, loadaddr->dst,
                           global_name_to_local_reg[loadaddr->var_name]);
          iter = bb->insns.erase(iter);
          continue;
        }
      }
      ++iter;
    }
  }
  for (auto &[name, func] : prog->functions) {
    unordered_map<string, Reg> name2reg;
    unordered_map<Reg, Reg> reg2base;
    for (auto &bb : func.bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
          if (global_const.count(loadaddr->var_name)) {
            if(!name2reg.count(loadaddr->var_name)){
              Reg reg = func.new_reg(global_const.at(loadaddr->var_name).type);
              name2reg[loadaddr->var_name] = reg;
              func.bbs.front()->push_front(
                  new ir::insns::LoadImm(reg, global_const.at(loadaddr->var_name)));
            }
            reg2base[loadaddr->dst] = name2reg.at(loadaddr->var_name);
          }
        }
        TypeCase(load, ir::insns::Load *, insn.get()){
          if(reg2base.count(load->addr)){
            copy_propagation(func.use_list, load->dst,
                             reg2base.at(load->addr));
          }
        }
      }
    }
  }
}

void mem2reg(ir::Program *prog) {
  for (auto &each : prog->functions) {
    Function *func = &each.second;
    CFG *cfg = func->cfg;
    cfg->build();
    cfg->remove_unreachable_bb();
    cfg->compute_dom();
    auto df = cfg->compute_df();
    func->do_liveness_analysis();
    unordered_map<Reg, BasicBlock *> alloc_set;
    unordered_map<Reg, int> alloc2type;
    unordered_map<Reg, unordered_set<BasicBlock *>> defs;
    unordered_map<BasicBlock *, unordered_map<Reg, Reg>> alloc_map;
    unordered_map<ir::insns::Phi *, Reg> phi2mem;

    for (auto &bb : func->bbs) {
      for (auto &i : bb->insns) {
        TypeCase(inst, ir::insns::Alloca *, i.get()) {
          if(inst->type.is_array()){
            continue;
          }
          alloc_set[inst->dst] = bb.get();
          alloc2type[inst->dst] = inst->type.base_type;
          defs[inst->dst] = {};
        }
        // 先定义后使用，此处不会出现Store到没有alloca的地址
        TypeCase(inst, ir::insns::Store *, i.get()) {
          if (alloc_set.find(inst->addr) != alloc_set.end()) {
            defs[inst->addr].insert(bb.get());
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
        TypeCase(inst, ir::insns::Alloca *, iter->get()) {
          if (!inst->type.is_array()) {
            inst->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        TypeCase(inst, ir::insns::Load *, iter->get()) {
          if (alloc_set.find(inst->addr) != alloc_set.end()) {
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(inst->addr)) {
              pos = pos->idom;
            }
            Reg reg;
            if (!pos) {
              reg = Reg(ScalarType::Int, 0);
            } else {
              reg = alloc_map[pos][inst->addr];
            }
            copy_propagation(func->use_list, inst->dst, reg);
            inst->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        TypeCase(inst, ir::insns::Store *, iter->get()) {
          if (alloc_set.find(inst->addr) != alloc_set.end()) {
            alloc_map[bb][inst->addr] = inst->val;
            inst->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
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
    remove_unused_phi(func);
  }
}

} // namespace mediumend
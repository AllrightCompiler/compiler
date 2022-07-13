#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void main_global_var_to_local(ir::Program *prog) {
  mark_global_addr_reg(prog);
  unordered_set<string> used_vars; // 变量被除了main函数以外的函数使用过
  unordered_set<string> main_used_vars; // 变量被除了main函数以外的函数使用过
  for (auto &[name, func] : prog->functions) {
    if (name == "main") {
      for (auto &bb : func.bbs) {
        for (auto &insn : bb->insns) {
          TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
            main_used_vars.insert(loadaddr->var_name);
          }
        }
      }
    } else {
      for (auto &bb : func.bbs) {
        for (auto &insn : bb->insns) {
          TypeCase(loadaddr, ir::insns::LoadAddr *, insn.get()) {
            used_vars.insert(loadaddr->var_name);
          }
        }
      }
    }
  }
  auto &func = prog->functions["main"];
  unordered_map<string, Reg>
      global_name_to_local_reg; // 全局变量到局部变量的映射
  auto &entry = func.bbs.front();
  for (auto &var_name : main_used_vars) {
    if (used_vars.count(var_name))
      continue;

    auto &var = prog->global_vars[var_name];
    if (var->type.is_array()) // 只改写非数组的全局变量
      continue;

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
  }
  for (auto &bb : func.bbs) {
    for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      TypeCase(loadaddr, ir::insns::LoadAddr *, iter->get()) {
        if (!used_vars.count(loadaddr->var_name)) {
          copy_propagation(func.use_list, loadaddr->dst,
                           global_name_to_local_reg[loadaddr->var_name]);
        }
      }
      ++iter;
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
    unordered_map<Reg, BasicBlock *> alloc_set;
    unordered_map<Reg, ScalarType> alloc2type;
    unordered_map<Reg, vector<BasicBlock *>> defs;
    unordered_map<Reg, vector<Reg>> defs_reg;
    unordered_map<BasicBlock *, unordered_map<Reg, Reg>> alloc_map;
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
            auto phi = new ir::insns::Phi(r);
            Y->insns.emplace_front(phi); // add phi
            phi->bb = Y;
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
      for (auto &succ : bb->succ) {
        if (!succ->visit) {
          succ->visit = true;
          stack.push_back(succ);
        }
        for (auto &inst : succ->insns) {
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
    }
    for (auto phi : phi2mem) {
      phi.first->add_use_def();
    }
  }
}

void simplification_phi(ir::Program *prog) {
  for (auto &f : prog->functions) {
    Function *func = &f.second;
    for (auto &bb : func->bbs) {
      for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
        TypeCase(inst, ir::insns::Phi *, iter->get()) {
          for (auto in_iter = inst->incoming.begin();
               in_iter != inst->incoming.end();) {
            if (!bb.get()->prev.count(in_iter->first)) {
              func->use_list[in_iter->second].erase(inst);
              in_iter = inst->incoming.erase(in_iter);
            } else {
              in_iter++;
            }
          }
          if (inst->incoming.size() == 1) {
            auto &dst_use_list = func->use_list[inst->dst];
            while (dst_use_list.size()) {
              auto uses = *dst_use_list.begin();
              auto dmy = dynamic_cast<ir::Instruction *>(uses);
              if (!dmy) {
                assert(false);
              }
              auto reg = inst->incoming.begin()->second;
              uses->change_use(inst->dst, reg);
            }
            iter->get()->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          }
          iter++;
        }
        else {
          break;
        }
      }
    }
  }
}

} // namespace mediumend
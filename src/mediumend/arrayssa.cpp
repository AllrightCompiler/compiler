#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

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
      TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
        if (!name2base.count(loadaddr->var_name)) {
          name2base[loadaddr->var_name] = loadaddr->dst;
        }
        reg2base[loadaddr->dst] = name2base[loadaddr->var_name];
      }
      else TypeCase(alloca, ir::insns::Alloca *, inst) {
        reg2base[alloca->dst] = alloca->dst;
      }
      else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
        reg2base[gep->base] = reg2base.at(gep->base);
      }
    }
  }
  return reg2base;
}

void array_mem2reg(ir::Program *prog) {
  unordered_map<std::string, unordered_set<std::string>> used_gvar;
  for (auto &each : prog->functions) {
    Function *func = &each.second;
    for (auto &bb : func->bbs) {
      for (auto &inst : bb->insns) {
        TypeCase(loadaddr, ir::insns::LoadAddr *, inst.get()) {
          used_gvar[each.first].insert(loadaddr->var_name);
        }
      }
    }
  }
  for (auto &each : prog->functions) {
    Function *func = &each.second;
    CFG *cfg = func->cfg;
    cfg->remove_unreachable_bb();
    cfg->compute_dom();
    auto df = cfg->compute_df();
    auto entry = func->bbs.front().get();

    for (auto each : prog->global_vars) {
      entry->push_front(new ir::insns::LoadAddr(
          func->new_reg(ScalarType::String), each.first));
    }
    unordered_map<Reg, BasicBlock *> alloc_set;
    unordered_map<Reg, int> alloc2type;
    unordered_map<Reg, unordered_set<BasicBlock *>> defs;
    unordered_map<BasicBlock *, unordered_map<Reg, Reg>> alloc_map;
    unordered_map<ir::insns::Phi *, Reg> phi2mem;

    unordered_map<Reg, Reg> reg2base;
    unordered_map<std::string, Reg> name2base;
    vector<BasicBlock *> stack;
    stack.push_back(entry);
    for (int i = 0; i < func->sig.param_types.size(); i++) {
      auto &param = func->sig.param_types[i];
      if (param.is_array()) {
        auto reg = Reg(param.base_type, i + 1);
        reg2base[reg] = reg;
        alloc_set[reg] = entry;
        alloc_map[entry][reg] = reg;
      }
    }
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      for (auto dom : bb->dom) {
        stack.push_back(dom);
      }
      for (auto &ins : bb->insns) {
        auto inst = ins.get();
        TypeCase(loadaddr, ir::insns::LoadAddr *, inst) {
          if (!prog->global_vars.at(loadaddr->var_name)->type.is_array()) {
            continue;
          }
          if (!name2base.count(loadaddr->var_name)) {
            name2base[loadaddr->var_name] = loadaddr->dst;
            alloc_set[loadaddr->dst] = bb;
            defs[loadaddr->dst].insert(bb);
          }
          reg2base[loadaddr->dst] = name2base[loadaddr->var_name];
        }
        else TypeCase(alloca, ir::insns::Alloca *, inst) {
          reg2base[alloca->dst] = alloca->dst;
          alloc_set[alloca->dst] = bb;
          defs[alloca->dst].insert(bb);
        }
        else TypeCase(gep, ir::insns::GetElementPtr *, inst) {
          reg2base[gep->dst] = reg2base.at(gep->base);
        }
        else TypeCase(store, ir::insns::Store *, inst) {
          if (reg2base.count(store->addr)) {
            defs[reg2base.at(store->addr)].insert(bb);
          }
        }
        else TypeCase(call, ir::insns::Call *, inst) {
          auto use = call->use();
          for (auto &use_reg : use) {
            if (reg2base.count(use_reg)) {
              defs[reg2base.at(use_reg)].insert(bb);
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
          if (F.find(Y) == F.end()) {
            Reg r = func->new_reg(alloc2type[v.first]);
            auto phi = new ir::insns::Phi(r, true);
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
    stack.push_back(func->bbs.front().get());
    func->clear_visit();
    func->bbs.front().get()->visit = true;
    std::unordered_map<BasicBlock *, unordered_map<Reg, unordered_set<Reg>>>
        use_before_def;
    use_before_def[nullptr] = {};
    while (stack.size()) {
      auto bb = stack.back();
      stack.pop_back();
      use_before_def[bb] = use_before_def.at(bb->idom);
      for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
        TypeCase(inst, ir::insns::Load *, iter->get()) {
          if (reg2base.find(inst->addr) != reg2base.end()) {
            auto base = reg2base[inst->addr];
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(base)) {
              pos = pos->idom;
            }
            Reg dep;
            if (!pos) {
              assert(false);
            } else {
              dep = alloc_map[pos][base];
            }
            inst->remove_use_def();
            auto new_inst =
                new ir::insns::MemUse(inst->dst, dep, inst->addr, false);
            iter->reset(new_inst);
            new_inst->bb = bb;
            new_inst->add_use_def();
            use_before_def[bb][base].insert(new_inst->dst);
          }
        }
        TypeCase(inst, ir::insns::Store *, iter->get()) {
          if (reg2base.find(inst->addr) != reg2base.end()) {
            auto base = reg2base[inst->addr];
            inst->remove_use_def();
            Reg dst = func->new_reg(ScalarType::String);
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(base)) {
              pos = pos->idom;
            }
            Reg dep;
            if (!pos) {
              assert(false);
            } else {
              dep = alloc_map[pos][base];
            }
            auto new_inst =
                new ir::insns::MemDef(dst, dep, inst->addr, inst->val, false,
                                      use_before_def[bb][base]);
            iter->reset(new_inst);
            new_inst->bb = bb;
            new_inst->add_use_def();
            alloc_map[bb][base] = dst;
            use_before_def[bb][base].clear();
          }
        }
        TypeCase(inst, ir::insns::Call *, iter->get()) {
          auto use = inst->use();
          unordered_map<Reg, Reg> use2def;
          unordered_map<std::string, Reg> name2def;
          for (auto reg : use) {
            if (reg2base.find(reg) != reg2base.end()) {
              auto base = reg2base[reg];
              BasicBlock *pos = bb;
              while (pos && !alloc_map[pos].count(base)) {
                pos = pos->idom;
              }
              Reg dst = func->new_reg(ScalarType::String);
              Reg src;
              if (!pos) {
                assert(false);
              } else {
                src = alloc_map.at(pos).at(base);
              }
              auto new_inst = new ir::insns::MemUse(dst, src, reg, true);
              bb->insns.insert(iter,
                               std::unique_ptr<ir::Instruction>(new_inst));
              new_inst->bb = bb;
              new_inst->add_use_def();
              inst->change_use(reg, dst);
              use2def[reg] = dst;
              use_before_def[bb][base].insert(dst);
            }
          }
          for (auto &gvar : used_gvar[inst->func]) {
            if (!name2base.count(gvar)) {
              continue;
            }
            auto base = name2base.at(gvar);
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(base)) {
              pos = pos->idom;
            }
            Reg dst = func->new_reg(ScalarType::String);
            Reg src;
            if (!pos) {
              assert(false);
            } else {
              src = alloc_map.at(pos).at(base);
            }
            auto new_inst = new ir::insns::MemUse(dst, src, src, true);
            bb->insns.insert(iter, std::unique_ptr<ir::Instruction>(new_inst));
            new_inst->bb = bb;
            new_inst->add_use_def();
            name2def[gvar] = dst;
            use_before_def[bb][base].insert(dst);
          }
          for (auto &each : name2def) {
            inst->global_use.push_back(each.second);
          }
          inst->add_use_def();
          iter++;
          for (auto &gvar : used_gvar[inst->func]) {
            if (!name2base.count(gvar)) {
              continue;
            }
            auto base = name2base[gvar];
            Reg dst = func->new_reg(ScalarType::String);
            BasicBlock *pos = bb;
            while (pos && !alloc_map[pos].count(base)) {
              pos = pos->idom;
            }
            Reg dep;
            if (!pos) {
              assert(false);
            } else {
              dep = alloc_map[pos][base];
            }
            auto new_inst =
                new ir::insns::MemDef(dst, dep, name2def.at(gvar), inst->dst,
                                      true, use_before_def[bb][base]);
            iter = bb->insns.insert(iter,
                                    std::unique_ptr<ir::Instruction>(new_inst));
            new_inst->bb = bb;
            new_inst->add_use_def();
            alloc_map[bb][base] = dst;
            use_before_def[bb][base].clear();
          }
          for (auto reg : use) {
            if (reg2base.find(reg) != reg2base.end()) {
              auto base = reg2base[reg];
              Reg dst = func->new_reg(ScalarType::String);
              BasicBlock *pos = bb;
              while (pos && !alloc_map[pos].count(base)) {
                pos = pos->idom;
              }
              Reg dep;
              if (!pos) {
                assert(false);
              } else {
                dep = alloc_map[pos][base];
              }
              auto new_inst = new ir::insns::MemDef(dst, dep, use2def.at(reg),
                                                    inst->dst, true, use_before_def[bb][base]);
              iter = bb->insns.insert(
                  iter, std::unique_ptr<ir::Instruction>(new_inst));
              new_inst->bb = bb;
              new_inst->add_use_def();
              alloc_map[bb][base] = dst;
              use_before_def[bb][base].clear();
            }
          }
        }
        TypeCase(inst, ir::insns::Phi *, iter->get()) {
          if (phi2mem.count(inst)) {
            alloc_map[bb][phi2mem.at(inst)] = inst->dst;
            inst->use_before_def = use_before_def[bb][phi2mem.at(inst)];
            use_before_def[bb][phi2mem.at(inst)].clear();
          }
        }
        TypeCase(inst, ir::insns::Alloca *, iter->get()) {
          alloc_map[bb][inst->dst] = inst->dst;
        }
        TypeCase(inst, ir::insns::LoadAddr *, iter->get()) {
          alloc_map[bb][inst->dst] = inst->dst;
        }
        iter++;
        continue;
      }
      for (auto &succ : bb->succ) {
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
      for (auto dom : bb->dom) {
        stack.push_back(dom);
      }
    }
    for (auto phi : phi2mem) {
      phi.first->add_use_def();
    }
    remove_unused_phi(func);
  }
}

void array_ssa_destruction(ir::Program *prog) {
  for (auto &each : prog->functions) {
    Function *func = &each.second;
    for (auto &bb : func->bbs) {
      for (auto iter = bb->insns.begin(); iter != bb->insns.end();) {
        auto inst = iter->get();
        TypeCase(memdef, ir::insns::MemDef *, inst) {
          if (memdef->call_def) {
            memdef->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          } else {
            memdef->remove_use_def();
            auto new_inst =
                new ir::insns::Store(memdef->store_dst, memdef->store_val);
            iter->reset(new_inst);
            new_inst->bb = bb.get();
            new_inst->add_use_def();
          }
        }
        else TypeCase(memuse, ir::insns::MemUse *, inst) {
          if (memuse->call_use) {
            copy_propagation(func->use_list, memuse->dst, memuse->load_src);
            memuse->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          } else {
            memuse->remove_use_def();
            auto new_inst = new ir::insns::Load(memuse->dst, memuse->load_src);
            iter->reset(new_inst);
            new_inst->bb = bb.get();
            new_inst->add_use_def();
          }
        }
        else TypeCase(phi, ir::insns::Phi *, inst) {
          if (phi->array_ssa) {
            phi->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          }
        }
        else TypeCase(call, ir::insns::Call *, inst) {
          call->remove_use_def();
          call->global_use.clear();
          call->add_use_def();
        }
        iter++;
      }
    }
  }
}

} // namespace mediumend
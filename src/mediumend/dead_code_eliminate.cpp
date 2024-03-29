#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using std::vector;
using std::unordered_set;

using ir::Reg;
using ir::Loop;
using ir::BasicBlock;
using ir::Instruction;

static ir::Program* cur_prog = nullptr;

void detect_pure_function(ir::Program *prog, ir::Function *func) {
  unordered_map<Reg, int> reg2base;
  int i = 0;
  for(auto &type : func->sig.param_types){
    i++;
    if(type.is_array()){
      func->pure = 0;
      reg2base[Reg(type.base_type, i)] = i;
    }
  }
  for (auto &bb : func->bbs) {
    for (auto &inst : bb->insns) {
      // 使用了全局变量，不能当作纯函数
      if(auto x = dynamic_cast<ir::insns::LoadAddr *>(inst.get())){
        func->pure = 0;
        func->array_ssa_pure = 0;
      }
      TypeCase(gep, ir::insns::GetElementPtr *, inst.get()){
        if(reg2base.count(gep->base)){
          reg2base[gep->dst] = reg2base.at(gep->base);
        }
      }
      TypeCase(store, ir::insns::Store *, inst.get()){
        if(reg2base.count(store->addr)){
          func->only_load_param = 0;
        }
      }
      // 此处为了便于处理递归函数，因此这么做
      TypeCase(call, ir::insns::Call *, inst.get()) {
        if(call->func == func->name){
          continue;
        }
        auto call_func = prog->functions.find(call->func);
        if(call_func != prog->functions.end()){
          if(call_func->second.pure == -1){
            detect_pure_function(prog, &call_func->second);
          }
          if(!call_func->second.is_pure()){
            func->pure = 0;
          }
          if(!call_func->second.is_array_ssa_pure()){
            func->array_ssa_pure = 0;
          }
        } else {
          func->pure = 0;
          func->array_ssa_pure = 0;
        }
      }
    }
  }
  if(func->pure == -1){
    func->pure = 1;
  }
  if(func->array_ssa_pure == -1){
    func->array_ssa_pure = 1;
  }
  if(func->only_load_param == -1){
    func->only_load_param = 1;
  }
}

void mark_pure_func(ir::Program *prog){
  for(auto &each : prog->functions){
    each.second.array_ssa_pure = -1;
    each.second.pure = -1;
  }
  detect_pure_function(prog, &prog->functions.at("main"));
  prog->functions.at("main").pure = 0;
  prog->functions.at("main").array_ssa_pure = 0;
}

void remove_unused_function(ir::Program *prog){
  std::unordered_set<std::string> used;
  std::vector<std::string> stack;
  stack.emplace_back("main");
  while(!stack.empty()){
    auto func_name = stack.back();
    stack.pop_back();
    if(prog->functions.find(func_name) == prog->functions.end()){
      continue;
    }
    used.insert(func_name);
    auto &func = prog->functions.at(func_name);
    for(auto &bb : func.bbs){
      for(auto &ins : bb->insns){
        if(auto call = dynamic_cast<ir::insns::Call *>(ins.get())){
          if(!used.count(call->func)){
            stack.emplace_back(call->func);
          }
        }
      }
    }
  }
  for(auto iter = prog->functions.begin(); iter != prog->functions.end();){
    if(!used.count(iter->first)){
      iter = prog->functions.erase(iter);
    }else{
      iter++;
    }
  }
}

void check_inst(ir::Instruction *inst){
  assert(inst);
  assert(inst->bb);
}


static unordered_set<Instruction *> get_effective_use(Instruction *inst,
                                               ir::Function *func) {
  unordered_set<Instruction *> ret;
  unordered_set<Instruction *> visited;
  vector<Instruction *> stack;
  stack.push_back(inst);
  while (stack.size()) {
    inst = stack.back();
    stack.pop_back();
    if (visited.count(inst)) {
      continue;
    }
    visited.insert(inst);
    TypeCase(output, ir::insns::Output *, inst) {
      auto uses = func->use_list[output->dst];
      for (auto each : uses) {
        stack.push_back(each);
      }
    }
    TypeCase(store, ir::insns::Store *, inst) { ret.insert(store); }
    TypeCase(memdef, ir::insns::MemDef *, inst) { ret.insert(memdef); }
    TypeCase(call, ir::insns::Call *, inst) { 
      if(!cur_prog->functions.count(call->func) || !cur_prog->functions.at(call->func).is_pure()){
        ret.insert(call);
      } 
    }
    TypeCase(memuse, ir::insns::MemUse *, inst) { ret.insert(memuse); }
  }
  return ret;
}


void remove_uneffective_inst(ir::Program *prog){
  cur_prog = prog;
  unordered_set<ir::Instruction *> useful_inst;
  unordered_set<ir::Function *> func_stack;
  unordered_set<std::string> visited_func;
  for(auto &each : prog->functions){
    each.second.ret_used = true;
  }
  prog->functions.at("main").ret_used = true;
  func_stack.insert(&prog->functions.at("main"));
  while(func_stack.size()){
    auto iter = func_stack.begin();
    ir::Function *func = *iter;
    func_stack.erase(iter);
    auto &bbs = func->bbs;
    unordered_set<ir::Instruction *> stack;
    visited_func.insert(func->name);
    for (auto &bb : bbs) {
      auto &insns = bb->insns;
      for (auto &inst : insns) {
        TypeCase(call, ir::insns::Call *, inst.get()){
          if(prog->functions.find(call->func) == prog->functions.end()){
            stack.insert(call);
          } else {
            if(in_array_ssa()){
              if(!prog->functions.at(call->func).is_array_ssa_pure()){
                stack.insert(call);
              }
            } else {
              if(!prog->functions.at(call->func).is_pure()){
                stack.insert(call);
              }
            }
          }
        } else TypeCase(term, ir::insns::Terminator *, inst.get()){
          TypeCase(ret, ir::insns::Return *, inst.get()){
            if(func->is_ret_used() || !func->sig.ret_type.has_value()){
              stack.insert(term);
            }
          } else {
            stack.insert(term);
          }
        } else TypeCase(store, ir::insns::Store *, inst.get()){
          stack.insert(store);
        } else TypeCase(memdef, ir::insns::MemDef *, inst.get()){
          if(func->name != "main"){
            stack.insert(memdef);
          }
        }
      }
    }
    while(!stack.empty()){
      auto ins = stack.begin();
      auto inst = *ins;
      stack.erase(ins);
      if(useful_inst.count(inst)){
        continue;
      }
      useful_inst.insert(inst);
      auto regs = inst->use();
      for(auto reg : regs){
        if(func->def_list.find(reg) != func->def_list.end()){
          auto def = func->def_list.at(reg);
          if(useful_inst.count(def)){
            continue;
          }
          check_inst(def);
          stack.insert(def);
        }
      }
      TypeCase(call, ir::insns::Call *, inst){
        if(call->func == func->name){
          auto used = get_effective_use(call, func);
          if(used.size()){
            if(!func->ret_used){
              func->ret_used = true;
              func_stack.insert(func);
            }
            func->ret_used = true;
          }
          continue;
        }
        if(prog->functions.count(call->func)){
          auto &callee = prog->functions.at(call->func);
          if(func->use_list[call->dst].size()){
            if(!callee.ret_used){
              callee.ret_used = true;
              func_stack.insert(&callee);
            }
          }
          if(!visited_func.count(call->func)){
            func_stack.insert(&callee);
          }
        }
      }
    }
  }
  // for(auto &each : prog->functions){
  //   bool remove_ret = true;
  //   if(each.second.ret_used || !each.second.sig.ret_type.has_value()){
  //     remove_ret = false;
  //   } else {
  //     each.second.sig.ret_type.reset();
  //   }
  //   auto &bbs = each.second.bbs;
  //   for(auto &bb : bbs){
  //     for(auto &inst : bb->insns){
  //       TypeCase(call, ir::insns::Call *, inst.get()){
  //         if(prog->functions.count(call->func) && !prog->functions.at(call->func).is_ret_used()){
  //           call->dst.type = ScalarType::String;
  //         }
  //       }
  //     }
  //     if(remove_ret){
  //       auto ret = dynamic_cast<ir::insns::Return *>(bb->insns.back().get());
  //       if(ret){
  //         ret->remove_use_def();
  //         ret->val.reset();
  //         useful_inst.insert(ret);
  //       }
  //     }
  //   }
  // }
  for(auto &each : prog->functions){
    auto &bbs = each.second.bbs;
    for(auto &bb : bbs){
      bb->remove_if([&](ir::Instruction *ir) {
        return !useful_inst.count(ir);
      });
    }
  }
}

// Clean Alogritm: Engineering a Compiler, Chap 10.2
bool eliminate_useless_cf_one_pass(ir::Function *func){
  func->clear_visit();
  vector<BasicBlock *> stack;
  if(func->bbs.empty()){
    return false;
  }
  stack.push_back(func->bbs.front().get());
  vector<BasicBlock *> order;
  func->bbs.front().get()->visit = true;
  BasicBlock *entry = func->bbs.front().get();
  while(stack.size()){
    auto bb = stack.back();
    stack.pop_back();
    order.push_back(bb);
    auto &succ_bb = bb->succ;
    for(auto next : succ_bb){
      if(!next->visit){
        stack.push_back(next);
        next->visit = true;
      }
    }
  }
  bool ret = false;
  func->clear_visit();
  for(int i = order.size() - 1; i >= 0; i--){
    auto bb = order[i];
    auto &inst = bb->insns.back();
    TypeCase(branch, ir::insns::Branch *, inst.get()){
      if(branch->true_target == branch->false_target){
        branch->remove_use_def();
        inst.reset(new ir::insns::Jump(branch->true_target));
        inst->bb = bb;
        ret = true;
      }
    }
    TypeCase(jmp, ir::insns::Jump *, inst.get()){
      auto target = jmp->target;
      if(bb->insns.size() == 1) {
        if(auto phi = dynamic_cast<ir::insns::Phi *>(target->insns.front().get())){
          bool eliminate = true;
          for(auto each : bb->prev){
            if(target->prev.count(each)){
              eliminate = false;
              break;
            }
          }
          if(!eliminate) {
            continue;
          }
        }
        if(entry == bb){
          continue;
        }
        for(auto &pre : bb->prev){
          auto &last = pre->insns.back();
          pre->change_succ(bb, target);
          target->prev.insert(pre);
          for(auto &ins : target->insns){
            TypeCase(phi, ir::insns::Phi *, ins.get()){
              if(phi->incoming.count(bb)){
                phi->incoming[pre] = phi->incoming[bb];
              }
            } else {
              break;
            }
          }
        }
        target->prev.erase(bb);
        for(auto &ins : target->insns){
          TypeCase(phi, ir::insns::Phi *, ins.get()){
            phi->incoming.erase(bb);
          } else {
            break;
          }
        }
        bb->visit = true;
        ret = true;
        continue;
      }
      if(target->prev.size() == 1){
        bb->succ.erase(target);
        bb->succ.insert(target->succ.begin(), target->succ.end());
        bb->insns.pop_back();
        for(auto suc: target->succ){
          suc->prev.erase(target);
          suc->prev.insert(bb);
          for(auto &inst : suc->insns){
            TypeCase(phi, ir::insns::Phi *, inst.get()){
              if(phi->incoming.count(target)){
                phi->incoming[bb] = phi->incoming[target];
                phi->incoming.erase(target);
              }
            } else {
              break;
            }
          }
        }
        for(auto iter = target->insns.begin(); iter != target->insns.end();){
          TypeCase(phi, ir::insns::Phi *, iter->get()){
            assert(phi->incoming.size() == 1);
            copy_propagation(func->use_list, phi->dst, phi->incoming.begin()->second);
            phi->remove_use_def();
            iter = target->insns.erase(iter);
          } else {
            iter->get()->bb = bb;
            iter++;
          }
        }
        bb->insns.splice(bb->insns.end(), target->insns);
        target->visit = true;
        target->succ.clear();
        ret = true;
        continue;
      }
      if(target->insns.size() == 1){
        TypeCase(br, ir::insns::Branch *, target->insns.back().get()){
          bb->succ.erase(target);
          target->prev.erase(bb);
          bb->succ.insert(br->true_target);
          bb->succ.insert(br->false_target);
          br->true_target->prev.insert(bb);
          br->false_target->prev.insert(bb);
          for(auto &ins : br->true_target->insns){
            TypeCase(phi, ir::insns::Phi *, ins.get()){
              if(phi->incoming.count(target)){
                phi->incoming[bb] = phi->incoming[target];
              }
            } else {
              break;
            }
          }
          for(auto &ins : br->false_target->insns){
            TypeCase(phi, ir::insns::Phi *, ins.get()){
              if(phi->incoming.count(target)){
                phi->incoming[bb] = phi->incoming[target];
              }
            } else {
              break;
            }
          }
          auto new_inst = new ir::insns::Branch(br->val, br->true_target, br->false_target);
          bb->insns.back().reset(new_inst);
          new_inst->bb = bb;
          new_inst->add_use_def();
          ret = true;
          continue;
        }
      }
    }
  }
  for(auto iter = func->bbs.begin(); iter != func->bbs.end();){
    if(iter->get()->visit){
      iter = func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
  return ret;
}

void clean_useless_cf(ir::Program *prog){
  for(auto &each : prog->functions){
    bool change = true;
    while(change){
      change = eliminate_useless_cf_one_pass(&each.second);
    }
  }
}


bool remove_useless_loop(ir::Function *func) {
  func->loop_analysis();
  bool changed = false;
  unordered_set<BasicBlock *> remove_bbs;
  for (auto &loop_ptr : func->loops) {
    Loop * loop = loop_ptr.get();
    if(!loop->no_inner){
      continue;
    }
    BasicBlock *idom_prev = loop->header->idom;
    bool check = true;
    unordered_set<BasicBlock*> loop_bbs;
    vector<BasicBlock *> stack;
    stack.push_back(loop->header);
    while(stack.size()){
      auto bb = stack.back();
      stack.pop_back();
      if(bb->loop != loop){
        continue;
      }
      loop_bbs.insert(bb);
      for(auto each : bb->dom){
        stack.push_back(each);
      }
    }
    // 这里判断只有一个入口
    for(auto each : loop->header->prev){
      if(each != idom_prev){
        if(!each->loop || each->loop != loop){
          check = false;
          break;
        }
      }
    }
    unordered_set<Reg> defs;
    BasicBlock * out = nullptr;
    BasicBlock* out_bb = nullptr;

    for (BasicBlock *bb : loop_bbs) {
      if(!check){
        break;
      }
      for(auto &inst : bb->insns){
        TypeCase(output, ir::insns::Output *, inst.get()){
          auto uses = func->use_list[output->dst];
          for(auto each : uses){
            if(!loop_bbs.count(each->bb)){
              check = false;
              break;
            }
          }
        }
        TypeCase(call, ir::insns::Call *, inst.get()){
          if(!cur_prog->functions.count(call->func) || !cur_prog->functions.at(call->func).is_pure()){
            check = false;
            break;
          }
        }
        TypeCase(store, ir::insns::Store *, inst.get()){
          check = false;
          break;
        }
      }
      // 这里判断只有一个出口
      for (BasicBlock *suc : bb->succ) {
        if (!suc->loop || suc->loop != loop) {
          if(out && suc != out){
            check = false;
            break;
          } else {
            out_bb = bb;
            out = suc;
          }
        }
      }
    }
    if(!out || !check){
      continue;
    }
    changed = true;
    // 标记所有需要删除的block，然后统一删除
    remove_bbs.merge(loop_bbs);
    // 处理前驱后继关系
    idom_prev->change_succ(loop->header, out);
    out->change_prev(out_bb, idom_prev);
    
  }
  for(auto &each : func->bbs){
    auto bb = each.get();
    if(remove_bbs.count(bb)){
      for(auto &inst : bb->insns){
        inst->remove_use_def();
      }
      for(auto prev : bb->prev){
        prev->succ.erase(bb);
      }
      for(auto succ : bb->succ){
        succ->prev.erase(bb);
      }
    }
  }
  for(auto iter = func->bbs.begin(); iter != func->bbs.end();){
    auto bb = iter->get();
    if(remove_bbs.count(bb)){
      iter = func->bbs.erase(iter);
    } else {
      iter++;
    }
  }
  return changed;
}

void remove_useless_loop(ir::Program * prog){
  cur_prog = prog;
  for(auto &func : prog->functions){
    while(remove_useless_loop(&func.second)) {}
  }
  cur_prog = nullptr;
}

}
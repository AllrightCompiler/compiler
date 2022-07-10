#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

namespace mediumend {

using std::vector;
using std::unordered_set;


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

void detect_pure_function(ir::Program *prog, ir::Function *func) {
  if(func->pure != -1){
    return;
  }
  for(auto &type : func->sig.param_types){
    if(type.is_array()){
      func->pure = 0;
      return;
    }
  }
  for (auto &bb : func->bbs) {
    for (auto &inst : bb->insns) {
      // 修改了全局变量，不是纯函数
      TypeCase(store, ir::insns::Store *, inst.get()){
        if(func->global_addr.count(store->addr)){
          func->pure = 0;
          return;
        }
      }
      // 使用了全局变量，不能当作纯函数
      if(auto x = dynamic_cast<ir::insns::LoadAddr *>(inst.get())){
        func->pure = 0;
        return;
      }
      // 此处为了便于处理递归函数，因此这么做
      TypeCase(call, ir::insns::Call *, inst.get()) {
        auto call_func = prog->functions.find(call->func);
        if(call_func != prog->functions.end()){
          if(call_func->second.pure == 1){
            continue;
          }
        }
        func->pure = 0;
        return;
      }
    }
  }
  func->pure = 1;
}

void mark_pure_func(ir::Program *prog){
  prog->functions.at("main").pure = 0;
  for(auto &func : prog->functions){
    detect_pure_function(prog, &func.second);
  }
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

void remove_uneffective_inst(ir::Program *prog){
  for(auto &each : prog->functions){
    ir::Function *func = &each.second;
    auto &bbs = func->bbs;
    vector<ir::Instruction *> stack;
    unordered_set<ir::Instruction *> remove_inst_set; 
    for (auto &bb : bbs) {
      auto &insns = bb->insns;
      for (auto &inst : insns) {
        TypeCase(output, ir::insns::Output *, inst.get()){
          if(auto call = dynamic_cast<ir::insns::Call *>(output)){
            if(!prog->functions.count(call->func)){
              continue;
            }
            if(prog->functions.at(call->func).pure == 0){
              continue;
            }
          }
          auto &reg = output->dst;
          if (reg.id && func->use_list[reg].size() == 0) {
            stack.push_back(output);
          }
        }
      }
    }
    while(!stack.empty()){
      auto inst = stack.back();
      stack.pop_back();
      auto regs = get_inst_use_reg(inst);
      inst->remove_use_def();
      for(auto reg : regs){
        if(func->use_list[reg].size() == 0){
          auto output = func->def_list[reg];
          if(auto call = dynamic_cast<ir::insns::Call *>(inst)){
            continue;
          }
          assert(output);
          stack.push_back(output);
        }
      }
      remove_inst_set.insert(inst);
    }
    for(auto &bb : bbs){
      bb->remove_if([=](ir::Instruction *ir) {
        return remove_inst_set.count(ir);
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
  for(int i = order.size() - 1; i >= 0; i--){
    auto bb = order[i];
    auto &inst = bb->insns.back();
    TypeCase(branch, ir::insns::Branch *, inst.get()){
      if(branch->true_target == branch->false_target){
        branch->remove_use_def();
        inst.reset(new ir::insns::Jump(branch->true_target));
        ret = true;
      }
    }
    TypeCase(jmp, ir::insns::Jump *, inst.get()){
      auto target = jmp->target;
      if(bb->insns.size() == 1) {
        if(auto phi = dynamic_cast<ir::insns::Phi *>(target->insns.front().get())){
          continue;
        }
        for(auto &pre : bb->prev){
          auto &last = pre->insns.back();
          TypeCase(br, ir::insns::Branch *, last.get()){
            if(br->true_target == bb){
              br->true_target = target;
            }
            if(br->false_target == bb){
              br->false_target = target;
            }
          }
          TypeCase(j, ir::insns::Jump *, last.get()){
            if(j->target == bb){
              j->target = target;
            }
          }
          pre->succ.erase(bb);
          pre->succ.insert(target);
          target->prev.erase(bb);
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
        for(auto &ins : target->insns){
          TypeCase(phi, ir::insns::Phi *, ins.get()){
            phi->incoming.erase(bb);
          } else {
            break;
          }
        }
        for(auto iter = func->bbs.begin(); iter != func->bbs.end(); ++iter){
          if(iter->get() == bb){
            func->bbs.erase(iter);
            break;
          }
        }
        ret = true;
        continue;
      }
      if(target->prev.size() == 1){
        bb->succ.erase(target);
        bb->succ.insert(target->succ.begin(), target->succ.end());
        bb->insns.pop_back();
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
        for(auto iter = func->bbs.begin(); iter != func->bbs.end(); ++iter){
          if(iter->get() == target){
            for(auto suc : target->succ){
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
            func->bbs.erase(iter);
            break;
          }
        }
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
          br->remove_use_def();
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

void eliminate_useless_store(ir::Program *prog){

}

}
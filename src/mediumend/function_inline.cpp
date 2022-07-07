#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using ir::Program;
using std::string;
using std::unordered_set;
using std::unordered_map;
using std::vector;

void inline_single_func(Function *caller, Program *prog, unordered_set<string> &cursive_calls){
  vector<ir::insns::Call *> calls;
  for(auto & bb : caller->bbs){
    for(auto &inst : bb->insns){
      TypeCase(call, ir::insns::Call *, inst.get()){
        if(!prog->functions.count(call->func) || cursive_calls.count(call->func)){
          continue;
        }
        calls.push_back(call);
      }
    }
  }
  unordered_map<BasicBlock *, BasicBlock *> bb_map;
  unordered_map<Reg, Reg> reg_map;
  int i = 0;
  for(auto inst : calls){
    auto callee = &prog->functions[inst->func];
    BasicBlock *inst_bb = nullptr;
    auto name = "inline" + std::to_string(i);
    for(auto &bb : caller->bbs){
      for(auto &ins : bb->insns){
        if(ins.get() == inst){
          inst_bb = bb.get();
        }
      }
    }
    bb_map.clear();
    reg_map.clear();
    auto &args = inst->args;
    auto &paras = callee->sig.param_types;
    auto ret_bb = new BasicBlock();
    ret_bb->label = name + "_ret";
    auto &succ = caller->cfg->succ;
    auto &prev = caller->cfg->prev;
    for(auto suc : succ[inst_bb]){
      prev[suc].erase(inst_bb);
      prev[suc].insert(ret_bb);
      for(auto &ins : suc->insns){
        TypeCase(phi, ir::insns::Phi *, ins.get()){
          for(auto &[bb, reg] : phi->incoming){
            if(bb == inst_bb){
              phi->incoming.erase(bb);
              phi->incoming[ret_bb] = reg;
            }
          }
        } else {
          break;
        }
      }
    }
    for(auto iter = caller->bbs.begin(); iter != caller->bbs.end(); ++iter){
      if(iter->get() == inst_bb){
        iter++;
        caller->bbs.insert(iter, std::unique_ptr<BasicBlock>(ret_bb));
        for(auto riter = callee->bbs.rbegin(); riter != callee->bbs.rend(); ++riter){
          auto copy = new BasicBlock;
          copy->label = name + "_" + riter->get()->label;
          bb_map[riter->get()] = copy;
          caller->bbs.insert(iter, std::unique_ptr<BasicBlock>(copy));
        }
        break;
      }
    }
    ir::insns::Phi *ret_phi = nullptr;
    auto inst_iter = inst_bb->insns.begin();
    for(; inst_iter != inst_bb->insns.end(); inst_iter++){
      if(inst_iter->get() == inst){
        if(prog->functions[inst->func].sig.ret_type.has_value()){
          ret_phi = new ir::insns::Phi(inst->dst);
          ret_bb->push(ret_phi);
        }
        inst_iter->reset(new ir::insns::Jump(bb_map[callee->bbs.front().get()]));
        inst_iter++;
        break;
      }
    }
    ret_bb->insns.splice(ret_bb->insns.end(), inst_bb->insns, inst_iter, inst_bb->insns.end());
    for(auto each : callee->def_list){
      auto reg = caller->new_reg(each.first.type);
      reg_map[each.first] = reg;
    }
    callee->cfg->compute_rpo();
    auto &rpo = callee->cfg->rpo;
    for(auto raw_bb : rpo){
      BasicBlock* mapped_bb = bb_map[raw_bb];
      for(auto &inst : raw_bb->insns){
        ir::Instruction * inst_copy;
        TypeCase(phi, ir::insns::Phi *, inst.get()){
          inst_copy = new ir::insns::Phi(reg_map[phi->dst]);
          for(auto &in : phi->incoming){
            ((ir::insns::Phi *)inst_copy)->incoming[bb_map[in.first]] = reg_map[in.second];
          }
        } else TypeCase(binary, ir::insns::Binary *, inst.get()){
          inst_copy = new ir::insns::Binary(reg_map[binary->dst], binary->op, reg_map[binary->src1], reg_map[binary->src2]);
        } else TypeCase(unary, ir::insns::Unary *, inst.get()){
          inst_copy = new ir::insns::Unary(reg_map[unary->dst], unary->op, reg_map[unary->src]);
        } else TypeCase(load, ir::insns::Load *, inst.get()){
          inst_copy = new ir::insns::Load(reg_map[load->dst], reg_map[load->addr]);
        } else TypeCase(store, ir::insns::Store *, inst.get()){
          inst_copy = new ir::insns::Store(reg_map[store->addr], reg_map[store->val]);
        } else TypeCase(call, ir::insns::Call *, inst.get()){
          vector<Reg> args_copy;
          for(auto &arg : args){
            args_copy.push_back(reg_map[arg]);
          }
          inst_copy = new ir::insns::Call(reg_map[call->dst], call->func, args_copy);
        } else TypeCase(ret, ir::insns::Return *, inst.get()){
          if(ret_phi){
            ret_phi->incoming[mapped_bb] = reg_map[ret->val.value()];
          }
          inst_copy = new ir::insns::Jump(ret_bb);
        } else TypeCase(jump, ir::insns::Jump *, inst.get()){
          inst_copy = new ir::insns::Jump(bb_map[jump->target]);
        } else TypeCase(branch, ir::insns::Branch *, inst.get()){
          inst_copy = new ir::insns::Branch(reg_map[branch->val], bb_map[branch->true_target], bb_map[branch->false_target]);
        } else TypeCase(loadimm, ir::insns::LoadImm *, inst.get()){
          inst_copy = new ir::insns::LoadImm(reg_map[loadimm->dst], loadimm->imm);
        } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst.get()){
          inst_copy = new ir::insns::LoadAddr(reg_map[loadaddr->dst], loadaddr->var_name);
        } else TypeCase(alloc, ir::insns::Alloca *, inst.get()){
          inst_copy = new ir::insns::Alloca(reg_map[alloc->dst], alloc->type);
        } else TypeCase(getptr, ir::insns::GetElementPtr *, inst.get()){
          vector<Reg> indexs_copy;
          for(auto &index : getptr->indices){
            indexs_copy.push_back(reg_map[index]);
          }
          inst_copy = new ir::insns::GetElementPtr(reg_map[getptr->dst], getptr->type, reg_map[getptr->base], indexs_copy);
        } else TypeCase(convey, ir::insns::Convert *, inst.get()){
          inst_copy = new ir::insns::Convert(reg_map[convey->src], reg_map[convey->dst]);
        } else {
          assert(false);
        }
        inst_copy->add_use_def(caller->use_list, caller->def_list);
        mapped_bb->push(inst_copy);
      }
      if(callee->cfg->succ[raw_bb].empty()){
        auto ret = dynamic_cast<ir::insns::Jump *>(mapped_bb->insns.back().get());
        if(!ret){
          // mapped_bb->push(new ir::insns::Jump(ret_bb));
        }
      }
    }
    if(ret_phi){
      ret_phi->add_use_def(caller->use_list, caller->def_list);
    }
    caller->cfg->build();
  }
}

void function_inline(ir::Program *prog) {
  unordered_map<string, vector<string>> call_prevs;
  unordered_map<string, int> use_cnt;
  unordered_set<string> cursive_calls;
  for(auto & func : prog->functions){
    if(!use_cnt[func.first]){
      use_cnt[func.first] = 0;
    }
    for(auto &bb : func.second.bbs){
      for(auto &insn : bb->insns){
        TypeCase(call, ir::insns::Call *, insn.get()){
          if(!prog->functions.count(call->func)){
            // 库函数直接跳过
            continue;
          }
          call_prevs[call->func].push_back(func.first);
          if(!use_cnt[func.first]){
            use_cnt[func.first] = 1;
          } else {
            use_cnt[func.first]++;
          }
        }
      }
    }
  }
  vector<string> stack;
  for(auto &func : use_cnt){
    if(func.second == 0){
      stack.push_back(func.first);
    }
  }
  while(stack.size()){
    auto &func_name = stack.back();
    stack.pop_back();
    auto &func = prog->functions[func_name];
    inline_single_func(&func, prog, cursive_calls);
    for(auto &prev : call_prevs[func_name]){
      use_cnt[prev]--;
      if(!use_cnt[prev]){
        stack.push_back(prev);
      }
    }
  }
}

} // namespace mediumend

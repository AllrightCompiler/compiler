#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;
using ir::Program;
using std::string;
using std::unordered_set;
using std::unordered_map;
using std::vector;

const int LONG_CALL_LEN = 100;
const int TOO_LONG_CALL_LEN = 1000000;

void inline_single_func(Function *caller, Program *prog, unordered_set<string> &cursive_or_long_calls){
  vector<ir::insns::Call *> calls;
  for(auto & bb : caller->bbs){
    for(auto &inst : bb->insns){
      TypeCase(call, ir::insns::Call *, inst.get()){
        if(!prog->functions.count(call->func) || cursive_or_long_calls.count(call->func)){
          continue;
        }
        calls.push_back(call);
      }
    }
  }
  int i = 0;
  for(auto inst : calls){
    i++;
    auto callee = &prog->functions.at(inst->func);
    BasicBlock *inst_bb = nullptr;
    auto name = "inline" + std::to_string(i);
    for(auto &bb : caller->bbs){
      for(auto &ins : bb->insns){
        if(ins.get() == inst){
          inst_bb = bb.get();
        }
      }
    }
    unordered_map<BasicBlock *, BasicBlock *> bb2bb;
    unordered_map<Reg, Reg> reg2reg;
    auto &args = inst->args;
    auto &paras = callee->sig.param_types;
    for(int i = 0; i < args.size(); i++){
      reg2reg[Reg(paras[i].base_type, i + 1)] = args[i];
    }
    auto ret_bb = new BasicBlock();
    ret_bb->label = name + "_ret";
    ret_bb->func = caller;
    for(auto suc : inst_bb->succ){
      suc->prev.erase(inst_bb);
      suc->prev.insert(ret_bb);
      for(auto &ins : suc->insns){
        TypeCase(phi, ir::insns::Phi *, ins.get()){
          if(phi->incoming.count(inst_bb)){
            auto reg = phi->incoming.at(inst_bb);
            phi->incoming.erase(inst_bb);
            phi->incoming[ret_bb] = reg;
          }
        } else {
          break;
        }
      }
    }
    for(auto iter = caller->bbs.begin(); iter != caller->bbs.end(); ++iter){
      if(iter->get() == inst_bb){
        iter++;
        for(auto riter = callee->bbs.begin(); riter != callee->bbs.end(); ++riter){
          auto copy = new BasicBlock;
          copy->label = name + "_" + riter->get()->label;
          copy->func = caller;
          bb2bb[riter->get()] = copy;
          caller->bbs.insert(iter, std::unique_ptr<BasicBlock>(copy));
        }
        caller->bbs.insert(iter, std::unique_ptr<BasicBlock>(ret_bb));
        break;
      }
    }
    ir::insns::Phi *ret_phi = nullptr;
    auto inst_iter = inst_bb->insns.begin();
    for(; inst_iter != inst_bb->insns.end(); inst_iter++){
      if(inst_iter->get() == inst){
        if(prog->functions.at(inst->func).sig.ret_type.has_value()){
          ret_phi = new ir::insns::Phi(inst->dst);
          ret_bb->insns.emplace_back(ret_phi);
          ret_phi->bb = ret_bb;
        }
        inst_iter->get()->remove_use_def();
        inst_iter->reset(new ir::insns::Jump(bb2bb.at(callee->bbs.front().get())));
        inst_iter->get()->bb = inst_bb;
        inst_iter++;
        break;
      }
    }
    for (auto it = inst_iter; it != inst_bb->insns.end(); it++) {
      it->get()->bb = ret_bb;
    }
    ret_bb->insns.splice(ret_bb->insns.end(), inst_bb->insns, inst_iter, inst_bb->insns.end());
    for(auto each : callee->def_list){
      auto reg = caller->new_reg(each.first.type);
      reg2reg[each.first] = reg;
    }
    vector<ir::insns::Alloca *> allocas;
    for(auto iter = callee->bbs.rbegin(); iter != callee->bbs.rend(); ++iter){
      auto raw_bb = iter->get();
      BasicBlock* mapped_bb = bb2bb.at(raw_bb);
      for(auto &inst : raw_bb->insns){
        ir::Instruction * inst_copy;
        TypeCase(phi, ir::insns::Phi *, inst.get()){
          inst_copy = new ir::insns::Phi(reg2reg.at(phi->dst));
          for(auto &in : phi->incoming){
            ((ir::insns::Phi *)inst_copy)->incoming[bb2bb.at(in.first)] = reg2reg.at(in.second);
          }
        } else TypeCase(binary, ir::insns::Binary *, inst.get()){
          inst_copy = new ir::insns::Binary(reg2reg.at(binary->dst), binary->op, reg2reg.at(binary->src1), reg2reg.at(binary->src2));
        } else TypeCase(unary, ir::insns::Unary *, inst.get()){
          inst_copy = new ir::insns::Unary(reg2reg.at(unary->dst), unary->op, reg2reg.at(unary->src));
        } else TypeCase(load, ir::insns::Load *, inst.get()){
          inst_copy = new ir::insns::Load(reg2reg.at(load->dst), reg2reg.at(load->addr));
        } else TypeCase(store, ir::insns::Store *, inst.get()){
          inst_copy = new ir::insns::Store(reg2reg.at(store->addr), reg2reg.at(store->val));
        } else TypeCase(call, ir::insns::Call *, inst.get()){
          vector<Reg> args_copy;
          for(auto &arg : call->args){
            args_copy.push_back(reg2reg.at(arg));
          }
          inst_copy = new ir::insns::Call(reg2reg.at(call->dst), call->func, args_copy);
        } else TypeCase(ret, ir::insns::Return *, inst.get()){
          if(ret_phi && ret->val.has_value()){
            ret_phi->incoming[mapped_bb] = reg2reg.at(ret->val.value());
          }
          inst_copy = new ir::insns::Jump(ret_bb);
        } else TypeCase(jump, ir::insns::Jump *, inst.get()){
          inst_copy = new ir::insns::Jump(bb2bb.at(jump->target));
        } else TypeCase(branch, ir::insns::Branch *, inst.get()){
          inst_copy = new ir::insns::Branch(reg2reg.at(branch->val), bb2bb.at(branch->true_target), bb2bb.at(branch->false_target));
        } else TypeCase(loadimm, ir::insns::LoadImm *, inst.get()){
          inst_copy = new ir::insns::LoadImm(reg2reg.at(loadimm->dst), loadimm->imm);
        } else TypeCase(loadaddr, ir::insns::LoadAddr *, inst.get()){
          inst_copy = new ir::insns::LoadAddr(reg2reg.at(loadaddr->dst), loadaddr->var_name);
        } else TypeCase(alloc, ir::insns::Alloca *, inst.get()){
          inst_copy = new ir::insns::Alloca(reg2reg.at(alloc->dst), alloc->type);
          // allocas.push_back(new ir::insns::Alloca(reg2reg.at(alloc->dst), alloc->type));
          // continue;
        } else TypeCase(getptr, ir::insns::GetElementPtr *, inst.get()){
          vector<Reg> indexs_copy;
          for(auto &index : getptr->indices){
            indexs_copy.push_back(reg2reg.at(index));
          }
          inst_copy = new ir::insns::GetElementPtr(reg2reg.at(getptr->dst), getptr->type, reg2reg.at(getptr->base), indexs_copy);
        } else TypeCase(convey, ir::insns::Convert *, inst.get()){
          inst_copy = new ir::insns::Convert(reg2reg.at(convey->dst), reg2reg.at(convey->src));
        } else TypeCase(switch_inst, ir::insns::Switch *, inst.get()){
          std::map<int, BasicBlock*> new_targets;
          for(auto each : switch_inst->targets){
            new_targets[each.first] = bb2bb.at(each.second);
          }
          inst_copy = new ir::insns::Switch(reg2reg.at(switch_inst->val), new_targets, bb2bb.at(switch_inst->default_target));
        } else {
          assert(false);
        }
        mapped_bb->push_back(inst_copy);
      }
    }
    // 找一个合适的位置插入，或者就等后面指令调度
    caller->cfg->build();
    // auto insert_bb = caller->bbs.front().get();
    // auto jmp = insert_bb->insns.back().release();
    // insert_bb->insns.pop_back();
    // for(int i = 0; i < allocas.size(); i++){
    //   insert_bb->push_back(allocas[i]);
    // }
    // insert_bb->insns.emplace_back(jmp);
    if(ret_phi){
      ret_phi->add_use_def();
    }
  }
}

void function_inline(ir::Program *prog) {
  unordered_map<string, vector<string>> call_prevs;
  unordered_map<string, int> use_cnt;
  unordered_set<string> cursive_or_long_calls;
  for(auto & func : prog->functions){
    if(!use_cnt[func.first]){
      use_cnt[func.first] = 0;
    }
    int length = 0;
    for(auto &bb : func.second.bbs){
      length += bb->insns.size();
      for(auto &insn : bb->insns){
        TypeCase(call, ir::insns::Call *, insn.get()){
          if(!prog->functions.count(call->func)){
            // 库函数直接跳过
            continue;
          }
          call_prevs[call->func].push_back(func.first);
          if(call->func == func.first){
            cursive_or_long_calls.insert(call->func);
          } else {
            if(!use_cnt[func.first]){
              use_cnt[func.first] = 1;
            } else {
              use_cnt[func.first]++;
            }
          }
        }
      }
    }
    if(length > LONG_CALL_LEN && length < TOO_LONG_CALL_LEN){
      cursive_or_long_calls.insert(func.first);
    }
  }
  vector<string> stack;
  for(auto &func : use_cnt){
    if(func.second == 0){
      stack.push_back(func.first);
    }
  }
  while(stack.size()){
    string func_name = stack.back();
    stack.pop_back();
    auto &func = prog->functions.at(func_name);
    inline_single_func(&func, prog, cursive_or_long_calls);
    for(auto &prev : call_prevs[func_name]){
      use_cnt[prev]--;
      if(!use_cnt[prev]){
        stack.push_back(prev);
      }
    }
  }
  ir_validation(prog);
}

} // namespace mediumend

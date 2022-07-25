#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>
namespace mediumend {

using ir::Function;
using ir::BasicBlock;
using ir::Instruction;
using ir::Reg;
using ir::Loop;
using std::unordered_map;

static ir::Program* cur_prog = nullptr;
static Function *cur_func = nullptr;
static unordered_map<Reg, Reg> reg_map;

unordered_map<Reg, int> find_const_int(Function *func){
  unordered_map<Reg, int> ret;
  for(auto &bb : func->bbs){
    for(auto &inst : bb->insns){
      TypeCase(loadimm, ir::insns::LoadImm *, inst.get()){
        if(loadimm->imm.type == ScalarType::Int){
          ret[loadimm->dst] = loadimm->imm.iv;
        }
      }
    }
  }
  return ret;
}

Instruction * copy_inst(Instruction *inst){
  Instruction *new_inst;
  TypeCase(output, ir::insns::Output *, inst){
    Reg dst = cur_func->new_reg(output->dst.type);
    reg_map[output->dst] = dst;
    TypeCase(binary, ir::insns::Binary *, output){
      new_inst = new ir::insns::Binary(dst, binary->op, reg_map.at(binary->src1), reg_map.at(binary->src2));
    } else TypeCase(unary, ir::insns::Unary *, output){
      new_inst = new ir::insns::Unary(dst, unary->op, reg_map.at(unary->src));
    } else TypeCase(loadaddr, ir::insns::LoadAddr *, output){
      new_inst = new ir::insns::LoadAddr(dst, loadaddr->var_name);
    } else TypeCase(gep, ir::insns::GetElementPtr *, output){
      auto indices = gep->indices;
      for(auto &index : indices){
        index = reg_map.at(index);
      }
      new_inst = new ir::insns::GetElementPtr(dst, gep->type, reg_map.at(gep->base), indices);
    } else TypeCase(load, ir::insns::Load *, output){
      new_inst = new ir::insns::Load(dst, reg_map.at(load->addr));
    } else TypeCase(phi, ir::insns::Phi *, output) {
      // TODO:leave to ZQH
      new_inst = new ir::insns::Phi(output->dst);
      for(auto &pair : phi->incoming){
        auto mapped_reg = reg_map.at(pair.second);
        auto mapped_bb = bb_map.at(pair.first);
        
      }
    } else TypeCase(call, ir::insns::Call *, output){
      auto args = call->args;
      for(auto &arg : args){
        arg = reg_map.at(arg);
      }
      new_inst = new ir::insns::Call(dst, call->func, args);
    } else TypeCase(loadimm, ir::insns::LoadImm *, output){
      new_inst = new ir::insns::LoadImm(dst, loadimm->imm);
    } 
  } else TypeCase(store, ir::insns::Store *, inst) {
    new_inst = new ir::insns::Store(reg_map.at(store->addr), reg_map.at(store->val));
  } else TypeCase(br, ir::insns::Branch *, inst){
    new_inst = new ir::insns::Branch(reg_map.at(br->val), bb_map.at(br->true_target), bb_map.at(br->false_target));
  } else TypeCase(jmp, ir::insns::Jump *, inst){
    new_inst = new ir::insns::Jump(bb_map.at(jmp->target));
  } else {
    assert(false);
  }
}

BasicBlock * copy_bb(BasicBlock *bb, int cnt = 0){
  BasicBlock* new_bb = new BasicBlock;
  new_bb->label = "copy_" + std::to_string(cnt) + bb->label;

  return new_bb;
}

void loop_unroll(Function * func){
  auto const_map = find_const_int(func);
  func->cfg->compute_dom();
  func->loop_analysis();
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
    unordered_set<Reg> defs;
    BasicBlock * out = nullptr;

    for (BasicBlock *bb : loop_bbs) {
      if(!check){
        break;
      }
      for(auto &inst : bb->insns){
        auto uses = inst->use();
        for(auto &use : uses){
          reg_map[use] = use;
        }
      }
      // 这里判断只有一个出口
      for (BasicBlock *suc : bb->succ) {
        if (!suc->loop || suc->loop != loop) {
          if(out && suc != out){
            check = false;
            break;
          } else {
            out = suc;
          }
        }
      }
    }
    if(!out || !check){
      continue;
    }

  }
}

}
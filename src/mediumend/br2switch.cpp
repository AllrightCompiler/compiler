#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;
using ir::BasicBlock;
using std::vector;
using std::map;

const static int BR_CNT = 7;

static Function * cur_func = nullptr;

bool check_bb_br(BasicBlock *bb, Reg reg, int &val, BasicBlock *&true_target, BasicBlock *&false_target) {
  auto term = bb->insns.back().get();
  if(bb->insns.size() == 3){
    TypeCase(br, ir::insns::Branch *, term){
      if(cur_func->def_list.count(br->val)){
        auto val_def = cur_func->def_list.at(br->val);
        TypeCase(binary, ir::insns::Binary *, val_def){
          if(binary->src1 == reg || binary->src2 == reg){
            if(binary->op == BinaryOp::Eq){
              Reg jmp_val = binary->src1;
              Reg imm_reg = binary->src2;
              if(binary->src2 == reg){
                jmp_val = binary->src2;
                imm_reg = binary->src1;
              }
              if(cur_func->def_list.count(imm_reg)){
                auto imm_def = cur_func->def_list.at(imm_reg);
                TypeCase(imm, ir::insns::LoadImm *, imm_def){
                  if(imm->imm.type == ScalarType::Int){
                    val = imm->imm.iv;
                    true_target = br->true_target;
                    false_target = br->false_target;
                    return true;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

void br2switch(Function *func) {
  cur_func = func;
  vector<BasicBlock *> stack;
  stack.push_back(func->bbs.front().get());
  bool found = false;
  map<int, BasicBlock *> switch_map;
  BasicBlock* default_target;
  unordered_set<BasicBlock *> internal_bbs;
  BasicBlock *bb;
  Reg jmp_val;
  while(stack.size() && !found){
    bb = stack.back();
    stack.pop_back();
    for(auto succ : bb->dom){
      stack.push_back(succ);
    }
    auto term = bb->insns.back().get();
    TypeCase(br, ir::insns::Branch *, term){
      if(func->def_list.count(br->val)){
        auto val_def = func->def_list.at(br->val);
        TypeCase(binary, ir::insns::Binary *, val_def){
          if(binary->op == BinaryOp::Eq){
            jmp_val = binary->src1;
            auto imm = binary->src2;
            if(func->def_list.count(imm)){
              auto imm_def = func->def_list.at(imm);
              TypeCase(imm, ir::insns::LoadImm *, imm_def){
                switch_map.clear();
                internal_bbs.clear();
                switch_map[imm->imm.iv] = br->true_target;
                BasicBlock * pos = br->false_target;
                while(true){
                  int val;
                  BasicBlock *dst;
                  BasicBlock *next;
                  if(check_bb_br(pos, jmp_val, val, dst, next)){
                    switch_map[val] = dst;
                    internal_bbs.insert(pos);
                    pos = next;
                  }else{
                    break;
                  }
                }
                if(switch_map.size() == BR_CNT){
                  found = true;
                  default_target = pos;
                }
              }
            }
          }
        }
      }
    }
  }
  if(found){
    auto br = dynamic_cast<ir::insns::Branch *>(bb->insns.back().get());
    assert(br);
    auto switch_insn = new ir::insns::Switch(jmp_val, switch_map, default_target);
    bb->pop_back();
    bb->push_back(switch_insn);    
    for(auto each : internal_bbs){
      auto br = dynamic_cast<ir::insns::Branch *>(each->insns.back().get());
      assert(br);
      br->true_target->change_prev(each, bb);
      br->false_target->change_prev(each, bb);
      for(auto prev : each->prev){
        prev->succ.erase(each);
      }
    }
    for(auto each : switch_insn->targets){
      bb->succ.insert(each.second);
    }
    bb->succ.insert(switch_insn->default_target);
    for(auto iter = func->bbs.begin(); iter != func->bbs.end();){
      if(internal_bbs.count(iter->get())){
        for(auto &inst : iter->get()->insns){
          inst->remove_use_def();
        }
        iter = func->bbs.erase(iter);
        continue;
      }
      ++iter;
    }
  }
}

void br2switch(ir::Program *prog) {
  for (auto &[name, func] : prog->functions) {
    br2switch(&func);
  }
  ir_validation(prog);
}

} // namespace mediumend
#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;

void constant_propagation(Function *func) {
  std::unordered_map<Reg, ConstValue> const_map;
  for (auto &bb : func->bbs) {
    for (auto &ins : bb->insns) {
      TypeCase(loadimm, ir::insns::LoadImm *, ins.get()) {
        const_map[loadimm->dst] = loadimm->imm;
      }
    }
  }
  for (auto &bb : func->bbs) {
    for (auto &ins : bb->insns) {
      TypeCase(unary, ir::insns::Unary *, ins.get()) {
        if (const_map.find(unary->src) != const_map.end()) {
          Reg reg = unary->dst;
          unary->removeUseDef(func->cfg->use_list, func->cfg->def_list);
          ConstValue new_val;
          new_val.type = unary->dst.type;
          switch (unary->op) {
            case UnaryOp::Not:
              assert(unary->dst.type == ScalarType::Int);
              new_val.iv = ~const_map[unary->src].iv;
              break;
            case UnaryOp::Sub:
              if(unary->dst.type == ScalarType::Int) {
                new_val.iv = -const_map[unary->src].iv;
              } else {
                new_val.fv = -const_map[unary->src].fv;
              }
              break;
            default:
              assert(false);
          }
          ins.reset(new ir::insns::LoadImm(reg, new_val));
          ins->addUseDef(func->cfg->use_list, func->cfg->def_list);
        }
        continue;
      }
      TypeCase(binary, ir::insns::Binary *, ins.get()) {
        if (const_map.find(binary->src1) != const_map.end() && const_map.find(binary->src2) != const_map.end()) {
          Reg reg = binary->dst;
          binary->removeUseDef(func->cfg->use_list, func->cfg->def_list);
          ConstValue new_val;
          new_val.type = binary->dst.type;
          switch (binary->op) {
            case BinaryOp::Add:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv + const_map[binary->src2].iv;
              } else {
                new_val.fv = const_map[binary->src1].fv + const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Sub:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv - const_map[binary->src2].iv;
              } else {
                new_val.fv = const_map[binary->src1].fv - const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Mul:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv * const_map[binary->src2].iv;
              } else {
                new_val.fv = const_map[binary->src1].fv * const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Div:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv / const_map[binary->src2].iv;
              } else {
                new_val.fv = const_map[binary->src1].fv / const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Mod:
              assert(binary->dst.type == ScalarType::Int);
              new_val.iv = const_map[binary->src1].iv % const_map[binary->src2].iv;
              break;
            // TODO:对于EQ和NEQ的处理需要再考虑一下？
            case BinaryOp::Eq:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv == const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv == const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Neq:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv != const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv != const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Lt:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv < const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv < const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Gt:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv > const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv > const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Leq:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv <= const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv <= const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::Geq:
              if(binary->dst.type == ScalarType::Int) {
                new_val.iv = const_map[binary->src1].iv >= const_map[binary->src2].iv;
              } else {
                new_val.iv = const_map[binary->src1].fv >= const_map[binary->src2].fv;
              }
              break;
            case BinaryOp::And:
              assert(binary->dst.type == ScalarType::Int);
              new_val.iv = const_map[binary->src1].iv & const_map[binary->src2].iv;
              break;
            case BinaryOp::Or:
              assert(binary->dst.type == ScalarType::Int);
              new_val.iv = const_map[binary->src1].iv | const_map[binary->src2].iv;
              break;
            default:
              assert(false);
          }
          auto new_ins = new ir::insns::LoadImm(reg, new_val);
          ins.reset(new_ins);
          ins->addUseDef(func->cfg->use_list, func->cfg->def_list);
          const_map[new_ins->dst] = new_ins->imm;
        }
        continue;
      }
      TypeCase(convey, ir::insns::Convert *, ins.get()){
        if(const_map.find(convey->src) != const_map.end()){
          Reg reg = convey->dst;
          convey->removeUseDef(func->cfg->use_list, func->cfg->def_list);
          ConstValue new_val;
          new_val.type = convey->dst.type;
          if(convey->dst.type == ScalarType::Int){
            if(convey->src.type == ScalarType::Int){
              new_val.iv = const_map[convey->src].iv;
            } else {
              new_val.iv = (int)const_map[convey->src].fv;
            }
          } else {
            if(convey->src.type == ScalarType::Int){
              new_val.fv = (float)const_map[convey->src].iv;
            } else {
              new_val.fv = const_map[convey->src].fv;
            }
          }
          auto new_ins = new ir::insns::LoadImm(reg, new_val);
          ins.reset(new_ins);
          ins->addUseDef(func->cfg->use_list, func->cfg->def_list);
          const_map[new_ins->dst] = new_ins->imm;
        }
        continue;
      }
      TypeCase(br, ir::insns::Branch *, ins.get()) {
        if (const_map.find(br->val) != const_map.end()) {
          br->removeUseDef(func->cfg->use_list, func->cfg->def_list);
          ir::BasicBlock *target;
          if(const_map[br->val].iv) {
            target = br->true_target;
          } else {
            target = br->false_target;
          }
          auto new_ins = new ir::insns::Jump(target);
          ins.reset(new_ins);
        }
        continue;
      }
    }
  }
  func->cfg->build();
  func->cfg->remove_unreachable_bb();
  for(auto &bb : func->bbs){
    for(auto &inst : bb->insns){
      if(auto phi = dynamic_cast<ir::insns::Phi *>(inst.get())){
        for(auto &in : phi->incoming){
          if(func->cfg->prev.find(in.first) == func->cfg->prev.end()){
            phi->incoming.erase(in.first);
          }
        }
      }
    }
  }
}

} // namespace mediumend

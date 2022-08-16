#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;
using std::unordered_map;

void algebra_simpilifacation(Function *func) {
  for(auto &bb : func->bbs){
    unordered_map<Reg, unordered_map<Reg, int>> reg_map;
    for(auto iter = bb->insns.begin(); iter != bb->insns.end();) {
      TypeCase(binary, ir::insns::Binary *, iter->get()){
        unordered_map<Reg, int> single_map;
        if((binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) || binary->dst.type != Int){
          ++iter;
          continue;
        }
        if(reg_map.count(binary->src1)){
          single_map = reg_map.at(binary->src1);
        } else {
          single_map[binary->src1] = 1;
        }
        if(reg_map.count(binary->src2)){
          for(auto each : reg_map.at(binary->src2)){
            if(binary->op == BinaryOp::Add){
            single_map[each.first] += each.second;
            }
            if(binary->op == BinaryOp::Sub){
              single_map[each.first] -= each.second;
            } 
          }
        } else {
          if(binary->op == BinaryOp::Add){
            single_map[binary->src2] += 1;
          }
          if(binary->op == BinaryOp::Sub){
            single_map[binary->src2] -= 1;
          } 
        }
        for(auto map_iter = single_map.begin(); map_iter != single_map.end();){
          if(map_iter->second == 0){
            map_iter = single_map.erase(map_iter);
          } else {
            ++map_iter;
          }
        }
        if(single_map.size() == 0){
          binary->remove_use_def();
          auto new_inst = new ir::insns::LoadImm(binary->dst, ConstValue(0));
          new_inst->bb = bb.get();
          new_inst->add_use_def();
          iter->reset(new_inst);
          reg_map[new_inst->dst] = single_map;
        } else if(single_map.size() == 1){
          auto reg_pair = single_map.begin();
          if(reg_pair->second == 1){
            copy_propagation(func->use_list, binary->dst, reg_pair->first);
            iter->get()->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          } else if(reg_pair->second == -1){
            binary->remove_use_def();
            auto new_inst = new ir::insns::Unary(binary->dst, UnaryOp::Sub, reg_pair->first);
            new_inst->bb = bb.get();
            new_inst->add_use_def();
            iter->reset(new_inst);
            reg_map[new_inst->dst] = single_map;
          }
        } else if(single_map.size() == 2){
          bool removable = true;
          for(auto &each : single_map){
            if(each.second != 1 && each.second != -1){
              removable = false;
            }
          }
          if(removable){
            binary->remove_use_def();
            auto map_iter = single_map.begin();
            auto src1 = *map_iter;
            map_iter++;
            auto src2 = *map_iter;
            bool add_not = false;
            if(src1.second == src2.second){
              binary->op = BinaryOp::Add;
              binary->src1 = src1.first;
              binary->src2 = src2.first;
              if(src1.second == -1){
                add_not = true;
              }
            } else {
              binary->op = BinaryOp::Sub;
              if(src1.second == 1){
                binary->src1 = src1.first;
                binary->src2 = src2.first;
              } else {
                binary->src1 = src2.first;
                binary->src2 = src1.first;
              }
            }
            binary->add_use_def();
            ++iter;
            if(add_not){
              auto reg = func->new_reg(ScalarType::Int);
              ir::Instruction* inst = new ir::insns::Unary(reg, UnaryOp::Sub, binary->dst);
              copy_propagation(func->use_list, binary->dst, reg);
              inst->bb = bb.get();
              inst->add_use_def();
              bb->insns.emplace(iter, inst);
              reg_map[reg] = single_map;
            } else {
              reg_map[binary->dst] = single_map;
            }
            continue;
          }
        }
      } else TypeCase(unary, ir::insns::Unary *, iter->get()){
        if(unary->op != UnaryOp::Sub || unary->dst.type != Int){
          ++iter;
          continue;
        }
        unordered_map<Reg, int> single_map;
        if(reg_map.count(unary->src)){
          single_map = reg_map.at(unary->src);
        } else {
          single_map[unary->src] = 1;
        }
        for(auto &each : single_map){
          each.second = -each.second;
        }
        reg_map[unary->dst] = single_map;
        if(single_map.size() == 1){
          auto reg_pair = single_map.begin();
          if(reg_pair->second == 1){
            copy_propagation(func->use_list, unary->dst, reg_pair->first);
            iter->get()->remove_use_def();
            iter = bb->insns.erase(iter);
            continue;
          }
        }
      }
      iter++;
    }
  }
}

void algebra_simpilifacation(ir::Program *prog) {
  for (auto &[_, func] : prog->functions) {
    algebra_simpilifacation(&func);
  }
}

} // namespace mediumend
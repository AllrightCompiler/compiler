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
        for(auto iter = single_map.begin(); iter != single_map.end();){
          if(iter->second == 0){
            iter = single_map.erase(iter);
          } else {
            ++iter;
          }
        }
        reg_map[binary->dst] = single_map;
        if(single_map.size() == 0){
          binary->remove_use_def();
          auto new_inst = new ir::insns::LoadImm(binary->dst, ConstValue(0));
          new_inst->bb = bb.get();
          new_inst->add_use_def();
          iter->reset(new_inst);
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
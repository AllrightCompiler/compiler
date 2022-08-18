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
        if((binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) || binary->dst.type != Int){
          ++iter;
          continue;
        }
        unordered_map<Reg, int> single_map;
        if(reg_map.count(binary->src1)){
          single_map = reg_map.at(binary->src1);
        } else {
          if(!func->has_param(binary->src1)){
            TypeCase(loadimm, ir::insns::LoadImm *, func->def_list.at(binary->src1)){
              if(loadimm->imm.type == ScalarType::Int && loadimm->imm.iv == 0){
                single_map = {};
              } else {
                single_map[binary->src1] = 1;
              }
            } else {
              single_map[binary->src1] = 1;
            }
          } else {
            single_map[binary->src1] = 1;
          }
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
          bool add_zero = false;
          if(!func->has_param(binary->src1)){
            TypeCase(loadimm, ir::insns::LoadImm *, func->def_list.at(binary->src1)){
              if(loadimm->imm.type == ScalarType::Int && loadimm->imm.iv == 0){
                add_zero = true;
              }
            }
          }
          if(!add_zero){
            if(binary->op == BinaryOp::Add){
              single_map[binary->src2] += 1;
            }
            if(binary->op == BinaryOp::Sub){
              single_map[binary->src2] -= 1;
            }
          }
        }
        for(auto map_iter = single_map.begin(); map_iter != single_map.end();){
          if(map_iter->second == 0){
            map_iter = single_map.erase(map_iter);
          } else {
            ++map_iter;
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
        } else if(single_map.size() == 2){
          bool removable = true;
          for(auto &each : single_map){
            if(each.second != 1 && each.second != -1){
              removable = false;
            }
          }
          auto map_iter = single_map.begin();
          auto src1 = *map_iter;
          map_iter++;
          auto src2 = *map_iter;
          if(src1.second == -1 && src2.second == -1){
            removable = false;
          }
          if(removable){
            binary->remove_use_def();
            bool add_not = false;
            if(src1.second == src2.second){
              binary->op = BinaryOp::Add;
              binary->src1 = src1.first;
              binary->src2 = src2.first;
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
    for(auto iter = bb->insns.begin(); iter != bb->insns.end();++iter) {
      TypeCase(binary, ir::insns::Binary *, iter->get()){
        if((binary->op != BinaryOp::Add && binary->op != BinaryOp::Sub) || binary->dst.type != Int){
          continue;
        }
        auto &single_map = reg_map.at(binary->dst);
        for(auto s_iter = std::next(iter); s_iter != bb->insns.end();s_iter++) {
          TypeCase(sbinary, ir::insns::Binary *, s_iter->get()){
            if((sbinary->op != BinaryOp::Add && sbinary->op != BinaryOp::Sub) || sbinary->dst.type != Int){
              continue;
            }
            bool change_able = true;
            auto &s_map = reg_map.at(sbinary->dst);
            for(auto &each : single_map){
              if(!s_map.count(each.first) || s_map.at(each.first) != each.second){
                change_able = false;
                break;
              }
            }
            if(!change_able){
              continue;
            }
            for(auto &each : single_map){
              s_map.erase(each.first);
            }
            s_map[binary->dst] = 1;
          }
        }
      }
    }
    for(auto &each : reg_map){
      if(each.second.size() == 2){
        auto m_iter = each.second.begin();
        auto src1 = *m_iter;
        m_iter++;
        auto src2 = *m_iter;
        if(src1.second == src2.second && src1.second == -1){
          continue;
        }
        auto binary = dynamic_cast<ir::insns::Binary*>(func->def_list.at(each.first));
        if(!binary){
          continue;
        }
        bool removable = true;
        if(src1.second == -1 && src2.second == -1){
          removable = false;
        }
        if(removable){
          binary->remove_use_def();
          bool add_not = false;
          if(src1.second == src2.second){
            binary->op = BinaryOp::Add;
            binary->src1 = src1.first;
            binary->src2 = src2.first;
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
        }
      }
    }
  }
}

void algebra_simpilifacation(ir::Program *prog) {
  for (auto &[_, func] : prog->functions) {
    algebra_simpilifacation(&func);
  }
}

} // namespace mediumend
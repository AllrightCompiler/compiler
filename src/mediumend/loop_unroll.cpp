#include "common/ir.hpp"
#include "mediumend/cfg.hpp"
#include "mediumend/optmizer.hpp"

#include <cassert>

namespace mediumend {

using ir::Function;
using ir::Reg;
using std::unordered_map;

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

void loop_unroll(Function * func){
  auto const_map = find_const_int(func);
  func->cfg->compute_dom();
  
}

}
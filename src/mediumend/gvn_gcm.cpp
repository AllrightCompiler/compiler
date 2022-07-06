#include "common/ir.hpp"
#include "mediumend/cfg.hpp"

#include <iostream>

namespace mediumend {

using ir::Function;
using ir::BasicBlock;
using ir::Reg;
using ir::Instruction;
using std::list;

// get Value Number of inst, insert when not found
Reg vn_get(unordered_map<Instruction *, Reg> &hashTable, Instruction *inst) {
  if (hashTable.find(inst) != hashTable.end()) return hashTable[inst];
  TypeCase(phi, ir::insns::Phi *, inst) {
    hashTable[inst] = phi->dst;
    for (auto pair : hashTable) {
      auto i = pair.first;
      TypeCase(phi_i, ir::insns::Phi *, i) {
        bool same = true;
        Reg first = (*phi_i->incoming.begin()).second;
        for (auto income : phi_i->incoming) {
          same &= (first == income.second);
        }
        if (same) {
          hashTable[inst] = phi_i->dst;
        }
      }
    }
  } else TypeCase(loadimm, ir::insns::LoadImm *, inst) {
    hashTable[inst] = loadimm->dst;
    for (auto pair : hashTable) {
      auto i = pair.first;
      TypeCase(loadimm_i, ir::insns::LoadImm *, i) {
        if (loadimm_i->imm == loadimm->imm) {
          hashTable[inst] = loadimm_i->dst;
        }
      }
    }
  } else TypeCase(other, ir::insns::Output *, inst) {
    hashTable[inst] = other->dst;
  } else assert(false);
  return hashTable[inst];
}

// reg_dst = reg_src
// change all reg_dst -> reg_src
void copy_propagation(unordered_map<Reg, list<Instruction *>> &use_list, Reg dst, Reg src) {
  while (use_list.size() > 0) {
    auto inst = use_list[dst].front();
    inst->change_use(use_list, dst, src);
  }
}

void gvn_gcm(Function *f) {
  // Global Value Numbering

  f->cfg->build();
  f->cfg->compute_dom();
  // cfg dom, domby, idom, domlevel set
  f->cfg->compute_rpo();
  // rpo set

  // use reg as value number
  // reg -> inst : f->def_list[reg]
  unordered_map<Instruction *, Reg> hashTable;

  for (auto bb : f->cfg->rpo) {
    for (auto &insn : bb->insns) {
      TypeCase(phi, ir::insns::Phi *, insn.get()) {
        unordered_map <Reg, Reg> regsToChange;
        for (auto income : phi->incoming) {
          Reg new_reg = vn_get(hashTable, f->def_list[income.second]);
          if (!(income.second == new_reg)) {
            regsToChange[income.second] = new_reg;
          }
        }
        for (auto reg_pair : regsToChange) {
          phi->change_use(f->use_list, reg_pair.first, reg_pair.second);
        }
        // check meaningless
        bool meaningless = true;
        Reg first = (*(phi->incoming.begin())).second;
        for (auto income : phi->incoming) {
          meaningless &= (first == income.second);
        }
        if (meaningless) { // all incoming same
          copy_propagation(f->use_list, phi->dst, first);
        }
      }
      TypeCase(binary, ir::insns::Binary *, insn.get()) {
        Reg new_reg1 = vn_get(hashTable, f->def_list[binary->src1]);
        Reg new_reg2 = vn_get(hashTable, f->def_list[binary->src2]);
        if (!(binary->src1 == new_reg1)) binary->change_use(f->use_list, binary->src1, new_reg1);
        if (!(binary->src2 == new_reg2)) binary->change_use(f->use_list, binary->src2, new_reg2);

      }
      // TODO: more inst types
    }
  }
}

void gvn_gcm(ir::Program *prog) {
  for(auto &func : prog->functions){
    // std::cout << "func: " << func.first << std::endl;
    gvn_gcm(&func.second);
  }
}

}


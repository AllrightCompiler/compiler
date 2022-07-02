#pragma once

#include "common/ir.hpp"

namespace mediumend {

using ir::Function;
using ir::BasicBlock;
using ir::Instruction;
using ir::Reg;
using std::unordered_map;
using std::unordered_set;
using std::list;

class CFG{
private:
    Function *func;
public:
    CFG(){}
    CFG(ir::Function *func);
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> prev, succ;
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> dom, domby;
    unordered_map<BasicBlock *, BasicBlock *> idom;
    unordered_map<BasicBlock *, int> domlevel;
    unordered_set<BasicBlock *> visit;
    unordered_map<Reg, list<Instruction *>> use_list;
    unordered_map<Reg, Instruction *> def_list;
    inline void clear_visit(){
        visit.clear();
    }
    inline void remove_bb_in_cfg(BasicBlock *bb){
        prev.erase(bb);
        succ.erase(bb);
        dom.erase(bb);
        domby.erase(bb);
        idom.erase(bb);
        domlevel.erase(bb);
        visit.erase(bb);
    }
    
    void remove_unreachable_bb();

    void compute_dom();

    void compute_use_def_list();

    void compute_dom_level(BasicBlock *bb, int dom_level);

    void remove_unused_reg();

    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_df();
};
    
} // namespace mediumend

#pragma once

#include "common/ir.hpp"

namespace mediumend {

using ir::BasicBlock;
using std::unordered_map;
using std::unordered_set;

void compute_use_def_list(ir::Function *);

void mark_global_addr_reg(ir::Function *);

class CFG{
private:
    ir::Function *func;
public:
    CFG(){}
    CFG(ir::Function *func);
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> prev, succ;
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> dom, domby;
    unordered_map<BasicBlock *, BasicBlock *> idom;
    unordered_map<BasicBlock *, int> domlevel;
    unordered_set<BasicBlock *> visit;
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

    void build();
    
    void remove_unreachable_bb();

    void compute_dom();

    void compute_dom_level(BasicBlock *bb, int dom_level);

    bool eliminate_useless_cf_one_pass();

    void clean_useless_cf();

    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_df();
};
    
} // namespace mediumend

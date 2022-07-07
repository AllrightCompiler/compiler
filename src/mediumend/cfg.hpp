#pragma once

#include "common/ir.hpp"

namespace mediumend {

using ir::BasicBlock;
using std::unordered_map;
using std::unordered_set;
using std::vector;

void compute_use_def_list(ir::Program *);

void mark_global_addr_reg(ir::Program *);

class CFG{
private:
    ir::Function *func;
public:
    CFG(){}
    CFG(ir::Function *func);
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> prev, succ;
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> dom, domby;
    unordered_map<BasicBlock *, BasicBlock *> idom;
    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> rdom, rdomby;
    unordered_map<BasicBlock *, BasicBlock *> ridom;
    unordered_map<BasicBlock *, int> domlevel;
    vector<BasicBlock *> rpo; // Reverse PostOrder
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

    void compute_rdom();

    void compute_rpo();

    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_df();

    unordered_map<BasicBlock *, unordered_set<BasicBlock *>> compute_rdf();

private:
    void compute_dom_level(BasicBlock *bb, int dom_level);
    
    void rpo_dfs(BasicBlock *bb);
};
    
} // namespace mediumend

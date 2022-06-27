#pragma once

#include "common/ir.hpp"

namespace mediumend {

class CFG{
public:
    CFG(ir::Function *func);
    std::unordered_map<ir::BasicBlock *, std::unordered_set<ir::BasicBlock *>> pred, succ;
    std::unordered_map<ir::BasicBlock *, std::unordered_set<ir::BasicBlock *>> dom, domby;
    std::unordered_map<ir::BasicBlock *, ir::BasicBlock *> idom;
};
    
} // namespace mediumend

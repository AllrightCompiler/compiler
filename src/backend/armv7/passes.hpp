#pragma once

#include "backend/armv7/program.hpp"

namespace armv7 {

void backend_passes(Program &p);

// 将可用的常量折叠进指令并进行指令替换
void fold_constants(Function &f);

// 移除无用指令
void remove_unused(Function &f);

// phi解构后的局部常量传播
void propagate_constants(Function &f);

// 移除无实际效果的指令，如mov r0, r0
void remove_nop(Function &f);

// 清理控制流
void sanitize_cfg(Function &f);

} // namespace armv7

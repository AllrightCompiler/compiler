#pragma once

#include "backend/armv7/program.hpp"

namespace armv7 {

void backend_passes(Program &p);

// 将可用的常量折叠进指令并进行指令替换
void fold_constants(Function &f);

// 移除无用指令
void remove_unused(Function &f);

} // namespace armv7
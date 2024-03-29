#pragma once

#include "backend/armv7/program.hpp"

namespace armv7 {

void merge_shift_with_binary_op(Function &func);
void merge_add_with_load_or_store(Function &func);
void merge_mul_with_add_or_sub(Function &func);

} // namespace armv7

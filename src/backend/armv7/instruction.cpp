#include "backend/armv7/instruction.hpp"
#include "common/common.hpp"

namespace armv7 {

ExCond logical_not(ExCond cond) {
  switch (cond) {
  case ExCond::Eq:
    return ExCond::Ne;
  case ExCond::Ne:
    return ExCond::Eq;
  case ExCond::Ge:
    return ExCond::Lt;
  case ExCond::Gt:
    return ExCond::Le;
  case ExCond::Le:
    return ExCond::Gt;
  case ExCond::Lt:
    return ExCond::Ge;
  default:
    __builtin_unreachable();
  }
}

ExCond from(BinaryOp op) {
  switch (op) {
  case BinaryOp::Eq:
    return ExCond::Eq;
  case BinaryOp::Neq:
    return ExCond::Ne;
  case BinaryOp::Geq:
    return ExCond::Ge;
  case BinaryOp::Gt:
    return ExCond::Gt;
  case BinaryOp::Leq:
    return ExCond::Le;
  case BinaryOp::Lt:
    return ExCond::Lt;
  default:
    return ExCond::Always;
  }
}

RType::Op RType::from(BinaryOp op) {
  switch (op) {
  case BinaryOp::Add:
    return Add;
  case BinaryOp::Sub:
    return Sub;
  case BinaryOp::Mul:
    return Mul;
  case BinaryOp::Div:
    return Div;
  default:
    __builtin_unreachable();
  }
}

} // namespace armv7

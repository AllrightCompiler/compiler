#include "backend/armv7/instruction.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/program.hpp"

#include "common/common.hpp"
#include "common/config.hpp"

#include <string>
#include <cstring>

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

using std::ostream;

void next_instruction(ostream &os) {
  os << '\n';
  for (int i = 0; i < ASM_INDENT; ++i)
    os << ' ';
}

ostream &operator<<(ostream &os, const Reg &r) {
  if (r.type != Fp) {
    if (r.id >= 0) {
      if (r.id < NR_GPRS)
        os << GPR_NAMES[r.id];
      else
        os << "<bad reg " << r.id << ">";
    } else
      os << "t" + std::to_string(-r.id);
  } else {
    if (r.id >= 0)
      os << "s" + std::to_string(r.id);
    else
      os << "f" + std::to_string(-r.id);
  }
  return os;
}

constexpr const char *COND_NAME[] = {
    [int(ExCond::Always)] = "", [int(ExCond::Eq)] = "eq",
    [int(ExCond::Ne)] = "ne",   [int(ExCond::Ge)] = "ge",
    [int(ExCond::Gt)] = "gt",   [int(ExCond::Le)] = "le",
    [int(ExCond::Lt)] = "lt",
};

ostream &operator<<(ostream &os, ExCond c) { return os << COND_NAME[int(c)]; }

constexpr const char *SHIFT_NAME[] = {
    [LSL] = "LSL",
    [LSR] = "LSR",
    [ASR] = "ASR",
    [ROR] = "ROR",
};

ostream &operator<<(ostream &os, const ShiftType &t) {
  return os << SHIFT_NAME[t];
}

ostream &operator<<(ostream &os, const RegImmShift &s) {
  os << s.r;
  if (s.s != 0)
    os << ", " << s.type << " #" << s.s;
  return os;
}

ostream &operator<<(ostream &os, const RegRegShift &s) {
  return os << s.r1 << ", " << s.type << ' ' << s.r2;
}

ostream &operator<<(ostream &os, const Operand2 &opd) {
  if (opd.is_imm8m())
    os << "#" << opd.get<int>();
  else if (opd.is_imm_shift())
    os << opd.get<RegImmShift>();
  else
    os << opd.get<RegRegShift>();
  return os;
}

int get_padding_length(const char *op, ExCond cond, bool is_float, bool is_ldst) {
  int base_len = std::strlen(op);
  int cond_len = cond == ExCond::Always ? 0 : 2;
  base_len += cond_len;

  if (is_float) {
    base_len += 4;
    if (!is_ldst)
      base_len++;
  }
  return std::max(8 - base_len, 1);
}

ostream &Instruction::write_op(std::ostream &os, const char *op, bool is_float,
                               bool is_ldst) const {
  if (is_float) {
    os << 'v' << op << cond << '.';
    if (!is_ldst)
      os << 'f';
    os << "32 ";
  } else {
    os << op << cond << ' ';
  }

  int len = get_padding_length(op, cond, is_float, is_ldst);
  while (len--)
    os << ' ';
  return os;
}

void RType::emit(std::ostream &os) const {
  constexpr const char *OP_NAMES[] = {
      [Add] = "add",
      [Sub] = "sub",
      [Mul] = "mul",
      [Div] = "sdiv",
  };
  write_op(os, OP_NAMES[op], dst.is_float()) << dst << ", " << s1 << ", " << s2;
}

void IType::emit(std::ostream &os) const {
  constexpr const char *OP_NAMES[] = {
      [Add] = "add", [Sub] = "sub", [RevSub] = "rsb"};
  write_op(os, OP_NAMES[op]) << dst << ", " << s1 << ", #" << imm;
}

void FullRType::emit(std::ostream &os) const {
  constexpr const char *OP_NAMES[] = {
      [Add] = "add", [Sub] = "sub", [RevSub] = "rsb"};
  write_op(os, OP_NAMES[op]) << dst << ", " << s1 << ", " << s2;
}

void Move::emit(std::ostream &os) const {
  auto op = flip ? "mvn" : "mov";
  write_op(os, op) << dst << ", " << src;
}

void MovW::emit(std::ostream &os) const {
  write_op(os, "movw") << dst << ", #" << imm;
}

void MovT::emit(std::ostream &os) const {
  write_op(os, "movt") << dst << ", #" << imm;
}

void LoadAddr::emit(std::ostream &os) const {
  write_op(os, "movw") << dst << ", #:lower16:" << symbol;
  next_instruction(os);
  write_op(os, "movt") << dst << ", #:upper16:" << symbol;
}

void Compare::emit(std::ostream &os) const {
  // vcmpe.f32
  // vmrs APSR_nzcv, FPSCR
  auto op = neg ? "cmn" : "cmp";
  write_op(os, op) << s1 << ", " << s2;
}

void Load::emit(std::ostream &os) const {
  write_op(os, "ldr", dst.is_float(), true);
  os << dst << ", [" << base;
  if (offset)
    os << ", #" << offset;
  os << ']';
}

void Store::emit(std::ostream &os) const {
  write_op(os, "str", src.is_float(), true);
  os << src << ", [" << base;
  if (offset)
    os << ", #" << offset;
  os << ']';
}

void FusedMul::emit(std::ostream &os) const {
  auto op = sub ? "mls" : "mla";
  write_op(os, op, dst.is_float())
      << dst << ", " << s1 << ", " << s2 << ", " << s3;
}

void Branch::emit(std::ostream &os) const {
  write_op(os, "b") << target->label;
}

void RegBranch::emit(std::ostream &os) const {
  auto op = type == Cbz ? "cbz" : "cbnz";
  write_op(os, op) << src << ", " << target->label;
}

void LoadStack::emit(std::ostream &os) const {
  os << "*load-stack " << dst << ", obj[" << base << "]+" << offset;
}

void StoreStack::emit(std::ostream &os) const {
  os << "*store-stack " << src << ", obj[" << base << "]+" << offset;
}

void LoadStackAddr::emit(std::ostream &os) const {
  os << "*local-addr " << dst << ", obj[" << base << "]+" << offset;
}

void AdjustSp::emit(std::ostream &os) const {
  os << "*sp-adjust " << offset;
}

void Push::emit(std::ostream &os) const {
  write_op(os, "push") << '{';
  int n = srcs.size();
  for (int i = 0; i < n; ++i) {
    if (i != 0)
      os << ", ";
    os << srcs[i];
  }
  os << '}';
}

void Pop::emit(std::ostream &os) const {
  write_op(os, "pop") << '{';
  int n = dsts.size();
  for (int i = 0; i < n; ++i) {
    if (i != 0)
      os << ", ";
    os << dsts[i];
  }
  os << '}';
}

void Call::emit(std::ostream &os) const {
  write_op(os, "bl") << func;
}

void Return::emit(std::ostream &os) const {
  write_op(os, "bx") << "lr";
}

void CountLeadingZero::emit(std::ostream &os) const {
  write_op(os, "clz") << dst << ", " << src;
}

void PseudoCompare::emit(std::ostream &os) const {
  os << "*compare-" << cond << ' ' << dst << ' ';
  cmp->emit(os);
}

} // namespace armv7

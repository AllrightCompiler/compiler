#pragma once

#include "common/ir.hpp"

#include <variant>

namespace armv7 {

// 此结构几乎与ir::Reg一致
// 负id表示虚拟（伪）寄存器
struct Reg {
  int id;
  ScalarType type;

  Reg() {}
  Reg(ScalarType type, int id) : type{type}, id{id} {}

  bool is_pseudo() const { return id < 0; }
  bool is_float() const { return type == Float; }

  bool operator<(const Reg &rhs) const {
    if (type != rhs.type)
      return type < rhs.type;
    return id < rhs.id;
  }

  static Reg from(ir::Reg ir_reg) { return Reg{ir_reg.type, -ir_reg.id}; }
};

// 指令执行条件码
enum class ExCond {
  Always,
  Eq,
  Ne,
  Ge,
  Gt,
  Le,
  Lt,
};

ExCond logical_not(ExCond cond);
ExCond from(BinaryOp op);

// Operand2的移位类型
// 注: RRX未使用
enum ShiftType {
  LSL, // Logical Shift Left
  LSR, // Logical Shift Right
  ASR, // Arithmetic Shift Right
  ROR, // ROtate Right
};

struct RegImmShift {
  ShiftType type;
  Reg r;
  int s;

  RegImmShift(Reg r) : type{LSL}, r{r}, s{0} {}
  RegImmShift(ShiftType type, Reg r, int s) : type{type}, r{r}, s{s} {}
};

struct RegRegShift {
  ShiftType type;
  Reg r1, r2;

  RegRegShift(ShiftType type, Reg r1, Reg r2) : type{type}, r1{r1}, r2{r2} {}
};

// Operand2: imm8m / Rm shift imm / Rm shift Rn
// using Operand2 = std::variant<int, RegImmShift, RegRegShift>;

struct Operand2 {
  std::variant<int, RegImmShift, RegRegShift> opd;

  bool is_imm8m() const { return opd.index() == 0; }
  bool is_imm_shift() const { return opd.index() == 1; }
  bool is_reg_shift() const { return opd.index() == 2; }

  template <typename T> auto get() const { return std::get<T>(opd); }

  static Operand2 from(int imm) { return Operand2{.opd = imm}; }
  static Operand2 from(Reg r) { return Operand2{.opd = RegImmShift{r}}; }
  static Operand2 from(ShiftType type, Reg r, int s) {
    return Operand2{.opd = RegImmShift{type, r, s}};
  }
};

struct Instruction {
  ExCond cond;

  Instruction() : cond{ExCond::Always} {}
  virtual ~Instruction() = default;
};

// 形如 op Rd, Rm, Rn 的指令
struct RType : Instruction {
  enum Op { Add, Sub, Mul, Div } op;
  Reg dst, s1, s2;

  RType(Op op, Reg dst, Reg s1, Reg s2) : op{op}, dst{dst}, s1{s1}, s2{s2} {}

  static Op from(BinaryOp);
};

// 形如 op Rd, Rm, imm 的指令
// 大部分的imm都是取自Operand2的imm8m
// 但Thumb-2的add和sub支持12位无符号立即数
struct IType : Instruction {
  enum Op { Add, Sub, RevSub } op;
  Reg dst, s1;
  int imm;

  IType(Op op, Reg dst, Reg s1, int imm) : op{op}, dst{dst}, s1{s1}, imm{imm} {}
};

// 具有 op Rd, R1, Operand2 形式的指令
struct FullRType : Instruction {
  enum Op { Add, Sub, RevSub } op;
  Reg dst, s1;
  Operand2 s2;

  FullRType(Op op, Reg dst, Reg s1, Operand2 s2)
      : op{op}, dst{dst}, s1{s1}, s2{std::move(s2)} {}
};

// 完整形式的MOV: MOV Rd, Operand2
// 还包括MVN Rd, Operand2, 注意这个东西是将Operand2按位取反
// e.g. mvn r0, #0 => r0 = -1
struct Move : Instruction {
  Reg dst;
  Operand2 src;
  bool flip;

  Move(Reg dst, Operand2 src) : dst{dst}, src{std::move(src)}, flip{false} {}
  Move(Reg dst, Operand2 src, bool flip)
      : dst{dst}, src{std::move(src)}, flip{flip} {}
};

// Thumb-2 movw: 加载低16位，清零高16位
struct MovW : Instruction {
  Reg dst;
  int imm;

  MovW(Reg dst, int imm) : dst{dst}, imm{imm} {}
};

// Thumb-2 movt: 加载高16位，低16为不变
struct MovT : Instruction {
  Reg dst;
  int imm;

  MovT(Reg dst, int imm) : dst{dst}, imm{imm} {}
};

// 加载全局变量的地址
// 可能会被翻译为movw + movt
// 足够近也可能是adr或ldr伪指令
struct LoadAddr : Instruction {
  Reg dst;
  std::string symbol;

  LoadAddr(Reg dst, std::string sym) : dst{dst}, symbol{std::move(sym)} {}
};

// CMP Rn, Operand2: 更新Rn - Operand2的CPSR标记
// CMN Rn, Operand2: 更新Rn + Operand2的CPSR标记
struct Compare : Instruction {
  Reg s1;
  Operand2 s2;
  bool neg;

  Compare(Reg s1, Operand2 s2) : s1{s1}, s2{std::move(s2)}, neg{false} {}
  Compare(Reg s1, Operand2 s2, bool neg)
      : s1{s1}, s2{std::move(s2)}, neg{neg} {}
};

// 简单load: base_reg + offset_imm 寻址
struct Load : Instruction {
  Reg dst, base;
  int offset;

  Load(Reg dst, Reg base, int offset) : dst{dst}, base{base}, offset{offset} {}
};

// 简单store: base_reg + offset_imm 寻址
struct Store : Instruction {
  Reg src, base;
  int offset;

  Store(Reg src, Reg base, int offset) : src{src}, base{base}, offset{offset} {}
};

// MLA Rd, Rm, Rs, Rn: Rd := Rn + Rm * Rs
// MLS Rd, Rm, Rs, Rn: Rd := Rn - Rm * Rs
struct FusedMul : Instruction {
  Reg dst, s1, s2, s3;
  bool sub;

  FusedMul(Reg dst, Reg s1, Reg s2, Reg s3)
      : dst{dst}, s1{s1}, s2{s2}, s3{s3}, sub{false} {}
  FusedMul(Reg dst, Reg s1, Reg s2, Reg s3, bool sub)
      : dst{dst}, s1{s1}, s2{s2}, s3{s3}, sub{sub} {}
};

struct BasicBlock;

// B.cond
struct Branch : Instruction {
  BasicBlock *target;

  Branch(BasicBlock *target) : target{target} {}
};

// CBZ/CBNZ
struct RegBranch : Instruction {
  enum Type { Cbnz, Cbz } type;
  BasicBlock *target;
  Reg src;

  RegBranch(Type type, Reg src, BasicBlock *target)
      : type{type}, target{target}, src{src} {}
};

// 下面是与栈指针sp相关的指令
// 为了拓展性，StackObject *与int offset配对，offset表示相对于stack object的偏移
struct StackObject;
struct SpRelative : Instruction {};

// dst = base + offset
struct LoadStackAddr : SpRelative {
  Reg dst;
  StackObject *base;
  int offset;

  LoadStackAddr(Reg dst, StackObject *base, int offset)
      : dst{dst}, base{base}, offset{offset} {}
};

// dst = [base + offset]
struct LoadStack : SpRelative {
  Reg dst;
  StackObject *base;
  int offset;

  LoadStack(Reg dst, StackObject *base, int offset)
      : dst{dst}, base{base}, offset{offset} {}
};

struct StoreStack : SpRelative {
  Reg src;
  StackObject *base;
  int offset;

  StoreStack(Reg src, StackObject *base, int offset)
      : src{src}, base{base}, offset{offset} {}
};

struct AdjustSp : SpRelative {
  int offset;

  AdjustSp(int offset) : offset{offset} {}
};

struct Push : SpRelative {
  std::vector<Reg> srcs;

  Push(std::vector<Reg> srcs) : srcs{std::move(srcs)} {}
};

struct Call : Instruction {
  std::string func;

  Call(std::string func) : func{std::move(func)} {}
};

struct Return : Instruction {};

struct CountLeadingZero : Instruction {
  Reg dst, src;

  CountLeadingZero(Reg dst, Reg src) : dst{dst}, src{src} {}
};

} // namespace armv7

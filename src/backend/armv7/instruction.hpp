#pragma once

#include "backend/armv7/arch.hpp"

#include "common/ir.hpp"

#include <set>
#include <variant>

namespace armv7 {

enum RegType {
  General,
  Fp,
};

inline constexpr RegType ir_to_machine_reg_type(int t) {
  return t == Float ? Fp : General;
}

inline RegType machine_reg_type(const Type &t) {
  return t.is_array() ? General : ir_to_machine_reg_type(t.base_type);
}

// 此结构几乎与ir::Reg一致
// 负id表示虚拟（伪）寄存器
struct Reg {
  using Type = RegType;

  int id;
  RegType type;

  Reg() {}
  Reg(RegType type, int id) : type{type}, id{id} {}

  bool is_virt() const { return id < 0; }
  bool is_pseudo() const { return id < 0; }
  bool is_float() const { return type == Fp; }

  bool operator==(const Reg &rhs) const {
    return type == rhs.type && id == rhs.id;
  }
  bool operator!=(const Reg &rhs) const { return !(*this == rhs); }
  bool operator<(const Reg &rhs) const {
    if (type != rhs.type)
      return type < rhs.type;
    return id < rhs.id;
  }

  static Reg from(ir::Reg ir_reg) {
    return Reg{ir_to_machine_reg_type(ir_reg.type), -ir_reg.id};
  }
  static Reg from(int t, int id) {
    return Reg{ir_to_machine_reg_type(t), id};
  }
};

std::ostream &operator<<(std::ostream &os, const Reg &r);

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
  std::variant<int, RegImmShift, RegRegShift, float> opd;

  bool is_imm8m() const { return opd.index() == 0; }
  bool is_imm_shift() const { return opd.index() == 1; }
  bool is_reg_shift() const { return opd.index() == 2; }
  bool is_fpimm() const { return opd.index() == 3; }

  template <typename T> auto &get() const { return std::get<T>(opd); }

  std::set<Reg> get_use() const {
    if (is_imm_shift())
      return {std::get<RegImmShift>(opd).r};
    if (is_reg_shift()) {
      auto &rr_shift = std::get<RegRegShift>(opd);
      return {rr_shift.r1, rr_shift.r2};
    }
    return {};
  }

  std::vector<Reg *> get_reg_ptrs() {
    if (is_imm_shift()) {
      auto &shift = std::get<RegImmShift>(opd);
      return {&(shift.r)};
    }
    if (is_reg_shift()) {
      auto &shift = std::get<RegRegShift>(opd);
      return {&(shift.r1), &(shift.r2)};
    }
    return {};
  }

  static Operand2 from(int imm) { return Operand2{.opd = imm}; }
  static Operand2 from(Reg r) { return Operand2{.opd = RegImmShift{r}}; }
  static Operand2 from(ShiftType type, Reg r, int s) {
    return Operand2{.opd = RegImmShift{type, r, s}};
  }
  static Operand2 from(float imm) { return Operand2{.opd = imm}; }
};

struct Instruction {
  ExCond cond;

  Instruction() : cond{ExCond::Always} {}
  virtual ~Instruction() = default;

  virtual void emit(std::ostream &os) const {}
  virtual std::set<Reg> def() const { return {}; }
  virtual std::set<Reg> use() const { return {}; }
  virtual std::vector<Reg *> reg_ptrs() { return {}; }

  void replace_reg(Reg src, Reg dst) {
    for (auto p : reg_ptrs())
      if (*p == src)
        *p = dst;
  }

  template <typename T> bool is() const {
    return dynamic_cast<const T *>(this) != nullptr;
  }

  std::ostream &write_op(std::ostream &os, const char *op,
                         bool is_float = false, bool is_ldst = false,
                         bool is_push_pop = false) const;
};

void next_instruction(std::ostream &os);

// 形如 op Rd, Rm, Rn 的指令
struct RType : Instruction {
  enum Op { Add, Sub, Mul, Div } op;
  Reg dst, s1, s2;

  RType(Op op, Reg dst, Reg s1, Reg s2) : op{op}, dst{dst}, s1{s1}, s2{s2} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {s1, s2}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &s1, &s2}; }

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

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {s1}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &s1}; }
};

// 具有 op Rd, R1, Operand2 形式的指令
struct FullRType : Instruction {
  enum Op { Add, Sub, RevSub } op;
  Reg dst, s1;
  Operand2 s2;

  FullRType(Op op, Reg dst, Reg s1, Operand2 s2)
      : op{op}, dst{dst}, s1{s1}, s2{std::move(s2)} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override {
    auto u = s2.get_use();
    u.insert(s1);
    return u;
  }
  std::vector<Reg *> reg_ptrs() override {
    auto ptrs = s2.get_reg_ptrs();
    ptrs.push_back(&dst);
    ptrs.push_back(&s1);
    return ptrs;
  }
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

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return src.get_use(); }
  std::vector<Reg *> reg_ptrs() override {
    auto ptrs = src.get_reg_ptrs();
    ptrs.push_back(&dst);
    return ptrs;
  }

  // “正规”的mov Rd, Rs
  // 不允许通用寄存器与浮点寄存器之间的 vmov
  bool is_reg_mov() const {
    if (!src.is_imm_shift())
      return false;
    auto &reg_shift = std::get<RegImmShift>(src.opd);
    return !flip && reg_shift.s == 0 &&
           (dst.is_float() == reg_shift.r.is_float());
  }

  bool is_transfer_vmov() const {
    if (!src.is_imm_shift()) {
      return false;
    }
    auto &reg_shift = std::get<RegImmShift>(src.opd);
    return dst.is_float() != reg_shift.r.is_float();
  }
};

// Thumb-2 movw: 加载低16位，清零高16位
struct MovW : Instruction {
  Reg dst;
  int imm;

  MovW(Reg dst, int imm) : dst{dst}, imm{imm} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

// Thumb-2 movt: 加载高16位，低16位不变
struct MovT : Instruction {
  Reg dst;
  int imm;

  MovT(Reg dst, int imm) : dst{dst}, imm{imm} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {dst}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

// 加载全局变量的地址
// 可能会被翻译为movw + movt
// 足够近也可能是adr或ldr伪指令
struct LoadAddr : Instruction {
  Reg dst;
  std::string symbol;

  LoadAddr(Reg dst, std::string sym) : dst{dst}, symbol{std::move(sym)} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
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

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override {
    auto u = s2.get_use();
    u.insert(s1);
    return u;
  }
  std::vector<Reg *> reg_ptrs() override {
    auto ptrs = s2.get_reg_ptrs();
    ptrs.push_back(&s1);
    return ptrs;
  }
};

// 简单load: base_reg + offset_imm 寻址
struct Load : Instruction {
  Reg dst, base;
  int offset;

  Load(Reg dst, Reg base, int offset) : dst{dst}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {base}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &base}; }
};

// 简单store: base_reg + offset_imm 寻址
struct Store : Instruction {
  Reg src, base;
  int offset;

  Store(Reg src, Reg base, int offset) : src{src}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {src, base}; }
  std::vector<Reg *> reg_ptrs() override { return {&src, &base}; }
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

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {s1, s2, s3}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &s1, &s2, &s3}; }
};

struct BasicBlock;
struct Terminator : Instruction {};

// B.cond
struct Branch : Terminator {
  BasicBlock *target;

  Branch(BasicBlock *target) : target{target} {}

  void emit(std::ostream &os) const override;
};

// CBZ/CBNZ
struct RegBranch : Terminator {
  enum Type { Cbnz, Cbz } type;
  BasicBlock *target;
  Reg src;

  RegBranch(Type type, Reg src, BasicBlock *target)
      : type{type}, target{target}, src{src} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {src}; }
  std::vector<Reg *> reg_ptrs() override { return {&src}; }
};

// 实际条件跳转的中间形式，保留了两个分支，需要在最后展开
// 有些情况比较和分支指令应被视作一个整体，避免中间插入其它指令影响cpsr
// NOTE: true_target的跳转条件即此伪指令的条件码
struct CmpBranch : Terminator {
  std::unique_ptr<Compare> cmp;
  BasicBlock *true_target, *false_target;

  CmpBranch(Compare *inner_cmp, BasicBlock *true_target,
            BasicBlock *false_target)
      : cmp{inner_cmp}, true_target{true_target}, false_target{false_target} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return cmp->use(); }
  std::vector<Reg *> reg_ptrs() override { return cmp->reg_ptrs(); }
};

// 下面是与栈指针sp相关的指令
// 为了拓展性，StackObject *与int offset配对，offset表示相对于stack object的偏移
// 由于sp不被用于寄存器分配，此类指令的def和use都不显含sp
struct StackObject;
struct SpRelative : Instruction {};

// dst = base + offset
struct LoadStackAddr : SpRelative {
  Reg dst;
  StackObject *base;
  int offset;

  LoadStackAddr(Reg dst, StackObject *base, int offset)
      : dst{dst}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

// dst = [base + offset]
struct LoadStack : SpRelative {
  Reg dst;
  StackObject *base;
  int offset;

  LoadStack(Reg dst, StackObject *base, int offset)
      : dst{dst}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

struct StoreStack : SpRelative {
  Reg src;
  StackObject *base;
  int offset;

  StoreStack(Reg src, StackObject *base, int offset)
      : src{src}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {src}; }
  std::vector<Reg *> reg_ptrs() override { return {&src}; }
};

// NOTE: 假设需要调整sp的情况下立即数范围总是合法的
// 1. 子函数调用后清栈时sp增加不超过4095
// 2. 分配/释放空间时sp修改量不超过4095或是imm8m
// 否则需要增加add/sub sp, sp, r的指令形式
struct AdjustSp : SpRelative {
  int offset;

  AdjustSp(int offset) : offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct Push : SpRelative {
  std::vector<Reg> srcs;

  Push(std::vector<Reg> srcs) : srcs{std::move(srcs)} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override {
    return std::set<Reg>(srcs.begin(), srcs.end());
  }
  std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs;
    for (Reg &r : srcs)
      ptrs.push_back(&r);
    return ptrs;
  }
};

struct Pop : SpRelative {
  std::vector<Reg> dsts;

  Pop(std::vector<Reg> dsts) : dsts{std::move(dsts)} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override {
    return std::set<Reg>(dsts.begin(), dsts.end());
  }
  std::set<Reg> use() const override { return {}; }
  // NOTE: Pop的操作数是物理寄存器，不提供可修改的寄存器列表
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct Call : Instruction {
  std::string func;
  int nr_gp_args, nr_fp_args;
  int variadic_at; // 第几个参数是 `...`

  Call(std::string func, int nr_gp_args, int nr_fp_args, int variadic_at = -1)
      : func{std::move(func)}, nr_gp_args{nr_gp_args}, nr_fp_args{nr_fp_args},
        variadic_at{variadic_at} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override {
    // lr和所有caller-saved (volatile)寄存器
    std::set<Reg> d{Reg{General, lr}};
    for (int i = 0; i < NR_GPRS; ++i)
      if (GPRS_ATTR[i] == Volatile)
        d.insert(Reg{General, i});
    for (int i = 0; i < NR_VOLATILE_FPRS; ++i)
      d.insert(Reg{Fp, i});
    return d;
  }
  std::set<Reg> use() const override {
    std::set<Reg> u;
    int gp_regs = nr_gp_args < NR_ARG_GPRS ? nr_gp_args : NR_ARG_GPRS;
    int fp_regs = nr_fp_args < NR_ARG_FPRS ? nr_fp_args : NR_ARG_FPRS;
    for (int i = 0; i < gp_regs; ++i)
      u.insert(Reg{General, ARG_GPRS[i]});
    for (int i = 0; i < fp_regs; ++i)
      u.insert(Reg{Fp, s0 + i});
    return u;
  }
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct Return : Terminator {
  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {Reg{General, r0}, Reg{Fp, s0}}; }
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct CountLeadingZero : Instruction {
  Reg dst, src;

  CountLeadingZero(Reg dst, Reg src) : dst{dst}, src{src} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {src}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

// 伪二元比较，需要在最后阶段被展开
// trick: 实际的op用的是本指令的条件码
struct PseudoCompare : Instruction {
  std::unique_ptr<Compare> cmp;
  Reg dst;

  PseudoCompare(Compare *real_cmp, Reg dst) : cmp{real_cmp}, dst{dst} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return cmp->use(); }
  std::vector<Reg *> reg_ptrs() override {
    auto ptrs = cmp->reg_ptrs();
    ptrs.push_back(&dst);
    return ptrs;
  }
};

enum class ConvertType {
  Float2Int,
  Int2Float,
};

struct Convert : Instruction {
  Reg dst, src;
  ConvertType type;

  Convert(Reg dst, Reg src, ConvertType type) : dst{dst}, src{src}, type{type} {
    assert(dst.is_float());
    assert(src.is_float());
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->src}; }
  std::vector<Reg *> reg_ptrs() override { return {&this->dst, &this->src}; }
};

struct Phi : Instruction {
  std::vector<std::pair<BasicBlock *, Reg>> srcs;
  Reg dst;

  Phi(Reg dst, std::vector<std::pair<BasicBlock *, Reg>> srcs)
      : dst{dst}, srcs{std::move(srcs)} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override {
    std::set<Reg> uses;
    for (auto &[_, r] : srcs)
      uses.insert(r);
    return uses;
  }
  std::vector<Reg *> reg_ptrs() override {
    std::vector<Reg *> ptrs{&dst};
    for (auto &[_, r] : srcs)
      ptrs.push_back(&r);
    return ptrs;
  }
};

struct Vneg : Instruction {
  Reg dst, src;
  Vneg(Reg dst, Reg src) : dst{dst}, src{src} {
    assert(dst.is_float());
    assert(src.is_float());
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->src}; }
  std::vector<Reg *> reg_ptrs() override { return {&this->dst, &this->src}; }
};

} // namespace armv7

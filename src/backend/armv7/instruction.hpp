#pragma once

#include "backend/armv7/arch.hpp"

#include "common/Display.hpp"
#include "common/ir.hpp"
#include "common/utils.hpp"

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
  static Reg from(int t, int id) { return Reg{ir_to_machine_reg_type(t), id}; }
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
  Cs,
  Cc,
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

std::optional<std::pair<ShiftType, int>>
combine_shift(std::pair<ShiftType, int> lhs, std::pair<ShiftType, int> rhs);

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

  bool is_reg() const {
    return this->is_imm_shift() && this->get<RegImmShift>().s == 0;
  }

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
  static Operand2 from(ShiftType type, Reg r, Reg s) {
    return Operand2{.opd = RegRegShift{type, r, s}};
  }
  static Operand2 from(float imm) { return Operand2{.opd = imm}; }
};

struct Instruction : Display {
  ExCond cond;
  bool update_cpsr;

  Instruction() : cond{ExCond::Always}, update_cpsr{false} {}
  virtual ~Instruction() = default;

  virtual void emit(std::ostream &os) const {}
  virtual std::set<Reg> def() const { return {}; }
  virtual std::set<Reg> use() const { return {}; }
  virtual std::vector<Reg *> reg_ptrs() { return {}; }

  [[nodiscard]] virtual std::unique_ptr<Instruction> clone() const = 0;

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
                         bool is_push_pop = false, bool padding = true) const;

  void print(std::ostream &out, unsigned indent) const final {
    print_indent(out, indent);
    this->emit(out);
  }
};

template <typename Derived> struct DefaultCloneableInstruction : Instruction {
  [[nodiscard]] std::unique_ptr<Instruction> clone() const final {
    return std::unique_ptr<Instruction>{
        new Derived{*static_cast<Derived const *>(this)}};
  }
};

void next_instruction(std::ostream &os);

// 形如 op Rd, Rm, Rn 的指令
struct RType final : DefaultCloneableInstruction<RType> {
  enum Op { Add, Sub, Mul, Div, SMMul } op;
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
struct IType final : DefaultCloneableInstruction<IType> {
  enum Op { Add, Sub, RevSub, Eor, Bic, And } op;
  Reg dst, s1;
  int imm;

  IType(Op op, Reg dst, Reg s1, int imm) : op{op}, dst{dst}, s1{s1}, imm{imm} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {s1}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &s1}; }
};

// 具有 op Rd, R1, Operand2 形式的指令
struct FullRType final : DefaultCloneableInstruction<FullRType> {
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
struct Move final : DefaultCloneableInstruction<Move> {
  Reg dst;
  Operand2 src;
  bool flip;

  Move(Reg dst, Operand2 src) : dst{dst}, src{std::move(src)}, flip{false} {}
  Move(Reg dst, Operand2 src, bool flip)
      : dst{dst}, src{std::move(src)}, flip{flip} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override {
    auto u = src.get_use();
    return u;
  }
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
struct MovW final : DefaultCloneableInstruction<MovW> {
  Reg dst;
  int imm;

  MovW(Reg dst, int imm) : dst{dst}, imm{imm} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst}; }
};

// Thumb-2 movt: 加载高16位，低16位不变
struct MovT final : DefaultCloneableInstruction<MovT> {
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
struct LoadAddr final : DefaultCloneableInstruction<LoadAddr> {
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
struct Compare final : DefaultCloneableInstruction<Compare> {
  Reg s1;
  Operand2 s2;
  bool neg;

  Compare(Reg s1, Operand2 s2) : Compare(s1, std::move(s2), false) {}
  Compare(Reg s1, Operand2 s2, bool neg) : s1{s1}, s2{std::move(s2)}, neg{neg} {
    this->update_cpsr = true;
  }

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
struct Load final : DefaultCloneableInstruction<Load> {
  Reg dst, base;
  int offset;

  Load(Reg dst, Reg base, int offset) : dst{dst}, base{base}, offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {base}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &base}; }
};

// 简单store: base_reg + offset_imm 寻址
struct Store final : DefaultCloneableInstruction<Store> {
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
// SMMLA Rd, Rm, Rs, Rn: Rd := high(Rn + Rm * Rs)
struct FusedMul final : DefaultCloneableInstruction<FusedMul> {
  enum Op { Add, Sub, SMAdd } op;
  Reg dst, s1, s2, s3;

  FusedMul(Op op, Reg dst, Reg s1, Reg s2, Reg s3)
      : op{op}, dst{dst}, s1{s1}, s2{s2}, s3{s3} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {s1, s2, s3}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &s1, &s2, &s3}; }
};

struct BasicBlock;
struct Terminator : Instruction {};
template <typename Derived> struct DefaultCloneableTerminator : Terminator {
  [[nodiscard]] std::unique_ptr<Instruction> clone() const final {
    return std::unique_ptr<Instruction>{
        new Derived{*static_cast<Derived const *>(this)}};
  }
};

// B.cond
struct Branch final : DefaultCloneableTerminator<Branch> {
  BasicBlock *target;

  Branch(BasicBlock *target) : target{target} {}

  void emit(std::ostream &os) const override;
};

// CBZ/CBNZ
struct RegBranch final : DefaultCloneableTerminator<RegBranch> {
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
struct CmpBranch final : DefaultCloneableTerminator<CmpBranch> {
  std::unique_ptr<Compare> cmp;
  BasicBlock *true_target, *false_target;

  CmpBranch(Compare *inner_cmp, BasicBlock *true_target,
            BasicBlock *false_target)
      : cmp{inner_cmp}, true_target{true_target}, false_target{false_target} {
    this->update_cpsr = true;
  }
  CmpBranch(CmpBranch const &other)
      : DefaultCloneableTerminator{other}, cmp{static_cast<Compare *>(
                                               other.cmp->clone().release())},
        true_target{other.true_target}, false_target{other.false_target} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return cmp->use(); }
  std::vector<Reg *> reg_ptrs() override { return cmp->reg_ptrs(); }
};

struct Switch final : DefaultCloneableTerminator<Switch> {
  Reg val, tmp;
  std::vector<std::pair<int, BasicBlock *>> targets;
  BasicBlock *default_target;

  Switch(Reg val, Reg tmp, BasicBlock *default_target,
         std::vector<std::pair<int, BasicBlock *>> targets)
      : val{val}, tmp{tmp}, default_target{default_target}, targets{std::move(
                                                                targets)} {
    this->update_cpsr = true;
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {tmp}; }
  std::set<Reg> use() const override { return {val, tmp}; }
  std::vector<Reg *> reg_ptrs() override { return {&val, &tmp}; }
};

// 下面是与栈指针sp相关的指令
// 为了拓展性，StackObject *与int offset配对，offset表示相对于stack object的偏移
// 由于sp不被用于寄存器分配，此类指令的def和use都不显含sp
struct StackObject;
struct SpRelative : Instruction {};
template <typename Derived> struct DefaultCloneableSpRelative : SpRelative {
  [[nodiscard]] std::unique_ptr<Instruction> clone() const final {
    return std::unique_ptr<Instruction>{
        new Derived{*static_cast<Derived const *>(this)}};
  }
};

// dst = base + offset
struct LoadStackAddr final : DefaultCloneableSpRelative<LoadStackAddr> {
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
struct LoadStack final : DefaultCloneableSpRelative<LoadStack> {
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

struct StoreStack final : DefaultCloneableSpRelative<StoreStack> {
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
struct AdjustSp final : DefaultCloneableSpRelative<AdjustSp> {
  int offset;

  AdjustSp(int offset) : offset{offset} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {}; }
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct Push final : DefaultCloneableSpRelative<Push> {
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

struct Pop final : DefaultCloneableSpRelative<Pop> {
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

struct Call final : DefaultCloneableInstruction<Call> {
  std::string func;
  int nr_gp_args, nr_fp_args;
  int variadic_at; // 第几个参数是 `...`

  Call(std::string func, int nr_gp_args, int nr_fp_args, int variadic_at = -1)
      : func{std::move(func)}, nr_gp_args{nr_gp_args}, nr_fp_args{nr_fp_args},
        variadic_at{variadic_at} {
    this->update_cpsr = true;
  }

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

struct Return final : DefaultCloneableTerminator<Return> {
  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override { return {Reg{General, r0}, Reg{Fp, s0}}; }
  std::vector<Reg *> reg_ptrs() override { return {}; }
};

struct CountLeadingZero final : DefaultCloneableInstruction<CountLeadingZero> {
  Reg dst, src;

  CountLeadingZero(Reg dst, Reg src) : dst{dst}, src{src} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {src}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

// 伪逻辑非，需要展开
struct PseudoNot : DefaultCloneableInstruction<PseudoNot> {
  Reg dst, src;

  PseudoNot(Reg dst, Reg src) : dst{dst}, src{src} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {dst}; }
  std::set<Reg> use() const override { return {src}; }
  std::vector<Reg *> reg_ptrs() override { return {&dst, &src}; }
};

// 伪二元比较，需要在最后阶段被展开
// trick: 实际的op用的是本指令的条件码
struct PseudoCompare final : DefaultCloneableInstruction<PseudoCompare> {
  std::unique_ptr<Compare> cmp;
  Reg dst;

  PseudoCompare(Compare *real_cmp, Reg dst) : cmp{real_cmp}, dst{dst} {
    this->update_cpsr = true;
  }
  PseudoCompare(PseudoCompare const &other)
      : DefaultCloneableInstruction{other}, cmp{static_cast<Compare *>(
                                                other.cmp->clone().release())},
        dst{other.dst} {}

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

struct Convert final : DefaultCloneableInstruction<Convert> {
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

struct Phi final : DefaultCloneableInstruction<Phi> {
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

struct Vneg final : DefaultCloneableInstruction<Vneg> {
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

struct ComplexLoad final : DefaultCloneableInstruction<ComplexLoad> {
  Reg dst, base, offset;
  ShiftType shift_type;
  int shift;
  bool neg;
  ComplexLoad(Reg dst, Reg base, Reg offset, bool neg)
      : ComplexLoad(dst, base, offset, ShiftType::LSL, 0, neg) {}
  ComplexLoad(Reg dst, Reg base, Reg offset, ShiftType shift_type, int shift,
              bool neg)
      : dst{dst}, base{base}, offset{offset},
        shift_type{shift_type}, shift{shift}, neg{neg} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->base, this->offset}; }
  std::vector<Reg *> reg_ptrs() override {
    return {&this->dst, &this->base, &this->offset};
  }
};

struct ComplexStore final : DefaultCloneableInstruction<ComplexStore> {
  Reg src, base, offset;
  ShiftType shift_type;
  int shift;
  bool neg;
  ComplexStore(Reg src, Reg base, Reg offset, bool neg)
      : ComplexStore(src, base, offset, ShiftType::LSL, 0, neg) {}
  ComplexStore(Reg src, Reg base, Reg offset, ShiftType shift_type, int shift,
               bool neg)
      : src{src}, base{base}, offset{offset},
        shift_type{shift_type}, shift{shift}, neg{neg} {}

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {}; }
  std::set<Reg> use() const override {
    return {this->src, this->base, this->offset};
  }
  std::vector<Reg *> reg_ptrs() override {
    return {&this->src, &this->base, &this->offset};
  }
};

// sdiv dst, #1, src
// 伪指令，在最后阶段展开
struct PseudoOneDividedByReg final
    : DefaultCloneableInstruction<PseudoOneDividedByReg> {
  Reg dst, src;

  PseudoOneDividedByReg(Reg dst, Reg src) : dst{dst}, src{src} {
    this->update_cpsr = true;
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->src}; }
  std::vector<Reg *> reg_ptrs() override { return {&this->dst, &this->src}; }
};

// dst = s1 mod s2
// 伪指令，在 `fold_constants` 阶段提前展开
struct PseudoModulo final : DefaultCloneableInstruction<PseudoModulo> {
  Reg dst, s1, s2;

  PseudoModulo(Reg dst, Reg s1, Reg s2) : dst{dst}, s1{s1}, s2{s2} {
    assert(!this->dst.is_float());
    assert(!this->s1.is_float());
    assert(!this->s2.is_float());
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->s1, this->s2}; }
  std::vector<Reg *> reg_ptrs() override {
    return {&this->dst, &this->s1, &this->s2};
  }
};

// dst = s1 / (1 << s2)
// 伪指令，在 `fold_constants` 阶段提前展开
struct PseudoDivPowerTwo : DefaultCloneableInstruction<PseudoDivPowerTwo> {
  Reg dst, s1, s2;

  PseudoDivPowerTwo(Reg dst, Reg s1, Reg s2) : dst{dst}, s1{s1}, s2{s2} {
    assert(!this->dst.is_float());
    assert(!this->s1.is_float());
    assert(!this->s2.is_float());
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->s1, this->s2}; }
  std::vector<Reg *> reg_ptrs() override {
    return {&this->dst, &this->s1, &this->s2};
  }
};

// bfc dst, #lsb, #width
// 清零 #lsb 开始的、宽 #width 的位
struct BitFieldClear final : DefaultCloneableInstruction<BitFieldClear> {
  Reg dst;
  int lsb, width;

  BitFieldClear(Reg dst, int lsb, int width)
      : dst{dst}, lsb{lsb}, width{width} {
    assert(lsb >= 0);
    assert(width > 0);
    assert(width + lsb < 32);
  }

  void emit(std::ostream &os) const override;
  std::set<Reg> def() const override { return {this->dst}; }
  std::set<Reg> use() const override { return {this->dst}; }
  std::vector<Reg *> reg_ptrs() override { return {&this->dst}; }
};

} // namespace armv7

namespace std {
template <> struct hash<armv7::Reg> {
  size_t operator()(armv7::Reg const r) const {
    return hash<pair<armv7::RegType, int>>{}({r.type, r.id});
  }
};
} // namespace std

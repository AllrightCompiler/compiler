#include "backend/armv7/passes.hpp"
#include "backend/armv7/ColoringRegAllocator.hpp"
#include "backend/armv7/arch.hpp"
#include "backend/armv7/instruction.hpp"
#include "backend/armv7/merge_instr.hpp"

#include "common/common.hpp"
#include "common/utils.hpp"

#include <iostream>
#include <iterator>

namespace armv7 {

void backend_passes(Program &p) {
  ColoringRegAllocator reg_allocator;

  // 具有f.some_pass()形式的步骤必须进行
  for (auto &[_, f] : p.functions) {
    fold_constants(f);

    merge_shift_with_binary_op(f);
    merge_add_with_load_or_store(f);
    merge_mul_with_add_or_sub(f);

    f.resolve_phi();

    // propagate_constants(f);
    remove_unused(f);

    f.do_reg_alloc(reg_allocator, false); // fp reg
    f.do_reg_alloc(reg_allocator);

    remove_nop(f);

    f.emit_prologue_epilogue();

    sanitize_cfg(f);

    f.replace_pseudo_insns();
  }
}

bool is_add_sub_imm(int x) { return is_imm8m(x); }

int compute_shift(ShiftType type, int r, int s) {
  switch (type) {
  case LSL:
    return r << s;
  case LSR:
    return unsigned(r) >> s;
  case ASR:
    return r >> s;
  case ROR:
    return (unsigned(r) >> s) | (r << (32 - s));
  default:
    __builtin_unreachable();
  }
}

// Division by Invariant Integers using Multiplication
// Figure 6.2
// https://zhuanlan.zhihu.com/p/151038723
std::pair<std::uint64_t, int> choose_multiplier(unsigned d) {
  assert(d != 0);
  constexpr auto N = 32;
  constexpr auto P = N - 1;
  auto l = N - countl_zero(d - 1); // ceil(log2(d))
  auto low = (std::uint64_t(1) << (N + l)) / d;
  auto high =
      ((std::uint64_t(1) << (N + l)) + (std::uint64_t(1) << (N + l - P))) / d;
  while (low / 2 < high / 2 && l > 0) {
    low /= 2;
    high /= 2;
    --l;
  }
  return {high, l};
}

void fold_constants(Function &f) {
  std::map<Reg, ConstValue> constants;

  for (auto &bb : f.bbs) {
    for (auto instr = bb->insns.begin(), next = instr; instr != bb->insns.end();
         instr = next) {
      next = std::next(instr);
      auto &insn = *instr;

      // 匹配常数加载指令
      bool is_load_imm = false;
      TypeCase(mov, Move *, insn.get()) {
        if (mov->src.is_imm8m()) {
          int imm = mov->src.get<int>();
          if (mov->flip)
            imm = ~imm;

          Reg dst = mov->dst;
          if (dst.is_virt())
            constants[dst] = imm;
          is_load_imm = true;
        } else if (mov->src.is_fpimm()) {
          float imm = mov->src.get<float>();
          assert(!mov->flip);
          if (mov->dst.is_virt()) {
            constants[mov->dst] = imm;
          }
          is_load_imm = true;
        }
      }
      else TypeCase(movw, MovW *, insn.get()) {
        Reg dst = movw->dst;
        if (dst.is_virt())
          constants[dst] = movw->imm;
        is_load_imm = true;
      }
      else TypeCase(movt, MovT *, insn.get()) {
        Reg dst = movt->dst;
        if (constants.count(dst)) {
          int lo = constants[dst].iv;
          constants[dst] = (movt->imm << 16) | lo;
          is_load_imm = true;
        }
      }
      if (is_load_imm)
        continue;

      auto get_imm = [&constants](Reg r) -> std::optional<int> {
        auto const iter = constants.find(r);
        if (iter != constants.end()) {
          return iter->second.iv;
        } else {
          return {};
        }
      };

      auto eval_operand2 =
          [&constants](const Operand2 &s2) -> std::optional<int> {
        int x;
        if (s2.is_imm_shift()) {
          // e.g. r1 LSL #2 转常数
          auto sh = s2.get<RegImmShift>();
          if (!constants.count(sh.r))
            return std::nullopt;
          x = compute_shift(sh.type, constants[sh.r].iv, sh.s);
        } else if (s2.is_reg_shift()) {
          // e.g. r1 LSL r2
          auto sh = s2.get<RegRegShift>();
          if (!constants.count(sh.r1) || !constants.count(sh.r2))
            return std::nullopt;
          x = compute_shift(sh.type, constants[sh.r1].iv, constants[sh.r2].iv);
        } else {
          x = s2.get<int>();
        }
        return x;
      };

      auto inline_compare_constant = [&](Compare *cmp,
                                         ExCond &cond) -> std::optional<bool> {
        auto opt_imm1 = get_imm(cmp->s1);
        auto opt_imm2 = eval_operand2(cmp->s2);
        if (opt_imm1 && opt_imm2) {
          switch (cond) {
          case ExCond::Eq:
            return *opt_imm1 == *opt_imm2;
          case ExCond::Ne:
            return *opt_imm1 != *opt_imm2;
          case ExCond::Ge:
            return *opt_imm1 >= *opt_imm2;
          case ExCond::Gt:
            return *opt_imm1 > *opt_imm2;
          case ExCond::Le:
            return *opt_imm1 <= *opt_imm2;
          case ExCond::Lt:
            return *opt_imm1 < *opt_imm2;
          case ExCond::Cs:
            return static_cast<unsigned>(*opt_imm1) >=
                   static_cast<unsigned>(*opt_imm2);
          case ExCond::Cc:
            return static_cast<unsigned>(*opt_imm1) <
                   static_cast<unsigned>(*opt_imm2);
          case ExCond::Always:;
          }
        }
        if (opt_imm2) {
          if (is_imm8m(*opt_imm2)) {
            cmp->s2 = Operand2::from(*opt_imm2);
          } else if (is_imm8m(-*opt_imm2)) {
            cmp->s2 = Operand2::from(-*opt_imm2);
            cmp->neg = !cmp->neg;
          }
        } else if (opt_imm1) {
          if (auto const neg = is_imm8m(-*opt_imm1);
              neg || is_imm8m(*opt_imm1)) {
            Reg tmp;
            if (cmp->s2.is_reg()) {
              tmp = cmp->s2.get<RegImmShift>().r;
            } else {
              tmp = f.new_reg(RegType::General);
              bb->insns.insert(instr, std::make_unique<Move>(tmp, cmp->s2));
            }
            cmp->s1 = tmp;
            cmp->s2 = Operand2::from(neg ? -*opt_imm1 : *opt_imm1);
            cmp->neg ^= neg;
            if (!cmp->neg) {
              cond = logical_not(cond);
            }
          }
        }
        return {};
      };

      TypeCase(r_ins, RType *, insn.get()) {
        auto op = r_ins->op;
        if (r_ins->dst.is_float()) // TODO: 浮点立即数
          continue;
        auto const opt_imm1 = get_imm(r_ins->s1);
        auto const opt_imm2 = get_imm(r_ins->s2);
        switch (op) {
        case RType::Add: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, r_ins->dst,
                                 *opt_imm1 + *opt_imm2);
            bb->insns.erase(instr);
          } else {
            Reg other;
            int imm;
            if (opt_imm1) {
              imm = *opt_imm1;
              other = r_ins->s2;
            } else if (opt_imm2) {
              imm = *opt_imm2;
              other = r_ins->s1;
            } else {
              continue;
            }
            if (imm == 0) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(other));
            } else if (is_add_sub_imm(imm)) {
              insn =
                  std::make_unique<IType>(IType::Add, r_ins->dst, other, imm);
            } else if (is_add_sub_imm(-imm)) {
              insn =
                  std::make_unique<IType>(IType::Sub, r_ins->dst, other, -imm);
            }
          }
        } break;
        case RType::Sub: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, r_ins->dst,
                                 *opt_imm1 - *opt_imm2);
            bb->insns.erase(instr);
          } else if (opt_imm1) {
            if (is_add_sub_imm(*opt_imm1)) {
              insn = std::make_unique<IType>(IType::RevSub, r_ins->dst,
                                             r_ins->s2, *opt_imm1);
            }
          } else if (opt_imm2) {
            if (*opt_imm2 == 0) {
              insn =
                  std::make_unique<Move>(r_ins->dst, Operand2::from(r_ins->s1));
            } else if (is_add_sub_imm(*opt_imm2)) {
              insn = std::make_unique<IType>(IType::Sub, r_ins->dst, r_ins->s1,
                                             *opt_imm2);
            } else if (is_add_sub_imm(-*opt_imm2)) {
              insn = std::make_unique<IType>(IType::Add, r_ins->dst, r_ins->s1,
                                             -*opt_imm2);
            }
          }
        } break;
        case RType::Mul: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, r_ins->dst,
                                 *opt_imm1 * *opt_imm2);
            bb->insns.erase(instr);
          } else {
            Reg other;
            int imm;
            if (opt_imm1) {
              imm = *opt_imm1;
              other = r_ins->s2;
            } else if (opt_imm2) {
              imm = *opt_imm2;
              other = r_ins->s1;
            } else {
              continue;
            }
            if (is_power_of_2(imm)) {
              if (imm == 0) {
                insn = std::make_unique<Move>(r_ins->dst, Operand2::from(0));
                next = instr;
              } else if (imm == 1) {
                insn =
                    std::make_unique<Move>(r_ins->dst, Operand2::from(other));
              } else {
                insn = std::make_unique<Move>(
                    r_ins->dst,
                    Operand2::from(ShiftType::LSL, other, bit_width(imm) - 1));
              }
            } else if (imm == -1) {
              insn =
                  std::make_unique<IType>(IType::RevSub, r_ins->dst, other, 0);
              next = instr;
            } else if (is_power_of_2(imm + 1)) {
              insn = std::make_unique<FullRType>(
                  FullRType::RevSub, r_ins->dst, other,
                  Operand2::from(ShiftType::LSL, other,
                                 bit_width(imm + 1) - 1));
            } else if (is_power_of_2(imm - 1)) {
              insn = std::make_unique<FullRType>(
                  FullRType::Add, r_ins->dst, other,
                  Operand2::from(ShiftType::LSL, other,
                                 bit_width(imm - 1) - 1));
            } else if (is_power_of_2(-imm + 1)) {
              insn = std::make_unique<FullRType>(
                  FullRType::Sub, r_ins->dst, other,
                  Operand2::from(ShiftType::LSL, other,
                                 bit_width(-imm + 1) - 1));
            } else if (is_power_of_2(-imm)) {
              auto const tmp = f.new_reg(RegType::General);
              next = emit_load_imm(bb->insns, instr, tmp, 0);
              insn = std::make_unique<FullRType>(
                  FullRType::Sub, r_ins->dst, tmp,
                  Operand2::from(ShiftType::LSL, other, bit_width(-imm) - 1));
            } else if (is_power_of_2(-imm - 1)) {
              auto const tmp = f.new_reg(RegType::General);
              bb->insns.insert(instr,
                               std::make_unique<FullRType>(
                                   FullRType::Add, tmp, other,
                                   Operand2::from(ShiftType::LSL, other,
                                                  bit_width(-imm - 1) - 1)));
              insn = std::make_unique<IType>(IType::RevSub, r_ins->dst, tmp, 0);
            }
          }
        } break;
        case RType::Div: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, r_ins->dst,
                                 *opt_imm1 / *opt_imm2);
            bb->insns.erase(instr);
          } else if (opt_imm1) {
            auto const imm = *opt_imm1;
            if (imm == 0) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(0));
              next = instr;
            } else if (imm == 1) {
              insn = std::make_unique<PseudoOneDividedByReg>(r_ins->dst,
                                                             r_ins->s2);
            }
          } else if (opt_imm2) {
            auto const imm = *opt_imm2;
            assert(imm != 0);
            auto const abs_imm = std::abs(imm);
            if (is_power_of_2(abs_imm)) {
              if (imm == 0x8000'0000) {
                auto const tmp1 = f.new_reg(RegType::General);
                bb->insns.insert(instr, std::make_unique<IType>(IType::Eor,
                                                                tmp1, r_ins->s1,
                                                                0x8000'0000));
                auto const tmp2 = f.new_reg(RegType::General);
                bb->insns.insert(
                    instr, std::make_unique<CountLeadingZero>(tmp2, tmp1));
                insn = std::make_unique<Move>(
                    r_ins->dst, Operand2::from(ShiftType::LSR, tmp2, 5));
              } else if (imm == 1) {
                insn = std::make_unique<Move>(r_ins->dst,
                                              Operand2::from(r_ins->s1));
              } else if (imm == 2) {
                auto const tmp = f.new_reg(RegType::General);
                bb->insns.insert(
                    instr, std::make_unique<FullRType>(
                               FullRType::Add, tmp, r_ins->s1,
                               Operand2::from(ShiftType::LSR, r_ins->s1, 31)));
                insn = std::make_unique<Move>(
                    r_ins->dst, Operand2::from(ShiftType::ASR, tmp, 1));
              } else if (imm == -1) {
                insn = std::make_unique<IType>(IType::RevSub, r_ins->dst,
                                               r_ins->s1, 0);
              } else if (imm == -2) {
                auto const tmp1 = f.new_reg(RegType::General);
                bb->insns.insert(
                    instr, std::make_unique<FullRType>(
                               FullRType::Add, tmp1, r_ins->s1,
                               Operand2::from(ShiftType::LSR, r_ins->s1, 31)));
                auto const tmp2 = f.new_reg(RegType::General);
                emit_load_imm(bb->insns, instr, tmp2, 0);
                insn = std::make_unique<FullRType>(
                    FullRType::Sub, r_ins->dst, tmp2,
                    Operand2::from(ShiftType::ASR, tmp1, 1));
              } else { // abs_imm > 2
                auto const log_abs_imm = bit_width(abs_imm) - 1;
                assert(log_abs_imm > 0);
                auto const tmp1 = f.new_reg(RegType::General);
                bb->insns.insert(
                    instr,
                    std::make_unique<Move>(
                        tmp1, Operand2::from(ShiftType::ASR, r_ins->s1, 31)));
                auto const tmp2 = f.new_reg(RegType::General);
                bb->insns.insert(instr, std::make_unique<FullRType>(
                                            FullRType::Add, tmp2, r_ins->s1,
                                            Operand2::from(ShiftType::LSR, tmp1,
                                                           32 - log_abs_imm)));
                if (imm > 0) {
                  insn = std::make_unique<Move>(
                      r_ins->dst,
                      Operand2::from(ShiftType::ASR, tmp2, log_abs_imm));
                } else { // imm < 0
                  auto const tmp3 = f.new_reg(RegType::General);
                  emit_load_imm(bb->insns, instr, tmp3, 0);
                  insn = std::make_unique<FullRType>(
                      FullRType::Sub, r_ins->dst, tmp3,
                      Operand2::from(ShiftType::ASR, tmp2, log_abs_imm));
                }
              }
            } else {
              // Division by Invariant Integers using Multiplication
              // Figure 5.2
              auto const [m, l] = choose_multiplier(abs_imm);
              auto const tmp1 = f.new_reg(RegType::General);
              auto const tmp2 = f.new_reg(RegType::General);
              if (m < std::uint64_t(1) << 31) {
                next =
                    emit_load_imm(bb->insns, instr, tmp1, static_cast<int>(m));
                bb->insns.insert(instr,
                                 std::make_unique<RType>(RType::SMMul, tmp2,
                                                         tmp1, r_ins->s1));
              } else {
                emit_load_imm(bb->insns, instr, tmp1,
                              static_cast<int>(m - (std::uint64_t(1) << 32)));
                bb->insns.insert(instr, std::make_unique<FusedMul>(
                                            FusedMul::SMAdd, tmp2, tmp1,
                                            r_ins->s1, r_ins->s1));
              }
              Reg tmp3;
              if (l != 0) {
                tmp3 = f.new_reg(RegType::General);
                bb->insns.insert(
                    instr, std::make_unique<Move>(
                               tmp3, Operand2::from(ShiftType::ASR, tmp2, l)));
              } else {
                tmp3 = tmp2;
              }
              insn = std::make_unique<FullRType>(
                  imm > 0 ? FullRType::Sub : FullRType::RevSub, r_ins->dst,
                  tmp3, Operand2::from(ShiftType::ASR, r_ins->s1, 31));
            }
          }
        } break;
        case RType::SMMul: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(
                bb->insns, instr, r_ins->dst,
                static_cast<int>(
                    static_cast<std::int64_t>(*opt_imm1) * *opt_imm2 >> 32));
            bb->insns.erase(instr);
          } else {
            Reg other;
            int imm;
            if (opt_imm1) {
              imm = *opt_imm1;
              other = r_ins->s2;
            } else if (opt_imm2) {
              imm = *opt_imm2;
              other = r_ins->s1;
            } else {
              continue;
            }
            if (imm == 0) {
              insn = std::make_unique<Move>(r_ins->dst, Operand2::from(0));
              next = instr;
            } else if (is_power_of_2(imm)) {
              if (imm == 1) {
                insn = std::make_unique<Move>(
                    r_ins->dst, Operand2::from(ShiftType::ASR, other, 31));
              } else {
                auto const log_imm = bit_width(imm) - 1;
                insn = std::make_unique<Move>(
                    r_ins->dst,
                    Operand2::from(ShiftType::ASR, other, 32 - log_imm));
              }
            } else if (imm == -1) {
              auto const tmp = f.new_reg(RegType::General);
              instr = bb->insns.insert(
                  instr, std::make_unique<IType>(IType::RevSub, tmp, other, 0));
              insn = std::make_unique<Move>(
                  r_ins->dst, Operand2::from(ShiftType::ASR, tmp, 31));
            }
          }
        } break;
        }
      }
      else TypeCase(i_ins, IType *, insn.get()) {
        auto const opt_imm1 = get_imm(i_ins->s1);
        switch (i_ins->op) {
        case IType::Add: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 *opt_imm1 + i_ins->imm);
            bb->insns.erase(instr);
          } else if (i_ins->imm == 0) {
            insn =
                std::make_unique<Move>(i_ins->dst, Operand2::from(i_ins->s1));
          }
        } break;
        case IType::Sub: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 *opt_imm1 - i_ins->imm);
            bb->insns.erase(instr);
          } else if (i_ins->imm == 0) {
            insn =
                std::make_unique<Move>(i_ins->dst, Operand2::from(i_ins->s1));
          }
        } break;
        case IType::RevSub: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 i_ins->imm - *opt_imm1);
            bb->insns.erase(instr);
          }
        } break;
        case IType::Eor: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 *opt_imm1 ^ i_ins->imm);
            bb->insns.erase(instr);
          } else if (i_ins->imm == 0) {
            insn =
                std::make_unique<Move>(i_ins->dst, Operand2::from(i_ins->s1));
          }
        } break;
        case IType::Bic: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 *opt_imm1 & ~i_ins->imm);
            bb->insns.erase(instr);
          } else if (i_ins->imm == 0) {
            insn =
                std::make_unique<Move>(i_ins->dst, Operand2::from(i_ins->s1));
          } else if (i_ins->imm == 0xffff'ffff) {
            insn = std::make_unique<Move>(i_ins->dst, Operand2::from(0));
            next = instr;
          }
        } break;
        case IType::And: {
          if (opt_imm1) {
            next = emit_load_imm(bb->insns, instr, i_ins->dst,
                                 *opt_imm1 & i_ins->imm);
            bb->insns.erase(instr);
          } else if (i_ins->imm == 0) {
            insn = std::make_unique<Move>(i_ins->dst, Operand2::from(0));
            next = instr;
          } else if (i_ins->imm == 0xffff'ffff) {
            insn =
                std::make_unique<Move>(i_ins->dst, Operand2::from(i_ins->s1));
          }
        } break;
        }
      }
      else TypeCase(fr_ins, FullRType *, insn.get()) {
        auto const opt_imm1 = get_imm(fr_ins->s1);
        auto const opt_imm2 = eval_operand2(fr_ins->s2);
        switch (fr_ins->op) {
        case FullRType::Add: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, fr_ins->dst,
                                 *opt_imm1 + *opt_imm2);
            bb->insns.erase(instr);
          } else if (opt_imm1) {
            if (auto const neg = is_add_sub_imm(-*opt_imm1);
                neg || is_add_sub_imm(*opt_imm1)) {
              Reg tmp;
              if (fr_ins->s2.is_reg()) {
                tmp = fr_ins->s2.get<RegImmShift>().r;
              } else {
                tmp = f.new_reg(RegType::General);
                bb->insns.insert(instr,
                                 std::make_unique<Move>(tmp, fr_ins->s2));
              }
              fr_ins->s1 = tmp;
              fr_ins->s2 = Operand2::from(neg ? -*opt_imm1 : *opt_imm1);
              if (neg) {
                fr_ins->op = FullRType::Sub;
              }
            }
          } else if (opt_imm2) {
            if (is_add_sub_imm(*opt_imm2)) {
              fr_ins->s2 = Operand2::from(*opt_imm2);
            } else if (is_add_sub_imm(-*opt_imm2)) {
              fr_ins->s2 = Operand2::from(-*opt_imm2);
              fr_ins->op = FullRType::Sub;
            }
          }
        } break;
        case FullRType::Sub: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, fr_ins->dst,
                                 *opt_imm1 - *opt_imm2);
            bb->insns.erase(instr);
          } else if (opt_imm1) {
            if (is_add_sub_imm(*opt_imm1)) {
              Reg tmp;
              if (fr_ins->s2.is_reg()) {
                tmp = fr_ins->s2.get<RegImmShift>().r;
              } else {
                tmp = f.new_reg(RegType::General);
                bb->insns.insert(instr,
                                 std::make_unique<Move>(tmp, fr_ins->s2));
              }
              fr_ins->s1 = tmp;
              fr_ins->s2 = Operand2::from(*opt_imm1);
              fr_ins->op = FullRType::RevSub;
            }
          } else if (opt_imm2) {
            if (is_add_sub_imm(*opt_imm2)) {
              fr_ins->s2 = Operand2::from(*opt_imm2);
            } else if (is_add_sub_imm(-*opt_imm2)) {
              fr_ins->s2 = Operand2::from(-*opt_imm2);
              fr_ins->op = FullRType::Add;
            }
          }
        } break;
        case FullRType::RevSub: {
          if (opt_imm1 && opt_imm2) {
            next = emit_load_imm(bb->insns, instr, fr_ins->dst,
                                 *opt_imm2 - *opt_imm1);
            bb->insns.erase(instr);
          } else if (opt_imm1) {
            if (auto const neg = is_add_sub_imm(-*opt_imm1);
                neg || is_add_sub_imm(*opt_imm1)) {
              Reg tmp;
              if (fr_ins->s2.is_reg()) {
                tmp = fr_ins->s2.get<RegImmShift>().r;
              } else {
                tmp = f.new_reg(RegType::General);
                bb->insns.insert(instr,
                                 std::make_unique<Move>(tmp, fr_ins->s2));
              }
              fr_ins->s1 = tmp;
              fr_ins->s2 = Operand2::from(neg ? -*opt_imm1 : *opt_imm1);
              if (neg) {
                fr_ins->op = FullRType::Add;
              } else {
                fr_ins->op = FullRType::Sub;
              }
            }
          } else if (opt_imm2) {
            if (is_add_sub_imm(*opt_imm2)) {
              fr_ins->s2 = Operand2::from(*opt_imm2);
            }
          }
        } break;
        }
      }
      else TypeCase(mov, Move *, insn.get()) {
        auto opt_imm = eval_operand2(mov->src);
        if (opt_imm && !mov->is_transfer_vmov()) {
          int imm = opt_imm.value();
          Reg dst = mov->dst;
          if (dst.is_virt())
            constants[dst] = imm;
          if (is_imm8m(imm)) {
            mov->src = Operand2::from(imm);
            next = instr;
          } else if (is_imm8m(~imm)) {
            mov->src = Operand2::from(~imm);
            mov->flip = !mov->flip;
            next = instr;
          }
        }
      }
      else TypeCase(cmp, Compare *, insn.get()) {
        inline_compare_constant(cmp, cmp->cond);
      }
      else TypeCase(pcmp, PseudoCompare *, insn.get()) {
        auto const res = inline_compare_constant(pcmp->cmp.get(), pcmp->cond);
        if (res) {
          insn =
              std::make_unique<Move>(pcmp->dst, Operand2::from(*res ? 1 : 0));
          next = instr;
        }
      }
      else TypeCase(br, CmpBranch *, insn.get()) {
        auto const res = inline_compare_constant(br->cmp.get(), br->cond);
        if (res) {
          if (*res) {
            insn = std::make_unique<Branch>(br->true_target);
            BasicBlock::remove_edge(bb.get(), br->false_target);
          } else {
            insn = std::make_unique<Branch>(br->false_target);
            BasicBlock::remove_edge(bb.get(), br->true_target);
          }
        }
      }
      else TypeCase(mod, PseudoModulo *, insn.get()) {
        auto const opt_imm1 = get_imm(mod->s1);
        auto const opt_imm2 = get_imm(mod->s2);
        if (opt_imm1 && opt_imm2) {
          next =
              emit_load_imm(bb->insns, instr, mod->dst, *opt_imm1 % *opt_imm2);
          bb->insns.erase(instr);
          continue;
        } else if (opt_imm1) {
          if (*opt_imm1 == 0) {
            insn = std::make_unique<Move>(mod->dst, Operand2::from(0));
            next = instr;
            continue;
          }
        } else if (opt_imm2) {
          auto const imm = *opt_imm2;
          assert(imm != 0);
          if (is_power_of_2(imm)) {
            if (imm == 1) {
              insn = std::make_unique<Move>(mod->dst, Operand2::from(0));
              next = instr;
            } else if (imm == 2) {
              auto const tmp1 = f.new_reg(RegType::General);
              bb->insns.insert(
                  instr, std::make_unique<FullRType>(
                             FullRType::Add, tmp1, mod->s1,
                             Operand2::from(ShiftType::LSR, mod->s1, 31)));
              auto const tmp2 = f.new_reg(RegType::General);
              bb->insns.insert(
                  instr, std::make_unique<IType>(IType::Bic, tmp2, tmp1, 1));
              insn =
                  std::make_unique<RType>(RType::Sub, mod->dst, mod->s1, tmp2);
            } else { // imm > 2
              auto const log_imm = bit_width(imm) - 1;
              assert(log_imm > 0);
              auto const tmp1 = f.new_reg(RegType::General);
              bb->insns.insert(instr, std::make_unique<Move>(
                                          tmp1, Operand2::from(ShiftType::ASR,
                                                               mod->s1, 31)));
              auto const tmp2 = f.new_reg(RegType::General);
              bb->insns.insert(instr, std::make_unique<FullRType>(
                                          FullRType::Add, tmp2, mod->s1,
                                          Operand2::from(ShiftType::LSR, tmp1,
                                                         32 - log_imm)));
              Reg tmp3;
              if (log_imm <= 8) {
                tmp3 = f.new_reg(RegType::General);
                bb->insns.insert(instr, std::make_unique<IType>(
                                            IType::Bic, tmp3, tmp2, imm - 1));
              } else if (log_imm >= 24) {
                tmp3 = f.new_reg(RegType::General);
                bb->insns.insert(instr,
                                 std::make_unique<IType>(IType::And, tmp3, tmp2,
                                                         ~(imm - 1)));
              } else {
                bb->insns.insert(
                    instr, std::make_unique<BitFieldClear>(tmp2, 0, log_imm));
                tmp3 = tmp2;
              }
              insn = std::make_unique<RType>(imm > 0 ? RType::Sub : RType::Add,
                                             mod->dst, mod->s1, tmp3);
            }
            continue;
          } else if (imm == -1) {
            insn = std::make_unique<Move>(mod->dst, Operand2::from(0));
            next = instr;
            continue;
          }
        }
        auto const tmp = f.new_reg(RegType::General);
        next = bb->insns.insert(
            instr, std::make_unique<RType>(RType::Div, tmp, mod->s1, mod->s2));
        insn = std::make_unique<FusedMul>(FusedMul::Sub, mod->dst, tmp, mod->s2,
                                          mod->s1);
      }
      else TypeCase(fused_mul, FusedMul *, insn.get()) {
        auto const opt_imm1 = get_imm(fused_mul->s1);
        auto const opt_imm2 = get_imm(fused_mul->s2);
        auto const opt_imm3 = get_imm(fused_mul->s3);
        assert(fused_mul->op == FusedMul::Add ||
               fused_mul->op == FusedMul::Sub);
        if (auto const s2 = (fused_mul->s1 == fused_mul->s3 && opt_imm2);
            s2 || fused_mul->s2 == fused_mul->s3 && opt_imm1) {
          // s3 ± s1 * s2
          // s3 ± imm * s3
          // s3 * (1 ± imm)
          Reg imm;
          if (s2) {
            imm = fused_mul->s2;
          } else {
            imm = fused_mul->s1;
          }
          auto const tmp = f.new_reg(RegType::General);
          next = bb->insns.insert(
              instr, std::make_unique<IType>(fused_mul->op == FusedMul::Add
                                                 ? IType::Add
                                                 : IType::RevSub,
                                             tmp, imm, 1));
          insn = std::make_unique<RType>(RType::Mul, fused_mul->dst,
                                         fused_mul->s3, tmp);
        } else { // 后续再经过 merge_mul_with_add_or_sub
          auto const tmp = f.new_reg(RegType::General);
          next = bb->insns.insert(
              instr, std::make_unique<RType>(RType::Mul, tmp, fused_mul->s1,
                                             fused_mul->s2));
          insn = std::make_unique<RType>(
              fused_mul->op == FusedMul::Add ? RType::Add : RType::Sub,
              fused_mul->dst, fused_mul->s3, tmp);
        }
      }
      else TypeCase(clz, CountLeadingZero *, insn.get()) {
        auto const opt_imm = get_imm(clz->src);
        if (opt_imm) {
          insn = std::make_unique<Move>(clz->dst,
                                        Operand2::from(countl_zero(*opt_imm)));
          next = instr;
        }
      }
      else TypeCase(_, PseudoOneDividedByReg *, insn.get()) {
        assert(false);
      }
      else TypeCase(bfc, BitFieldClear *, insn.get()) {
        assert(false);
      }
    }
  }
}

// 简单的局部常量传播，只是为了消除phi解构引入的多余mov dst, #imm
void propagate_constants(Function &f) {
  std::map<Reg, int> int_vals;
  for (auto &bb : f.bbs) {
    int_vals.clear();
    for (auto &insn : bb->insns) {
      TypeCase(mov, Move *, insn.get()) {
        if (!mov->is_reg_mov())
          continue;

        Reg dst = mov->dst;
        Reg src = mov->src.get<RegImmShift>().r;
        std::optional<int> opt_imm;
        if (f.reg_val.count(src)) {
          auto &val = f.reg_val.at(src);
          if (val.index() == RegValueType::Imm)
            opt_imm = std::get<Imm>(val);
        }
        if (int_vals.count(src))
          opt_imm = int_vals.at(src);
        
        if (opt_imm) {
          int imm = *opt_imm;
          int_vals[dst] = imm;
          if (is_imm8m(imm))
            mov->src = Operand2::from(imm);
          else if (is_imm8m(~imm)) {
            mov->src = Operand2::from(~imm);
            mov->flip = !mov->flip;
          }
        }
      }
    }
  }
}

void remove_unused(Function &f) {
  f.do_liveness_analysis();
  for (auto &bb : f.bbs) {
    auto live = bb->live_out;
    for (auto it = bb->insns.rbegin(); it != bb->insns.rend();) {
      bool no_effect = true;
      auto ins = it->get();

      if (ins->is<SpRelative>() || ins->is<Store>() ||
          ins->is<ComplexStore>() || ins->is<Call>() || ins->is<Terminator>())
        no_effect = false;
      else {
        for (Reg d : ins->def())
          if (live.count(d))
            no_effect = false;
      }

      if (no_effect) {
        auto base = it.base();
        bb->insns.erase(std::prev(base));
        it = std::make_reverse_iterator(base);
      } else {
        for (Reg d : ins->def())
          live.erase(d);
        for (Reg u : ins->use())
          live.insert(u);
        ++it;
      }
    }
  }
}

void remove_nop(Function &f) {
  for (auto &bb : f.bbs) {
    auto &insns = bb->insns;
    for (auto it = insns.begin(); it != insns.end();) {
      bool remove = false;
      TypeCase(mov, Move *, it->get()) {
        if (mov->is_reg_mov() && mov->use().count(mov->dst) &&
            mov->cond == ExCond::Always)
          remove = true;
      }
      TypeCase(i_type, IType *, it->get()) {
        if (i_type->op == IType::Add || i_type->op == IType::Sub) {
          if (i_type->dst == i_type->s1 && i_type->imm == 0)
            remove = true;
        }
      }

      if (remove)
        it = insns.erase(it);
      else
        ++it;
    }
  }
}

void post_order_dfs(BasicBlock *bb, std::unordered_set<BasicBlock *> &visited,
                    std::vector<BasicBlock *> &order) {
  if (visited.count(bb))
    return;
  visited.insert(bb);
  for (auto next : bb->succ)
    post_order_dfs(next, visited, order);
  order.push_back(bb);
}

std::vector<BasicBlock *> Function::compute_post_order() const {
  std::vector<BasicBlock *> order;
  if (bbs.empty())
    return order;

  std::unordered_set<BasicBlock *> visited;
  post_order_dfs(bbs.front().get(), visited, order);
  return order;
}

// inline std::unique_ptr<Instruction> &get_terminator(BasicBlock *bb) {
//   assert(!bb->insns.empty());
//   auto &insn = bb->insns.back();
//   assert(insn->is<Terminator>());
//   return insn;
// }

bool clean_control_flow(Function &f) {
  if (f.bbs.empty())
    return false;

  std::unordered_set<BasicBlock *> visited;
  std::vector<BasicBlock *> order;
  auto entry = f.bbs.front().get();
  post_order_dfs(entry, visited, order);

  bool changed = false;
  auto get_terminator = [](BasicBlock *bb) -> std::unique_ptr<Instruction> & {
    assert(!bb->insns.empty());
    auto &insn = bb->insns.back();
    // assert(insn->is<Terminator>());
    return insn;
  };
  for (auto bb_iter = order.rbegin(); bb_iter != order.rend(); ++bb_iter) {
    auto bb = *bb_iter;
    if (!visited.count(bb)) // skip newly produced unreachable basic blocks
      continue;

    auto &insn = get_terminator(bb);
    TypeCase(br, CmpBranch *, insn.get()) {
      if (br->true_target == br->false_target) { // case 1
        insn.reset(new Branch{br->true_target});
        changed = true;
      }
    }
    TypeCase(br, Branch *, insn.get()) {
      auto target = br->target;
      if (bb->insns.size() == 1 && bb != entry) { // case 2: remove empty `bb`
        // replace transfers to `bb` with transfers to `target`
        for (auto pred : bb->pred) {
          auto transfer = get_terminator(pred).get();
          TypeCase(cmp_br, CmpBranch *, transfer) {
            if (cmp_br->true_target == bb)
              cmp_br->true_target = target;
            if (cmp_br->false_target == bb)
              cmp_br->false_target = target;
          }
          else TypeCase(jump, Branch *, transfer) {
            assert(jump->target == bb);
            jump->target = target;
          }
          else TypeCase(sw, Switch *, transfer) {
            if (sw->default_target == bb)
              sw->default_target = target;
            for (auto &pair : sw->targets)
              if (pair.second == bb)
                pair.second = target;
          }
          pred->succ.erase(bb);
          pred->succ.insert(target);
        }
        target->pred.erase(bb);
        target->pred.merge(bb->pred);
        bb->pred.clear();
        bb->succ.clear();

        visited.erase(bb);
        changed = true;
        continue;
      }
      if (target->pred.size() == 1) { // case 3
        // merge `target` into `bb`
        for (auto next : target->succ) {
          next->pred.erase(target);
          next->pred.insert(bb);
        }
        bb->succ.erase(target);
        bb->succ.merge(target->succ);
        target->succ.clear();
        target->pred.clear();

        bb->insns.pop_back();
        bb->insns.splice(bb->insns.end(), target->insns);

        visited.erase(target);
        changed = true;
        continue;
      }
      if (target->insns.size() == 1) {
        auto &last = get_terminator(target);
        TypeCase(cmp_br, CmpBranch *, last.get()) { // case 4
                                                    // hoist a branch
          auto true_target = cmp_br->true_target;
          auto false_target = cmp_br->false_target;
          BasicBlock::add_edge(bb, true_target);
          BasicBlock::add_edge(bb, false_target);
          BasicBlock::remove_edge(bb, target);

          auto cmp_cloned = new Compare{*cmp_br->cmp};
          auto cmp_br_cloned =
              new CmpBranch{cmp_cloned, true_target, false_target};
          cmp_br_cloned->cond = cmp_br->cond;
          insn.reset(cmp_br_cloned);
          changed = true;
        }
      }
    }
  }

  // remove unreachable basic blocks
  for (auto it = f.bbs.begin(); it != f.bbs.end();) {
    if (!visited.count(it->get()))
      it = f.bbs.erase(it);
    else
      ++it;
  }
  return changed;
}

void sanitize_cfg(Function &f) {
  bool changed;
  do {
    changed = clean_control_flow(f);
  } while (changed);
}

} // namespace armv7

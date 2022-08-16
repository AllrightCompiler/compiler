#include "backend/armv7/merge_instr.hpp"

#include "common/common.hpp"

#include <functional>

namespace armv7 {

// def -> bool
// reg, def, use -> instr
static void merge_instr(
    Function &func,
    std::function<bool(Instruction const &)> const &check_def_instr,
    std::function<std::unique_ptr<Instruction>(Reg, Instruction const &,
                                               Instruction const &)> const
        &transform_use_instr) {
  func.build_def_use();
  for (auto const &[r, occurs] : func.reg_def) {
    if (occurs.size() != 1) {
      continue;
    }
    auto &def = *occurs.cbegin();
    if (def.instr->get()->cond != ExCond::Always) {
      continue;
    }
    if (!check_def_instr(**def.instr)) {
      continue;
    }
    int max_use_instr_index = -1;
    std::vector<std::pair<OccurPoint const *, std::unique_ptr<Instruction>>>
        new_instrs;
    auto const &uses = func.reg_use[r];
    for (auto const &use : uses) {
      auto new_instr = transform_use_instr(r, **def.instr, **use.instr);
      if (!new_instr) {
        goto continue_;
      }
      new_instr->cond = use.instr->get()->cond;
      new_instrs.emplace_back(&use, std::move(new_instr));
      if (use.bb == def.bb) {
        max_use_instr_index = std::max(use.index, max_use_instr_index);
      }
    }
    for (auto dep_r : def.instr->get()->use()) {
      for (auto const &dep_def : func.reg_def[dep_r]) {
        if (dep_def.bb == def.bb && def.index <= dep_def.index &&
            dep_def.index <= max_use_instr_index) {
          goto continue_;
        }
      }
    }
    for (auto &[use, new_instr] : new_instrs) {
      func.erase_def_use(*use, **use->instr);
      *use->instr = std::move(new_instr);
      func.insert_def_use(*use, **use->instr);
    }
  continue_:
    continue;
  }
}

void merge_shift_with_binary_op(Function &func) {
  auto const check_def_instr = [](Instruction const &def) -> bool {
    TypeCase(instr, Move const *, &def) {
      return instr->is_reg_mov() && instr->src.get<RegImmShift>().s != 0;
    }
    else {
      return false;
    }
  };
  auto const transform_use_instr =
      [](Reg r, Instruction const &def,
         Instruction const &use) -> std::unique_ptr<Instruction> {
    auto &instr = dynamic_cast<Move const &>(def);
    assert(instr.is_reg_mov());
    assert(instr.dst == r);
    auto &shift = instr.src.get<RegImmShift>();
    TypeCase(r_instr, RType const *, &use) {
      if ((r_instr->s1 == r) == (r_instr->s2 == r)) {
        return {};
      }
      if (r_instr->s1 == r) {
        switch (r_instr->op) {
        case RType::Add:
          return std::make_unique<FullRType>(
              FullRType::Add, r_instr->dst, r_instr->s2,
              Operand2::from(shift.type, shift.r, shift.s));
        case RType::Sub:
          return std::make_unique<FullRType>(
              FullRType::RevSub, r_instr->dst, r_instr->s2,
              Operand2::from(shift.type, shift.r, shift.s));
        default:
          return {};
        }
      } else { // r_instr->s2 == r
        switch (r_instr->op) {
        case RType::Add:
          return std::make_unique<FullRType>(
              FullRType::Add, r_instr->dst, r_instr->s1,
              Operand2::from(shift.type, shift.r, shift.s));
        case RType::Sub:
          return std::make_unique<FullRType>(
              FullRType::Sub, r_instr->dst, r_instr->s1,
              Operand2::from(shift.type, shift.r, shift.s));
        default:
          return {};
        }
      }
    }
    TypeCase(fr_instr, FullRType const *, &use) {
      if (fr_instr->s2.is_imm_shift()) {
        auto &s2 = fr_instr->s2.get<RegImmShift>();
        if ((fr_instr->s1 == r) == (s2.r == r)) {
          return {};
        }
        if (fr_instr->s1 == r) {
          if (s2.s == 0) {
            switch (fr_instr->op) {
            case FullRType::Add:
              return std::make_unique<FullRType>(FullRType::Add, fr_instr->dst,
                                                 s2.r, Operand2{shift});
            case FullRType::Sub:
              return std::make_unique<FullRType>(
                  FullRType::RevSub, fr_instr->dst, s2.r, Operand2{shift});
            case FullRType::RevSub:
              return std::make_unique<FullRType>(FullRType::Sub, fr_instr->dst,
                                                 s2.r, Operand2{shift});
            }
          }
        } else { // s2.r == r
          if (auto const new_shift =
                  combine_shift({shift.type, shift.s}, {s2.type, s2.s})) {
            return std::make_unique<FullRType>(
                fr_instr->op, fr_instr->dst, fr_instr->s1,
                Operand2::from(new_shift->first, r, new_shift->second));
          }
        }
      }
      TypeCase(load, ComplexLoad const *, &use) {
        if ((load->base == r) == (load->offset == r)) {
          return {};
        }
        if (load->base == r) {
          if (load->shift == 0) {
            return std::make_unique<ComplexLoad>(load->dst, load->offset,
                                                 shift.r, shift.type, shift.s);
          }
        } else { // load->offset == r
          if (auto const new_shift = combine_shift(
                  {shift.type, shift.s}, {load->shift_type, load->shift})) {
            return std::make_unique<ComplexLoad>(
                load->dst, load->base, r, new_shift->first, new_shift->second);
          }
        }
      }
      TypeCase(store, ComplexStore const *, &use) {
        if ((store->base == r) == (store->offset == r)) {
          return {};
        }
        if (store->base == r) {
          if (store->shift == 0) {
            return std::make_unique<ComplexStore>(store->src, store->offset,
                                                  shift.r, shift.type, shift.s);
          }
        } else { // load->offset == r
          if (auto const new_shift = combine_shift(
                  {shift.type, shift.s}, {store->shift_type, store->shift})) {
            return std::make_unique<ComplexStore>(store->src, store->base, r,
                                                  new_shift->first,
                                                  new_shift->second);
          }
        }
      }
    }
    return {};
  };
  merge_instr(func, check_def_instr, transform_use_instr);
}
void merge_add_with_load_or_store(Function &func) {
  auto const check_def_instr = [](Instruction const &def) -> bool {
    TypeCase(instr, RType const *, &def) { return instr->op == RType::Add; }
    TypeCase(instr, IType const *, &def) {
      return instr->op == IType::Add || instr->op == IType::Sub;
    }
    TypeCase(instr, FullRType const *, &def) {
      if (instr->s2.is_imm8m()) {
        return instr->op == FullRType::Add || instr->op == FullRType::Sub;
      } else if (instr->s2.is_imm_shift()) {
        return instr->op == FullRType::Add;
      }
    }
    return false;
  };
  auto const transform_use_instr =
      [](Reg r, Instruction const &def,
         Instruction const &use) -> std::unique_ptr<Instruction> {
    TypeCase(instr, RType const *, &def) {
      assert(instr->dst == r);
      TypeCase(load, Load const *, &use) {
        if (!load->dst.is_float() && load->offset == 0 && load->base == r) {
          return std::make_unique<ComplexLoad>(load->dst, instr->s1, instr->s2);
        }
      }
      TypeCase(store, Store const *, &use) {
        if (!store->src.is_float() && store->offset == 0 && store->base == r) {
          return std::make_unique<ComplexStore>(store->src, instr->s1,
                                                instr->s2);
        }
      }
    }
    TypeCase(instr, IType const *, &def) {
      assert(instr->dst == r);
      int offset = instr->imm;
      if (instr->op == IType::Sub) {
        offset = -offset;
      }
      TypeCase(load, Load const *, &use) {
        if (!load->dst.is_float() && load->base == r &&
            is_valid_ldst_offset(offset + load->offset)) {
          return std::make_unique<Load>(load->dst, instr->s1,
                                        offset + load->offset);
        }
      }
      TypeCase(store, Store const *, &use) {
        if (!store->src.is_float() && store->base == r &&
            is_valid_ldst_offset(offset + store->offset)) {
          return std::make_unique<Store>(store->src, instr->s1,
                                         offset + store->offset);
        }
      }
    }
    TypeCase(instr, FullRType const *, &def) {
      assert(instr->dst == r);
      if (instr->s2.is_imm8m()) {
        int offset = instr->s2.get<int>();
        if (instr->op == FullRType::Sub) {
          offset = -offset;
        }
        TypeCase(load, Load const *, &use) {
          if (!load->dst.is_float() && load->base == r &&
              is_valid_ldst_offset(offset + load->offset)) {
            return std::make_unique<Load>(load->dst, instr->s1,
                                          offset + load->offset);
          }
        }
        TypeCase(store, Store const *, &use) {
          if (!store->src.is_float() && store->base == r &&
              is_valid_ldst_offset(offset + store->offset)) {
            return std::make_unique<Store>(store->src, instr->s1,
                                           offset + store->offset);
          }
        }
      } else if (instr->s2.is_imm_shift()) {
        auto &s2 = instr->s2.get<RegImmShift>();
        TypeCase(load, Load const *, &use) {
          if (!load->dst.is_float() && load->offset == 0 && load->base == r) {
            return std::make_unique<ComplexLoad>(load->dst, instr->s1, s2.r,
                                                 s2.type, s2.s);
          }
        }
        TypeCase(store, Store const *, &use) {
          if (!store->src.is_float() && store->offset == 0 &&
              store->base == r) {
            return std::make_unique<ComplexStore>(store->src, instr->s1, s2.r,
                                                  s2.type, s2.s);
          }
        }
      }
    }
    return {};
  };
  merge_instr(func, check_def_instr, transform_use_instr);
}
void merge_mul_with_add_or_sub(Function &func) {
  auto const check_def_instr = [](Instruction const &def) -> bool {
    TypeCase(instr, RType const *, &def) { return instr->op == RType::Mul; }
    else {
      return false;
    }
  };
  auto const transform_use_instr =
      [](Reg r, Instruction const &def,
         Instruction const &use) -> std::unique_ptr<Instruction> {
    auto &instr = dynamic_cast<RType const &>(def);
    assert(instr.dst == r);
    TypeCase(r_instr, RType const *, &use) {
      if (r_instr->s1 == r_instr->s2) {
        return {};
      }
      switch (r_instr->op) {
      case RType::Add: {
        Reg other;
        if (r_instr->s1 == r) {
          other = r_instr->s2;
        } else {
          other = r_instr->s1;
        }
        return std::make_unique<FusedMul>(FusedMul::Add, r_instr->dst, instr.s1,
                                          instr.s2, other);
      } break;
      case RType::Sub: {
        if (r_instr->s2 == r) {
          return std::make_unique<FusedMul>(FusedMul::Sub, r_instr->dst,
                                            instr.s1, instr.s2, r_instr->s1);
        }
      } break;
      default:
        return {};
      }
    }
    TypeCase(fr_instr, FullRType const *, &use) {
      if (!fr_instr->s2.is_reg()) {
        return {};
      }
      auto const s2 = fr_instr->s2.get<RegImmShift>().r;
      if (fr_instr->s1 == s2) {
        return {};
      }
      switch (fr_instr->op) {
      case FullRType::Add: {
        Reg other;
        if (fr_instr->s1 == r) {
          other = s2;
        } else {
          other = fr_instr->s1;
        }
        return std::make_unique<FusedMul>(FusedMul::Add, fr_instr->dst,
                                          instr.s1, instr.s2, other);
      } break;
      case FullRType::Sub: {
        if (s2 == r) {
          return std::make_unique<FusedMul>(FusedMul::Sub, fr_instr->dst,
                                            instr.s1, instr.s2, fr_instr->s1);
        }
      } break;
      case FullRType::RevSub: {
        if (fr_instr->s1 == r) {
          return std::make_unique<FusedMul>(FusedMul::Sub, fr_instr->dst,
                                            instr.s1, instr.s2, s2);
        }
      } break;
      }
    }
    return {};
  };
  merge_instr(func, check_def_instr, transform_use_instr);
}

} // namespace armv7

#include "backend/llvm/program.hpp"
#include "backend/llvm/instruction.hpp"

#include "mediumend/cfg.hpp"

#include "common/ir.hpp"

#include <cassert>
#include <iostream>
#include <set>
#include <string>

namespace llvm {

class ProgramTranslator {
  const ir::Program &src;
  llvm::Program &dst;

  std::unordered_map<ir::BasicBlock *, llvm::BasicBlock *> bb_map;
  std::map<ir::Reg, Type> reg_type;   // 只保存非标量寄存器 (String)
  std::map<ir::Reg, ir::Reg> i1_regs; // 保存逻辑结果, i1 -> i32

  void translate_instruction(llvm::BasicBlock *bb, ir::Instruction *insn,
                             Function &f) {
    using namespace ir::insns;
    Instruction *copy = nullptr;

    TypeCase(alloca, Alloca *, insn) {
      auto dst = alloca->dst;
      assert(dst.type == String);
      reg_type[dst] = alloca->type.get_pointer_type();

      copy = new Alloca{dst, alloca->type};
    }
    else TypeCase(load, Load *, insn) {
      auto addr = load->addr;
      auto dst = load->dst;
      assert(addr.type == String);
      assert(dst.type == Int || dst.type == Float);
      assert(reg_type.at(addr).is_pointer_to_scalar());

      copy = new Load{dst, addr};
    }
    else TypeCase(load_addr, LoadAddr *, insn) {
      auto dst = load_addr->dst;
      assert(dst.type == String);

      auto &var_name = load_addr->var_name;
      if (src.global_vars.count(var_name)) {
        reg_type[dst] = src.global_vars.at(var_name)->type.get_pointer_type();
      } else if (var_name.substr(0, 5) == ".str.") {
        int index = std::stoi(var_name.substr(5));
        assert(index < src.string_table.size());
        // NOTE: 这次是真的String，应该是i8*之类的
        // TODO
        reg_type[dst] = Type{String};
      } else {
        assert(false);
      }

      copy = new LoadAddr{dst, var_name};
    }
    else TypeCase(load_imm, LoadImm *, insn) {
      auto dst = load_imm->dst;
      copy = new LoadImm{dst, load_imm->imm};
    }
    else TypeCase(store, Store *, insn) {
      auto addr = store->addr;
      auto val = store->val;
      assert(reg_type.at(addr).is_pointer_to_scalar());
      copy = new Store{addr, val};
    }
    else TypeCase(gep, GetElementPtr *, insn) {
      auto dst = gep->dst;
      auto base = gep->base;
      assert(dst.type == String && base.type == String);

      std::vector<ir::Reg> indices;
      for (auto r : gep->indices)
        indices.push_back(r);

      auto &ptr_type = reg_type.at(base);
      auto &src_type = gep->type;
      Type actual_ptr_type;
      bool omit_first_index = true;
      int deref_dims; // 相对于actual_ptr_type需要移除的维数

      if (src_type.get_pointer_type() == ptr_type) {
        // 常规数组访问
        actual_ptr_type = ptr_type;
        omit_first_index = true;
        deref_dims = indices.size();
      } else {
        // 应当有以下2种情况:
        // 1. src_type == ptr_type 源操作数类型为指针，实际上是函数数组参数
        // 2. src_type != ptr_type 数组手动初始化发生了强制转换，转换后与1一致
        if (src_type != ptr_type) {
          auto new_base = f.new_reg(String);
          bb->push(new PtrCast{new_base, base, src_type, ptr_type});
          base = new_base;
        }
        actual_ptr_type = src_type;
        omit_first_index = false;
        deref_dims = indices.size() - 1;
      }

      Type dst_type = actual_ptr_type;
      auto pos = dst_type.dims.begin() + 1;
      dst_type.dims.erase(pos, pos + deref_dims);
      reg_type[dst] = std::move(dst_type);

      Type actual_src_type = actual_ptr_type;
      actual_src_type.dims.erase(actual_src_type.dims.begin());

      auto new_gep = new GetElementPtr{dst, std::move(actual_src_type), base,
                                       std::move(indices)};
      new_gep->omit_first_index = omit_first_index;
      copy = new_gep;
    }
    else TypeCase(convert, Convert *, insn) {
      auto dst = convert->dst;
      auto src = convert->src;
      copy = new Convert{dst, src};
    }
    else TypeCase(call, Call *, insn) {
      auto dst = call->dst;

      auto &fn_name = call->func;
      auto &sig = src.functions.count(fn_name) ? src.functions.at(fn_name).sig
                                               : src.lib_funcs.at(fn_name).sig;
      auto &params = sig.param_types;

      std::vector<ir::Reg> args;
      int nr_args = call->args.size();
      int nr_params = params.size();
      for (int i = 0; i < nr_args; ++i) {
        auto arg = call->args[i];
        if (i < nr_params && params[i].is_array() && arg.type == String &&
            reg_type.at(arg) != params[i]) {
          auto new_arg = f.new_reg(String);
          bb->push(new PtrCast{new_arg, arg, params[i], reg_type.at(arg)});
          arg = new_arg;
        }
        args.push_back(arg);
      }

      copy = new Call{dst, call->func, std::move(args)};
    }
    else TypeCase(ret, Return *, insn) {
      std::optional<ir::Reg> ret_val;
      if (ret->val)
        ret_val = ret->val.value();
      copy = new Return{ret_val};
    }
    else TypeCase(unary, Unary *, insn) {
      auto dst = unary->dst;
      auto src = unary->src;

      if (unary->op == UnaryOp::Not) {
        auto tmp = f.new_reg(Int);
        bb->push(new ZeroCmp{tmp, ZeroCmp::Eq, src});
        copy = new IntCast{dst, IntCast::Zext, tmp};
      } else {
        copy = new Unary{dst, unary->op, src};
      }
    }
    else TypeCase(binary, Binary *, insn) {
      auto dst = binary->dst;
      auto src1 = binary->src1;
      auto src2 = binary->src2;
      copy = new Binary{dst, binary->op, src1, src2};
    }
    else TypeCase(jump, Jump *, insn) {
      auto target = bb_map.at(jump->target);
      copy = new SimpleJump{target};
    }
    else TypeCase(br, Branch *, insn) {
      auto src = br->val;
      if (!i1_regs.count(src)) {
        auto tmp = f.new_reg(Int);
        bb->push(new ZeroCmp{tmp, ZeroCmp::Ne, src});
        src = tmp;
      }

      auto true_target = bb_map.at(br->true_target);
      auto false_target = bb_map.at(br->false_target);
      copy = new SimpleBranch{src, true_target, false_target};
    }
    else TypeCase(phi, Phi *, insn) {
      auto dst = phi->dst;
      auto s_phi = new SimplePhi{dst};
      for (auto &[bb, reg] : phi->incoming)
        s_phi->srcs[bb_map.at(bb)] = reg;
      copy = s_phi;
    }
    else {
      assert(false);
    }

    bb->push(copy);

    // 对于cmp，生成i32结果
    TypeCase(binary, Binary *, insn) {
      switch (binary->op) {
      case BinaryOp::Eq:
      case BinaryOp::Neq:
      case BinaryOp::Lt:
      case BinaryOp::Leq:
      case BinaryOp::Gt:
      case BinaryOp::Geq: {
        auto dst = f.new_reg(Int);
        bb->push(new IntCast{dst, IntCast::Zext, binary->dst});
        i1_regs[binary->dst] = dst;
      }
      default:
        break;
      }
    }
  }

  void rename_registers(llvm::Function &f) {
    std::unordered_map<ir::Reg, ir::Reg> reg_map;

    int nr_params = f.sig.param_types.size();
    for (int i = 0; i < nr_params; ++i) {
      auto &type = f.sig.param_types[i];
      auto scalar_type = type.is_array() ? String : type.base_type;
      ir::Reg old_param{scalar_type, i + 1};
      ir::Reg new_param{scalar_type, i};

      reg_map[old_param] = new_param;
    }

    int nr_regs = nr_params;
    for (auto &bb : f.bbs) {
      for (auto &insn : bb->insns) {
        TypeCase(output, ir::insns::Output *, insn.get()) {
          auto old_dst = output->dst;
          if (output->is<ir::insns::Call>() && old_dst.type == String) {
            reg_map[old_dst] = ir::Reg{String, -1}; // 此寄存器实际并不存在
            continue;
          }

          auto new_dst = ir::Reg{old_dst.type, nr_regs++};
          reg_map[old_dst] = new_dst;
        }
      }
    }

    f.nr_regs = nr_regs;
    for (auto &bb : f.bbs) {
      for (auto &insn : bb->insns) {
        auto reg_ptrs = insn->reg_ptrs();
        for (auto p : reg_ptrs) {
          *p = reg_map.at(*p);
        }
      }
    }
  }

  void translate_function(llvm::Function &df, const ir::Function &sf) {
    bb_map.clear();
    reg_type.clear();
    i1_regs.clear();

    df.name = sf.name;
    df.sig = sf.sig;
    df.nr_regs = sf.nr_regs;

    // 标注参数类型
    for (size_t i = 0; i < sf.sig.param_types.size(); ++i) {
      auto &type = sf.sig.param_types[i];
      auto scalar_type = type.is_array() ? String : type.base_type;
      ir::Reg param{scalar_type, int(i + 1)};
      if (scalar_type == String)
        reg_type[param] = type;
    }

    for (auto &sbb : sf.bbs) {
      auto dbb = new BasicBlock;
      dbb->label = sbb->label;
      bb_map[sbb.get()] = dbb;
      df.bbs.emplace_back(dbb);
    }

    if (sf.cfg) {
      sf.cfg->compute_rpo();
      for (auto sbb : sf.cfg->rpo) {
        for (auto &insn : sbb->insns)
          translate_instruction(bb_map.at(sbb), insn.get(), df);
      }
    } else {
      auto it = df.bbs.begin();
      for (auto &sbb : sf.bbs) {
        for (auto &insn : sbb->insns)
          translate_instruction(it->get(), insn.get(), df);
        ++it;
      }
    }

    rename_registers(df);
  }

public:
  ProgramTranslator(llvm::Program &dst, const ir::Program &src)
      : dst{dst}, src{src} {}

  void translate() {
    dst.global_vars = src.global_vars;
    dst.string_table = src.string_table;
    for (auto &[name, lib_f] : src.lib_funcs)
      dst.lib_funcs[name] = lib_f;
    for (auto &[name, src_f] : src.functions) {
      auto &dst_f = dst.functions[name];
      translate_function(dst_f, src_f);
    }
  }
};

Program::Program()
    : lib_func_decls{"declare i32 @getint()",
                     "declare i32 @getch()",
                     "declare float @getfloat()",
                     "declare void @putint(i32)",
                     "declare void @putch(i32)",
                     "declare void @putfloat(float)",
                     "declare i32 @getarray(i32*)",
                     "declare i32 @getfarray(float*)",
                     "declare void @putarray(i32, i32*)",
                     "declare void @putfarray(i32, float*)",
                     "declare void @putf(i8*, i32, ...)"} {}

// 需要重写的特殊指令(如 getelementptr)
bool Program::emit_special_instruction(std::ostream &os,
                                       const Instruction &insn) const {
  using namespace ir::insns;
  using namespace ir;

  auto ins = &insn;
  TypeCase(call, const Call *, ins) {
    if (call->dst.id >= 0)
      call->write_reg(os);
    auto &sig = get_signature(call->func);
    os << "call " << type_string(sig.ret_type) << ' ' << var_name(call->func)
       << '(';
    for (size_t i = 0; i < call->args.size(); ++i) {
      if (i != 0)
        os << ", ";
      os << type_string(sig.param_types[i]) << " " << reg_name(call->args[i]);
    }
    os << ')';
    return true;
  }
  TypeCase(load_imm, const LoadImm *, ins) {
    load_imm->write_reg(os);
    if (load_imm->dst.type != Float) {
      os << "add i32 " << load_imm->imm << ", 0";
    } else {
      os << "fadd float " << load_imm->imm << ", 0.0";
    }
    return true;
  }
  TypeCase(load_addr, const LoadAddr *, ins) {
    auto &var = global_vars.at(load_addr->var_name);
    auto ts = type_string(var->type);
    load_addr->write_reg(os) << "getelementptr " << ts << ", " << ts << "* "
                             << var_name(load_addr->var_name) << ", i32 0";
    return true;
  }
  TypeCase(unary, const Unary *, ins) {
    if (unary->op == UnaryOp::Sub) {
      unary->write_reg(os);
      if (unary->src.type == Float) {
        os << "fneg float " << reg_name(unary->src);
      } else {
        os << "sub i32 0, " << reg_name(unary->src);
      }
      return true;
    }
    return false;
  }
  TypeCase(gep, const GetElementPtr *, ins) {
    auto ts = type_string(gep->type);
    gep->write_reg(os) << "getelementptr " << ts << ", " << ts << "* "
                       << reg_name(gep->base);
    if (gep->omit_first_index)
      os << ", i32 0";
    for (auto r : gep->indices)
      os << ", i32 " << reg_name(r);
    return true;
  }
  return false;
}

void Program::emit_basic_block(std::ostream &os, const BasicBlock &bb) const {
  os << bb.label << ":\n";
  for (auto &insn : bb.insns) {
    os << "  ";
    if (!emit_special_instruction(os, *insn))
      insn->emit(os);
    os << '\n';
  }
}

void Program::emit_function(std::ostream &os, const Function &f) const {
  os << "define " << ir::type_string(f.sig.ret_type) << " "
     << ir::var_name(f.name) << "(";

  auto &types = f.sig.param_types;
  for (size_t i = 0; i < types.size(); ++i) {
    if (i != 0)
      os << ", ";
    os << ir::type_string(types[i]) << " %" << std::to_string(i);
  }
  os << ") {\n";
  int i = 0;
  for (auto &bb : f.bbs) {
    if (i != 0)
      os << "\n";
    emit_basic_block(os, *bb);
    ++i;
  }
  os << "}\n\n";
}

void Program::emit(std::ostream &os) const {
  emit_header(os);
  os << '\n';
  for (auto &[_, f] : functions)
    emit_function(os, f);
}

void Program::emit_header(std::ostream &os) const {
  for (auto &func_decl : lib_func_decls)
    os << func_decl << '\n';
  os << '\n';
}

std::unique_ptr<Program> translate(const ir::Program &ir_program) {
  auto program = std::make_unique<Program>();
  ProgramTranslator translator{*program, ir_program};
  translator.translate();
  return std::move(program);
};

void emit_global(std::ostream &os, const ir::Program &ir_program) {
  for (auto &[name, var] : ir_program.global_vars)
    ir::emit_global_var(os, name, var.get());
}

} // namespace llvm

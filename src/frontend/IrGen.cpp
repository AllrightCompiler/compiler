#include "frontend/IrGen.hpp"
#include "frontend/ast.hpp"

#include "frontend/Typer.hpp"

#include "common/ir.hpp"

#include <cassert>

namespace frontend {

using namespace ir;

IrGen::IrGen() : program{new Program} {
  local_regs = global_regs = 0;
  cur_func = nullptr;
  init_bb = cur_bb = new BasicBlock;
  init_bb->label = ".init.BB";

  auto &lib = program->lib_funcs;
  lib["getint"].sig = {.ret_type = Int, .param_types = {}};
  lib["getch"].sig = {.ret_type = Int, .param_types = {}};
  lib["getfloat"].sig = {.ret_type = Float, .param_types = {}};
  lib["getarray"].sig = {.ret_type = Int,
                         .param_types = {Type{Int, std::vector<int>{0}}}};
  lib["getfarray"].sig = {.ret_type = Int,
                          .param_types = {Type{Float, std::vector<int>{0}}}};
  lib["putint"].sig = {.ret_type = std::nullopt, .param_types = {Type{Int}}};
  lib["putch"].sig = {.ret_type = std::nullopt, .param_types = {Type{Int}}};
  lib["putfloat"].sig = {.ret_type = std::nullopt,
                         .param_types = {Type{Float}}};
  lib["putarray"].sig = {
      .ret_type = std::nullopt,
      .param_types = {Type{Int}, Type{Int, std::vector<int>{0}}}};
  lib["putfarray"].sig = {
      .ret_type = std::nullopt,
      .param_types = {Type{Int}, Type{Float, std::vector<int>{0}}}};
  lib["putf"].sig = {.ret_type = std::nullopt,
                     .param_types = {Type{String}, Type{Int}}};
  lib["__builtin_array_init"].sig = {
      .ret_type = std::nullopt,
      .param_types = {Type{Int, std::vector<int>{0}}, Type{Int}}};
}

BasicBlock *IrGen::new_bb() {
  assert(cur_func);
  auto bb = new BasicBlock;
  int id = cur_func->bbs.size();
  bb->label = "B" + std::to_string(id);
  bb->func = cur_func;
  cur_func->bbs.emplace_back(bb);
  return bb;
}

Reg IrGen::scalar_cast(Reg src, ScalarType dst_type) {
  assert(dst_type == Int || dst_type == Float);
  if (src.type != dst_type) {
    Reg converted = new_reg(dst_type);
    emit(new insns::Convert{converted, src});
    return converted;
  }
  return src;
}

void IrGen::visit_compile_unit(const ast::CompileUnit &node) {
  for (auto &child : node.children()) {
    if (child.index() == 0) {
      auto &decl = std::get<std::unique_ptr<ast::Declaration>>(child);
      visit_declaration(*decl);
    } else {
      auto &func = std::get<std::unique_ptr<ast::Function>>(child);
      visit_function(*func);
    }
  }

  // 将init_bb包装成.init函数
  init_bb->push_back(new insns::Return{std::nullopt});
  auto &init_fn = program->functions[".init"];
  init_fn.name = ".init";
  init_fn.bbs.emplace_back(init_bb);
  init_bb->func = &init_fn;
  init_fn.nr_regs = global_regs;
}

void IrGen::visit_declaration(const ast::Declaration &node) {
  auto &name = node.ident().name();
  auto &var = node.var;
  auto &var_type = var->type;
  bool is_global = cur_func == nullptr;

  Reg reg;
  mem_vars[var.get()].global = is_global;
  if (!is_global) {
    reg = new_reg(String);
    mem_vars[var.get()].reg = reg;
    emit(new insns::Alloca{reg, var_type});
  } else { // global
    program->global_vars[name] = var;
  }

  auto &initializer = node.init();
  if (initializer) {
    auto &value = initializer->value();
    if (is_global) {
      // load base address of global variable into reg
      reg = new_reg(String);
      emit(new insns::LoadAddr{reg, name});
    }

    if (var_type.is_array()) {
      bool zeroed = false;
      int nr_elems = var_type.nr_elems();

      // 对较大的局部数组直接调用内置清零初始化函数
      if (nr_elems > 16 && !is_global) {
        Reg n = new_reg(Int);
        emit(new insns::LoadImm{n, nr_elems});
        emit(
            new insns::Call{new_reg(String), "__builtin_array_init", {reg, n}});
        zeroed = true;
      }

      int index = 0;
      std::set<int> assigned_indices;
      visit_initializer(
          std::get<std::vector<std::unique_ptr<ast::Initializer>>>(value), var,
          reg, 0, index, assigned_indices);

      // 如果非全局变量，在其他索引处填充0
      if (!is_global && !zeroed) {
        Reg zero_reg = new_reg(var_type.base_type);
        ConstValue zero_imm = (var_type.base_type == Float)
                                  ? ConstValue{float(0)}
                                  : ConstValue{0};
        emit(new insns::LoadImm{zero_reg, zero_imm});

        int cur_idx = 0;
        for (int idx : assigned_indices) {
          while (cur_idx < idx) {
            emit_array_init(reg, zero_reg, cur_idx);
            cur_idx++;
          }
          cur_idx = idx + 1;
        }
        while (cur_idx < nr_elems)
          emit_array_init(reg, zero_reg, cur_idx++);
      }
    } else {                         // scalar
      if (!is_global || !var->val) { // 局部变量或初值未在编译期求得
        ast::Expression *expr;
        if (value.index() == 0) {
          expr = std::get<std::unique_ptr<ast::Expression>>(value).get();
        } else {
          auto &init_list =
              std::get<std::vector<std::unique_ptr<ast::Initializer>>>(value);
          expr =
              std::get<std::unique_ptr<ast::Expression>>(init_list[0]->value())
                  .get();
          // TODO: 如果init_list是空的 -> 标量0初始化
        }
        Reg val = scalar_cast(visit_arith_expr(expr), var_type.base_type);
        emit(new insns::Store{reg, val});
      }
    }
  } else {
    if (!is_global && !var_type.is_array()) {
      Reg zero_reg = new_reg(var_type.base_type);
      ConstValue zero_imm =
          (var_type.base_type == Float) ? ConstValue{float(0)} : ConstValue{0};
      emit(new insns::LoadImm{zero_reg, zero_imm});
      emit(new insns::Store{reg, zero_reg});
    }
  }
}

void IrGen::emit_array_init(ir::Reg base_reg, ir::Reg val_reg, int index) {
  Reg idx_reg = new_reg(Int);
  Reg addr_reg = new_reg(String);
  emit(new insns::LoadImm{idx_reg, index});
  emit(new insns::GetElementPtr{
      addr_reg, Type{val_reg.type, std::vector{0}}, base_reg, {idx_reg}});
  emit(new insns::Store{addr_reg, val_reg});
}

void IrGen::visit_initializer(
    const std::vector<std::unique_ptr<ast::Initializer>> &init_list,
    const std::shared_ptr<Var> &var, ir::Reg base_reg, int depth, int &index,
    std::set<int> &assigned_indices) {

  auto &type = var->type;
  int dim_size = 1;
  if (depth > 0) {
    for (int i = depth; i < type.nr_dims(); ++i)
      dim_size *= type.dims[i];
  }
  int padded = index + dim_size;

  auto &val_map = *(var->arr_val);
  for (auto &p_init : init_list) {
    auto &value = p_init->value();
    if (value.index() == 0) {
      // 如果是全局变量且val_map中有对应索引，说明该项的值已在编译期求出，无需再赋值
      if (cur_func || !val_map.count(index)) {
        auto &expr = std::get<std::unique_ptr<ast::Expression>>(value);
        Reg val_reg = scalar_cast(visit_arith_expr(expr), var->type.base_type);
        emit_array_init(base_reg, val_reg, index);
        assigned_indices.insert(index);
      }
      ++index;
    } else {
      auto &sub_list =
          std::get<std::vector<std::unique_ptr<ast::Initializer>>>(value);
      visit_initializer(sub_list, var, base_reg, depth + 1, index,
                        assigned_indices);
    }
  }
  if (index < padded)
    index = padded;
}

void IrGen::visit_function(const ast::Function &node) {
  auto &name = node.ident().name();
  cur_func = &program->functions[name];
  cur_func->name = name;
  cur_bb = new_bb();

  auto &func_sig = cur_func->sig;
  auto &ret_type = node.type();
  if (!ret_type) {
    func_sig.ret_type = std::nullopt;
  } else {
    func_sig.ret_type = ret_type->type();
  }

  // 前几个寄存器编号分配给参数
  // 对于标量参数（int/float），会另外在栈上开空间
  // 数组参数不会退化成指针，可以认为是只读的，直接保存在寄存器中
  int nr_params = node.params().size();
  local_regs = nr_params;
  for (int i = 0; i < nr_params; ++i) {
    auto &param = node.params()[i];
    auto &var = param->var;
    func_sig.param_types.push_back(var->type);

    auto &storage = mem_vars[var.get()];
    storage.global = false;
    if (var->type.is_array())
      storage.reg = Reg{String, i + 1};
    else { // int/float
      Reg addr_reg = new_reg(String), arg_reg = Reg{var->type.base_type, i + 1};
      storage.reg = addr_reg;

      emit(new insns::Alloca{addr_reg, var->type});
      emit(new insns::Store{addr_reg, arg_reg});
    }
  }

  if (name == "main")
    emit(new insns::Call{new_reg(String), ".init", {}});

  visit_statement(*node.body());

  // 总是添加return作为兜底
  emit(new insns::Return{std::nullopt});

  cur_func->nr_regs = local_regs;
  cur_func = nullptr;
  cur_bb = init_bb;
}

void IrGen::visit_statement(const ast::Statement &node) {
  auto stmt = &node;

  TypeCase(expr_stmt, const ast::ExprStmt *, stmt) {
    auto &expr = expr_stmt->expr();
    if (expr)
      visit_arith_expr(expr);
    return;
  }
  TypeCase(assign, const ast::Assignment *, stmt) {
    auto &lhs = assign->lhs();
    auto &rhs = assign->rhs();

    Reg val = scalar_cast(visit_arith_expr(rhs), lhs->type->base_type);
    Reg addr = visit_lvalue(*lhs, true);
    emit(new insns::Store{addr, val});
    return;
  }
  TypeCase(block, const ast::Block *, stmt) {
    for (auto &child : block->children()) {
      if (child.index() == 0) {
        auto &decl = std::get<std::unique_ptr<ast::Declaration>>(child);
        visit_declaration(*decl);
      } else {
        auto &sub_stmt = std::get<std::unique_ptr<ast::Statement>>(child);
        visit_statement(*sub_stmt);
      }
    }
    return;
  }
  TypeCase(if_, const ast::IfElse *, stmt) {
    visit_if(*if_);
    return;
  }
  TypeCase(while_, const ast::While *, stmt) {
    visit_while(*while_);
    return;
  }
  TypeCase(break_, const ast::Break *, stmt) {
    emit(new insns::Jump{break_targets.back()});
    cur_bb = new_bb();
    return;
  }
  TypeCase(continue_, const ast::Continue *, stmt) {
    emit(new insns::Jump{continue_targets.back()});
    cur_bb = new_bb();
    return;
  }
  TypeCase(return_, const ast::Return *, stmt) {
    std::optional<Reg> ret;
    if (return_->res()) {
      Reg val = visit_arith_expr(return_->res());
      ret = scalar_cast(val, cur_func->sig.ret_type.value());
    }
    emit(new insns::Return{ret});
    cur_bb = new_bb();
    return;
  }
}

// 如果求值类型为数组，返回的寄存器中持有数组的首地址；
// 如果求值类型为标量，当`return_addr_when_scalar`为真时返回该元素的地址，否则为值
Reg IrGen::visit_lvalue(const ast::LValue &node, bool return_addr_when_scalar) {
  auto &var = node.var;
  auto &name = node.ident().name();
  auto &storage = mem_vars.find(var.get())->second;

  Reg addr, base = storage.reg;
  if (storage.global) {
    base = new_reg(String);
    emit(new insns::LoadAddr{base, name});
  }

  std::vector<Reg> index_regs;
  auto &indices = node.indices();
  if (!indices.empty()) {
    for (auto &index : indices) {
      index_regs.push_back(scalar_cast(visit_arith_expr(index), Int));
    }
    addr = new_reg(String);
    emit(
        new insns::GetElementPtr{addr, var->type, base, std::move(index_regs)});
  } else {
    addr = base;
  }

  if (!node.type->is_array() && !return_addr_when_scalar) {
    Reg val = new_reg(var->type.base_type);
    emit(new insns::Load{val, addr});
    return val;
  }
  return addr;
}

Reg IrGen::visit_arith_expr(const ast::Expression *expr) {
  TypeCase(int_literal, const ast::IntLiteral *, expr) {
    Reg reg = new_reg(Int);
    emit(new insns::LoadImm{reg, int_literal->value()});
    return reg;
  }
  TypeCase(float_literal, const ast::FloatLiteral *, expr) {
    Reg reg = new_reg(Float);
    emit(new insns::LoadImm(reg, float_literal->value()));
    return reg;
  }
  TypeCase(lvalue, const ast::LValue *, expr) {
    return visit_lvalue(*lvalue, false);
  }
  TypeCase(call, const ast::Call *, expr) {
    auto &callee_name = call->func().name();
    ir::FunctionSignature *sig = nullptr;
    if (program->functions.count(callee_name))
      sig = &(program->functions[callee_name].sig);
    else if (program->lib_funcs.count(callee_name))
      sig = &(program->lib_funcs[callee_name].sig);

    int nr_params = sig->param_types.size();
    int nr_args = call->args().size();
    std::vector<Reg> arg_regs;
    for (int i = 0; i < nr_args; ++i) {
      auto &arg = call->args()[i];
      if (arg.index() == 0) { // expression
        auto &arg_expr = std::get<std::unique_ptr<ast::Expression>>(arg);
        Reg reg = visit_arith_expr(arg_expr);
        if (i < nr_params && !sig->param_types[i].is_array())
          reg = scalar_cast(reg, sig->param_types[i].base_type);
        arg_regs.push_back(reg);
      } else { // string literal
        auto &str_literal = std::get<ast::StringLiteral>(arg);
        int str_id = program->string_table.size();
        program->string_table.push_back(str_literal.value());
        auto str_name = ".str." + std::to_string(str_id);
        Reg reg = new_reg(String);
        emit(new insns::LoadAddr{reg, str_name});
        arg_regs.push_back(reg);
      }
    }

    // int和float即相应类型，剩下的随便写个string
    ScalarType ret_reg_type = sig->ret_type.value_or(String);
    Reg ret_reg = new_reg(ret_reg_type);
    emit(new insns::Call{ret_reg, callee_name, std::move(arg_regs)});
    return ret_reg;
  }
  TypeCase(unary, const ast::UnaryExpr *, expr) {
    auto op = unary->op();
    Reg src = visit_arith_expr(unary->operand());
    switch (op) {
    case UnaryOp::Add:
      return src;
    case UnaryOp::Sub: {
      Reg dst = new_reg(src.type);
      emit(new insns::Unary{dst, op, src});
      return dst;
    }
    case UnaryOp::Not: {
      // int情况下为了保留语义信息，选择直接生成unary not
      // 该IR指令可以后续被翻译为icmp eq 0或反转跳转目标
      // float的比较可能更复杂，也直接生成unary not
      // 这里不进行隐式类型转换
      Reg dst = new_reg(Int);
      emit(new insns::Unary{dst, op, src});
      return dst;
    }
    }
  }
  TypeCase(binary, const ast::BinaryExpr *, expr) {
    auto op = binary->op();
    assert(op != BinaryOp::And && op != BinaryOp::Or);

    Reg src1 = visit_arith_expr(binary->lhs());
    Reg src2 = visit_arith_expr(binary->rhs());
    auto src_type = (src1.type == Float || src2.type == Float) ? Float : Int;
    auto dst_type = binary->type->base_type;

    // WARNING: 小心单精度float操作！
    src1 = scalar_cast(src1, src_type);
    src2 = scalar_cast(src2, src_type);
    Reg dst = new_reg(dst_type);
    emit(new insns::Binary{dst, op, src1, src2});
    return dst;
  }
  assert(false);
}

BranchTargets IrGen::emit_branch(Reg val) {
  assert(cur_func);
  auto br_insn = new insns::Branch{val, nullptr, nullptr};
  emit(br_insn);
  cur_bb = nullptr;
  return BranchTargets{.true_list = {&br_insn->true_target},
                       .false_list = {&br_insn->false_target}};
}

BranchTargets IrGen::visit_logical_expr(const ast::Expression *expr) {
  TypeCase(binary, const ast::BinaryExpr *, expr) {
    auto op = binary->op();
    if (op == BinaryOp::Or) {
      auto cond1_targets = visit_logical_expr(binary->lhs());
      cur_bb = new_bb();
      for (auto p : cond1_targets.false_list)
        *p = cur_bb;

      auto cond2_targets = visit_logical_expr(binary->rhs());
      std::copy(cond1_targets[1].begin(), cond1_targets[1].end(),
                std::back_inserter(cond2_targets.true_list));
      return cond2_targets;
    }
    if (op == BinaryOp::And) {
      auto cond1_targets = visit_logical_expr(binary->lhs());
      cur_bb = new_bb();
      for (auto p : cond1_targets.true_list)
        *p = cur_bb;

      auto cond2_targets = visit_logical_expr(binary->rhs());
      std::copy(cond1_targets[0].begin(), cond1_targets[0].end(),
                std::back_inserter(cond2_targets.false_list));
      return cond2_targets;
    }
  }
  return emit_branch(visit_arith_expr(expr));
}

void IrGen::visit_if(const ast::IfElse &node) {
  auto &otherwise = node.otherwise();

  auto true_bb = new_bb();
  auto false_bb = otherwise ? new_bb() : nullptr;
  auto next_bb = new_bb();
  if (!otherwise)
    false_bb = next_bb;

  auto br_targets = visit_logical_expr(node.cond());
  br_targets.resolve(true_bb, false_bb);

  cur_bb = true_bb;
  visit_statement(*node.then());
  emit(new insns::Jump{next_bb});
  if (otherwise) {
    cur_bb = false_bb;
    visit_statement(*otherwise);
    emit(new insns::Jump{next_bb});
  }
  cur_bb = next_bb;
}

void IrGen::visit_while(const ast::While &node) {
  auto cond_bb = new_bb();
  auto body_bb = new_bb();
  auto next_bb = new_bb();

  emit(new insns::Jump{cond_bb});
  cur_bb = cond_bb;
  auto br_targets = visit_logical_expr(node.cond());
  br_targets.resolve(body_bb, next_bb);

  cur_bb = body_bb;
  break_targets.push_back(next_bb);
  continue_targets.push_back(cond_bb);
  visit_statement(*node.body());
  auto body_br_targets = visit_logical_expr(node.cond());
  body_br_targets.resolve(body_bb, next_bb);
  continue_targets.pop_back();
  break_targets.pop_back();
  cur_bb = next_bb;
}

} // namespace frontend

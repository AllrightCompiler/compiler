#include "frontend/Typer.hpp"
#include "frontend/ast.hpp"

#include "common/common.hpp"
#include "common/errors.hpp"

#include <cassert>
// #include <cstdio>

namespace frontend {

Typer::Typer() {
  auto &funcs = sym_tab.lib_funcs;
  funcs["getint"] = {.ret_type = Int, .param_types = {}, .variadic = false};
  funcs["getch"] = {.ret_type = Int, .param_types = {}, .variadic = false};
  funcs["getfloat"] = {.ret_type = Float, .param_types = {}, .variadic = false};
  funcs["getarray"] = {.ret_type = Int,
                       .param_types = {Type{Int, std::vector<int>{0}}},
                       .variadic = false};
  funcs["getfarray"] = {.ret_type = Int,
                        .param_types = {Type{Float, std::vector<int>{0}}},
                        .variadic = false};
  funcs["putint"] = {
      .ret_type = std::nullopt, .param_types = {Type{Int}}, .variadic = false};
  funcs["putch"] = {
      .ret_type = std::nullopt, .param_types = {Type{Int}}, .variadic = false};
  funcs["putfloat"] = {.ret_type = std::nullopt,
                       .param_types = {Type{Float}},
                       .variadic = false};
  funcs["putarray"] = {
      .ret_type = std::nullopt,
      .param_types = {Type{Int}, Type{Int, std::vector<int>{0}}},
      .variadic = false};
  funcs["putfarray"] = {
      .ret_type = std::nullopt,
      .param_types = {Type{Int}, Type{Float, std::vector<int>{0}}},
      .variadic = false};
  funcs["putf"] = {.ret_type = std::nullopt,
                   .param_types = {Type{String}, Type{Int}},
                   .variadic = true};
}

void Typer::visit_compile_unit(const ast::CompileUnit &node) {
  for (auto &child : node.children()) {
    if (child.index() == 0) {
      auto &decl = std::get<std::unique_ptr<ast::Declaration>>(child);
      visit_declaration(*decl);
    } else {
      auto &func = std::get<std::unique_ptr<ast::Function>>(child);
      visit_function(*func);
    }
  }
}

void Typer::visit_declaration(const ast::Declaration &node) {
  auto &name = node.ident().name();
  if (frontend::lookup_var(sym_tab.current_vars(), name) != nullptr)
    throw CompileError{"identifier already defined"};

  Type tp = parse_type(node.type());
  tp.is_const = node.const_qualified();

  std::optional<ConstValue> init_val;
  std::map<int, ConstValue> *arr_val = nullptr;
  auto &initializer = node.init();
  if (initializer) {
    auto &value = initializer->value();
    if (value.index() == 0) {
      // scalar initializer
      auto &expr = std::get<std::unique_ptr<ast::Expression>>(value);
      init_val = parse_scalar_init(expr, tp);
    } else {
      // array initializer
      auto &init_list =
          std::get<std::vector<std::unique_ptr<ast::Initializer>>>(value);

      // float a = {};
      if (!tp.is_array()) { // scalar case
        if (init_list.size() > 1)
          throw CompileError{"invalid initializer for scalar type"};
        if (init_list.size() == 0) {
          init_val = implicit_cast(tp.base_type, ConstValue{0});
        } else { // init_list.size() == 1
          auto &expr =
              std::get<std::unique_ptr<ast::Expression>>(init_list[0]->value());
          init_val = parse_scalar_init(expr, tp);
        }
      } else {         // real array case
        int index = 0; // index of array
        arr_val = new std::map<int, ConstValue>{};
        visit_initializer(init_list, tp, 0, *arr_val, index);
      }
    }
  }

  // if (initializer)
  //   printf("var name: %s value: %d\n", name.c_str(),
  //   init_val.iv);
  // if (arr_val) {
  //   printf("array var: %s\n", name.c_str());
  //   for (auto &[i, v] : *arr_val) {
  //     printf("    [%d] = %d\n", i, v.iv);
  //   }
  // }
  auto var = std::make_shared<Var>(std::move(tp), std::move(init_val));
  if (arr_val)
    var->arr_val.reset(arr_val);
  node.var = var; // copy
  sym_tab.add(name, std::move(var));
}

void Typer::visit_initializer(
    const std::vector<std::unique_ptr<ast::Initializer>> &init_list,
    const Type &type, int depth, std::map<int, ConstValue> &arr_val,
    int &index) {

  int dim_size = 1;
  if (depth > 0) {
    for (int i = depth; i < type.nr_dims(); ++i)
      dim_size *= type.dims[i];
  }
  int padded = index + dim_size;

  for (auto &p_init : init_list) {
    auto &value = p_init->value();
    if (value.index() == 0) {
      // new scalar entry
      auto &expr = std::get<std::unique_ptr<ast::Expression>>(value);
      if (auto val = parse_scalar_init(expr, type))
        arr_val[index] = val.value();
      ++index;
    } else {
      auto &sub_list =
          std::get<std::vector<std::unique_ptr<ast::Initializer>>>(value);
      visit_initializer(sub_list, type, depth + 1, arr_val, index);
    }
  }
  if (index < padded)
    index = padded;
}

void Typer::visit_function(const ast::Function &node) {
  auto &name = node.ident().name();
  if (sym_tab.functions.count(name) || sym_tab.lib_funcs.count(name))
    throw CompileError{"function already defined"};

  auto &func = sym_tab.functions[name];
  sym_tab.cur_func = &func;

  auto &ret_type = node.type();
  if (!ret_type) { // nullptr -> void
    func.ret_type = std::nullopt;
  } else {
    func.ret_type = ret_type->type();
  }

  for (auto &param : node.params()) {
    auto &param_name = param->ident().name();
    auto param_type = parse_type(param->type());
    func.param_names.push_back(param_name);
    func.param_types.push_back(param_type);

    auto var = std::make_shared<Var>(std::move(param_type));
    param->var = var; // copy
    func.scopes.add(param_name, std::move(var));
  }

  visit_statement(*node.body());
  sym_tab.cur_func = nullptr;
}

void Typer::visit_statement(const ast::Statement &node) {
  auto stmt = &node;
  auto &scopes = sym_tab.cur_func->scopes;

  TypeCase(expr_stmt, const ast::ExprStmt *, stmt) {
    visit_expr(expr_stmt->expr());
    return;
  }
  TypeCase(assign, const ast::Assignment *, stmt) {
    auto &lhs = assign->lhs();
    auto &rhs = assign->rhs();
    auto t1 = visit_expr(lhs.get());
    auto t2 = visit_expr(rhs.get());

    if (lhs->var->type.is_const)
      throw CompileError{"cannot assign a const value"};
    if (!t1 || !t2)
      throw CompileError{"invalid binary operation on void type"};
    if (t1->is_array() || t2->is_array())
      throw CompileError{"invalid binary operation on array type"};
    // 这里假设剩下的是int或float，它们总是兼容的
    return;
  }
  TypeCase(block, const ast::Block *, stmt) {
    scopes.open();
    for (auto &child : block->children()) {
      if (child.index() == 0) {
        auto &decl = std::get<std::unique_ptr<ast::Declaration>>(child);
        visit_declaration(*decl);
      } else {
        auto &sub_stmt = std::get<std::unique_ptr<ast::Statement>>(child);
        visit_statement(*sub_stmt);
      }
    }
    scopes.close();
    return;
  }
  TypeCase(if_, const ast::IfElse *, stmt) {
    auto t = visit_expr(if_->cond());
    if (!t || t->is_array())
      throw CompileError{"invalid type for condition expression"};
    visit_statement(*if_->then());
    visit_statement(*if_->otherwise());
    return;
  }
  TypeCase(while_, const ast::While *, stmt) {
    auto t = visit_expr(while_->cond());
    if (!t || t->is_array())
      throw CompileError{"invalid type for condition expression"};
    scopes.open(true);
    visit_statement(*while_->body());
    scopes.close();
    return;
  }
  TypeCase(break_, const ast::Break *, stmt) {
    if (!scopes.nearest_loop())
      throw CompileError{"not in a loop"};
    return;
  }
  TypeCase(continue_, const ast::Continue *, stmt) {
    if (!scopes.nearest_loop())
      throw CompileError{"not in a loop"};
    return;
  }
  TypeCase(return_, const ast::Return *, stmt) {
    auto &res = return_->res();
    auto func = sym_tab.cur_func;
    if (!res) { // 无实际返回值 (return;)
      if (func->ret_type)
        throw CompileError{"function should return a value"};
      return;
    }

    auto t = visit_expr(return_->res());
    if (!func->ret_type && t.has_value())
      throw CompileError{"function does return a value"};
    if (t.has_value() && t->is_array())
      throw CompileError{"invalid return value"};
    // 这里假设剩下的是int或float，它们总是兼容的
    return;
  }
}

// 注意：并没有递归挂载符号变量
void Typer::attach_symbol(const ast::LValue &node) {
  if (node.var) // already assigned, skip
    return;

  auto &name = node.ident().name();
  auto &var = sym_tab.get_var(name);
  if (!var)
    throw CompileError{"undefined symbol"};
  node.var = var; // copy
}

std::optional<ConstValue> Typer::eval(const ast::Expression *node) {
  TypeCase(int_literal, const ast::IntLiteral *, node)
    return ConstValue{int_literal->value()};
  TypeCase(float_literal, const ast::FloatLiteral *, node)
    return ConstValue{float_literal->value()};

  // no constpexr functions
  TypeCase(call, const ast::Call *, node)
    return std::nullopt;
  TypeCase(lval, const ast::LValue *, node) {
    attach_symbol(*lval);
    auto &var = lval->var;
    auto &type = var->type;
    if (!type.is_const)
      return std::nullopt;
    if (type.is_array()) {
      auto &subscripts = lval->indices();
      if (type.nr_dims() != subscripts.size())
        return std::nullopt;

      // evaluate from right to left
      auto opt_val = eval(subscripts.back());
      if (!opt_val)
        return std::nullopt;
      int index = implicit_cast(Int, opt_val.value()).iv;
      if (index < 0 || index >= type.dims.back())
        throw CompileError{"array index out of range"};

      int dim_size = 1;
      for (int i = subscripts.size() - 2; i >= 0; --i) {
        auto opt_val = eval(subscripts[i]);
        if (!opt_val)
          return std::nullopt;
        int v = implicit_cast(Int, opt_val.value()).iv;
        if (v < 0 || v >= type.dims[i])
          throw CompileError{"array index out of range"};

        dim_size *= type.dims[i + 1];
        index += dim_size * v;
      }

      assert(var->arr_val);
      auto it = var->arr_val->find(index);
      if (it != var->arr_val->end())
        return it->second;
      else
        return implicit_cast(type.base_type, ConstValue{0});
    }
    return var->val;
  }
  TypeCase(unary, const ast::UnaryExpr *, node) {
    auto op = unary->op();
    auto val = eval(unary->operand());
    if (!val)
      return std::nullopt;
    switch (op) {
    case UnaryOp::Add:
      return val;
      break;
    case UnaryOp::Not:
      if (val->type == Int)
        return ConstValue{!val->iv};
      if (val->type == Float)
        return ConstValue{!val->fv};
      break;
    case UnaryOp::Sub:
      if (val->type == Int)
        return ConstValue{-val->iv};
      if (val->type == Float)
        return ConstValue{-val->fv};
      break;
    }
  }
  TypeCase(binary, const ast::BinaryExpr *, node) {
    auto op = binary->op();
    auto lhs = eval(binary->lhs());
    auto rhs = eval(binary->rhs());
    if (!lhs || !rhs)
      return std::nullopt;
    return frontend::eval(op, lhs.value(), rhs.value());
  }
  return std::nullopt;
}

ConstValue eval(BinaryOp op, const ConstValue &lhs, const ConstValue &rhs) {
  auto as_double = [](const ConstValue &val) -> double {
    return val.type == Int ? val.iv : val.fv;
  };
  auto as_bool = [](const ConstValue &val) -> bool {
    return val.type == Int ? bool(val.iv) : bool(val.fv);
  };
  bool is_float = (lhs.type == Float || rhs.type == Float);

  switch (op) {
  case BinaryOp::Add:
  case BinaryOp::Sub:
  case BinaryOp::Mul:
  case BinaryOp::Div: {
    // TODO: 确保转double运算后再转float 与 直接使用float进行运算 结果一致
    double res, lv = as_double(lhs), rv = as_double(rhs);
    switch (op) {
    case BinaryOp::Add:
      res = lv + rv;
      break;
    case BinaryOp::Sub:
      res = lv - rv;
      break;
    case BinaryOp::Mul:
      res = lv * rv;
      break;
    case BinaryOp::Div:
      // TODO: 应该抛异常吗？
      if (rv == 0)
        throw CompileError{"divide by zero"};
      res = lv / rv;
      break;
    default:
      __builtin_unreachable();
    }
    return is_float ? ConstValue{float(res)} : ConstValue{int(res)};
  }
  // int only
  case BinaryOp::Mod:
    if (is_float)
      throw CompileError{"invalid operands to binary %"};
    if (rhs.iv == 0)
      throw CompileError{"divide by zero"};
    return ConstValue{lhs.iv % rhs.iv};
    break;
  case BinaryOp::And:
  case BinaryOp::Or: {
    bool res, lv = as_bool(lhs), rv = as_bool(rhs);
    if (op == BinaryOp::And)
      res = lv && rv;
    else
      res = lv || rv;
    return ConstValue{int(res)};
  }
  case BinaryOp::Eq:
  case BinaryOp::Neq:
  case BinaryOp::Lt:
  case BinaryOp::Leq:
  case BinaryOp::Gt:
  case BinaryOp::Geq: {
    bool res = false;
    double lv = as_double(lhs), rv = as_double(rhs);
    switch (op) {
    case BinaryOp::Eq:
      res = (lv == rv);
      break;
    case BinaryOp::Neq:
      res = (lv != rv);
      break;
    case BinaryOp::Lt:
      res = (lv < rv);
      break;
    case BinaryOp::Leq:
      res = (lv <= rv);
      break;
    case BinaryOp::Gt:
      res = (lv > rv);
      break;
    case BinaryOp::Geq:
      res = (lv >= rv);
      break;
    default:
      __builtin_unreachable();
    }
    return ConstValue{int(res)};
  }
  default:
    throw CompileError{"invalid BinaryOp"};
  }
  __builtin_unreachable();
}

ConstValue Typer::implicit_cast(ScalarType dst_type, ConstValue val) const {
  if (dst_type == val.type)
    return val;
  if (dst_type == Int && val.type == Float)
    return ConstValue{int(val.fv)};
  if (dst_type == Float && val.type == Int)
    return ConstValue{float(val.iv)};
  throw CompileError{"bad type"};
  __builtin_unreachable();
}

std::optional<ConstValue>
Typer::parse_scalar_init(const std::unique_ptr<ast::Expression> &expr,
                         const Type &type) {
  auto t = visit_expr(expr);
  if (!t || t->is_array())
    throw CompileError{"type mismatch"};

  auto opt_val = eval(expr);
  if (!opt_val && type.is_const)
    throw CompileError{"value cannot be evaluated at compile time"};
  if (opt_val)
    return implicit_cast(type.base_type, opt_val.value());
  return std::nullopt;
}

std::optional<Type> Typer::visit_expr(const ast::Expression *expr) {
  auto t = visit_expr_aux(expr);
  expr->type = t; // copy
  return t;
}

std::optional<Type> Typer::visit_expr_aux(const ast::Expression *expr) {
  TypeCase(_, const ast::IntLiteral *, expr)
    return Type{Int};
  TypeCase(_, const ast::FloatLiteral *, expr)
    return Type{Float};

  TypeCase(lval, const ast::LValue *, expr) {
    attach_symbol(*lval);
    for (auto &index_expr : lval->indices())
      visit_expr(index_expr);

    auto &tp = lval->var->type;
    int deref_dims = lval->indices().size();
    if (deref_dims > tp.nr_dims())
      throw CompileError{"too many subscripts"};

    std::vector<int> dims;
    std::copy(tp.dims.begin() + deref_dims, tp.dims.end(),
              std::back_inserter(dims));
    return Type{tp.base_type, std::move(dims)};
  }
  TypeCase(call, const ast::Call *, expr) {
    auto func_name = call->func().name();
    bool variadic = false;
    std::optional<Type> ret_type;
    std::vector<Type> *p_param_types = nullptr;

    if (sym_tab.functions.count(func_name)) {
      auto &func = sym_tab.functions[func_name];
      ret_type = func.ret_type;
      p_param_types = &func.param_types;
    } else if (sym_tab.lib_funcs.count(func_name)) {
      auto &lib_func = sym_tab.lib_funcs[func_name];
      variadic = lib_func.variadic;
      ret_type = lib_func.ret_type;
      p_param_types = &lib_func.param_types;
    } else
      throw CompileError{"unresolved function call"};

    auto &param_types = *p_param_types;
    int nr_params = param_types.size();
    int nr_args = call->args().size();
    if ((!variadic && nr_args != nr_params) ||
        (variadic && nr_args < nr_params))
      throw CompileError{"wrong number of arguments"};

    for (int i = 0; i < nr_args; ++i) {
      auto &arg = call->args()[i];
      std::optional<Type> t;
      if (arg.index() == 0) {
        auto &arg_expr = std::get<std::unique_ptr<ast::Expression>>(arg);
        t = visit_expr(arg_expr);
        if (!t.has_value())
          throw CompileError{"void-type argument"};
      }

      if (i < nr_params) {
        // 非可变参部分，进行类型检查
        if (param_types[i].base_type == String) {
          if (arg.index() != 1)
            throw CompileError{"argument is not string literal"};
        } else {
          // 注: t为std::nullopt对应一个字符串字面量实参
          if (!t.has_value() || !type_compatible(param_types[i], t.value()))
            throw CompileError{"incompatible argument type"};
        }
      }
    }
    return ret_type;
  }
  TypeCase(unary, const ast::UnaryExpr *, expr) {
    auto t = visit_expr(unary->operand());
    if (!t.has_value())
      throw CompileError{"invalid unary operation on void type"};
    // 一元逻辑非返回int
    if (unary->op() == UnaryOp::Not)
      return Type{Int};
    else
      return t;
  }
  TypeCase(binary, const ast::BinaryExpr *, expr) {
    auto t1 = visit_expr(binary->lhs());
    auto t2 = visit_expr(binary->rhs());
    if (!t1 || !t2)
      throw CompileError{"invalid binary operation on void type"};
    if (t1->is_array() || t2->is_array())
      throw CompileError{"invalid binary operation on array type"};

    auto op = binary->op();
    auto ret_type =
        (t1->base_type == Float || t2->base_type == Float) ? Float : Int;
    if (op == BinaryOp::Mod && ret_type != Int)
      throw CompileError{"invalid operands to binary % (int only)"};

    // 算术操作返回提升后的类型，逻辑操作返回int
    switch (op) {
    case BinaryOp::Add:
    case BinaryOp::Sub:
    case BinaryOp::Mul:
    case BinaryOp::Div:
      return Type{ret_type};
    default: // Mod, Eq, Neq, Lt, Gt, Leq, Geq, And, Or
      return Type{Int};
    }
  }
  return std::nullopt;
}

Type Typer::parse_type(const std::unique_ptr<ast::SysYType> &p) {
  Type tp;
  tp.is_const = false;
  auto ptr = p.get();
  TypeCase(scalar_type, ast::ScalarType *, ptr) {
    tp.base_type = scalar_type->type();
  } else TypeCase(array_type, ast::ArrayType *, ptr) {
    tp.base_type = array_type->base_type();
    if (array_type->first_dimension_omitted())
      tp.dims.push_back(0);
    for (auto &dim : array_type->dimensions()) {
      auto val = eval(dim);
      if (!val)
        throw CompileError{"array dimensions should be compile-time constants"};
      if (val->type != Int)
        throw CompileError{"array dimensions must be integers"};
      if (val->iv <= 0)
        throw CompileError{"array dimensions must be positive"};
      tp.dims.push_back(val->iv);
    }
  } else {
    throw CompileError{"unrecognized type"};
  }
  return tp;
}

// 如果可以隐式转换或传参，返回true
bool type_compatible(const Type &t1, const Type &t2) {
  bool a1 = t1.is_array(), a2 = t2.is_array();
  if (!a1 && !a2) {
    // 这里已经排除掉了字符串字面量，int和float是兼容的
    return true;
  }
  if (t1.base_type != t2.base_type)
    return false;
  if (t1.nr_dims() != t2.nr_dims())
    return false;
  // 数组第一维维度可以不同
  for (int i = 1; i < t1.nr_dims(); ++i)
    if (t1.dims[i] != t2.dims[i])
      return false;
  return true;
}

} // namespace frontend

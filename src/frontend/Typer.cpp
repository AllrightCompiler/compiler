#include "frontend/Typer.hpp"
#include "frontend/ast.hpp"

#include "common/common.hpp"
#include "common/errors.hpp"

#include <cassert>
// #include <cstdio>

namespace frontend {

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
  Type tp;
  tp.is_const = node.const_qualified();
  auto ptr = node.type().get();
  // TODO: refactor -> parse_type
  if (TypeCase(scalar_type, ast::ScalarType *, ptr)) {
    tp.base_type = scalar_type->type();
  } else if (TypeCase(array_type, ast::ArrayType *, ptr)) {
    tp.base_type = array_type->base_type();
    if (array_type->first_dimension_omitted())
      tp.dims.push_back(0);
    for (auto &dim : array_type->dimensions()) {
      auto val = eval(dim);
      if (!val)
        throw CompileError{"array dimensions should be compile-time constants"};
      if (val->type == Float)
        throw CompileError{"array dimensions must be integers"};
      if (val->iv < 0)
        throw CompileError{"array dimensions must be non-negative"};
      tp.dims.push_back(val->iv);
    }
  } else {
    throw CompileError{"bad type"};
  }

  ConstValue init_val;
  std::map<int, ConstValue> *arr_val = nullptr;
  auto &initializer = node.init();
  if (initializer) {
    auto &value = initializer->value();
    if (value.index() == 0) {
      // scalar initializer
      auto &expr = std::get<std::unique_ptr<ast::Expression>>(value);
      parse_scalar_init(expr, tp, init_val);
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
          // TODO: type check
          auto &expr =
              std::get<std::unique_ptr<ast::Expression>>(init_list[0]->value());
          parse_scalar_init(expr, tp, init_val);
        }
      } else {         // real array case
        int index = 0; // index of array
        arr_val = new std::map<int, ConstValue>{};
        visit_initializer(init_list, tp, 0, *arr_val, index);
      }
    }
  }

  // if (initializer)
  //   printf("var name: %s value: %d\n", node.ident().name().c_str(),
  //   init_val.iv);
  // if (arr_val) {
  //   printf("array var: %s\n", node.ident().name().c_str());
  //   for (auto &[i, v] : *arr_val) {
  //     printf("    [%d] = %d\n", i, v.iv);
  //   }
  // }
  auto var = std::make_shared<Var>(std::move(tp), std::move(init_val));
  if (arr_val)
    var->arr_val.reset(arr_val);
  sym_tab.add(node.ident().name(), std::move(var));
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
      ConstValue val;
      if (parse_scalar_init(expr, type, val))
        arr_val[index] = val;
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

void Typer::visit_function(const ast::Function &func) {}

void Typer::visit_lvalue(const ast::LValue &node) {
  if (node.var) // already assigned, skip
    return;

  auto name = node.ident().name();
  auto &var = sym_tab.get_var(name);
  if (!var)
    throw CompileError{"undefined symbol"};
  node.var = var; // copy
}

std::optional<ConstValue> Typer::eval(const ast::Expression *node) {
  if (TypeCase(int_literal, const ast::IntLiteral *, node))
    return ConstValue{int_literal->value()};
  if (TypeCase(float_literal, const ast::FloatLiteral *, node))
    return ConstValue{float_literal->value()};

  // no constpexr functions
  if (TypeCase(call, const ast::Call *, node))
    return std::nullopt;
  if (TypeCase(lval, const ast::LValue *, node)) {
    visit_lvalue(*lval);
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
      int dim_size = 1;
      for (int i = subscripts.size() - 2; i >= 0; --i) {
        auto opt_val = eval(subscripts[i]);
        if (!opt_val)
          return std::nullopt;
        int v = implicit_cast(Int, opt_val.value()).iv;
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
  if (TypeCase(unary, const ast::UnaryExpr *, node)) {
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
  if (TypeCase(binary, const ast::BinaryExpr *, node)) {
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

bool Typer::parse_scalar_init(const std::unique_ptr<ast::Expression> &expr,
                              const Type &type, ConstValue &init_val) {
  auto opt_val = eval(expr);
  if (!opt_val && type.is_const)
    throw CompileError{"value cannot be evaluated at compile time"};
  if (opt_val) {
    init_val = implicit_cast(type.base_type, opt_val.value());
    return true;
  }
  return false;
}

} // namespace frontend

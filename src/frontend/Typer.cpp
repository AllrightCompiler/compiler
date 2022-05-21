#include "frontend/Typer.hpp"
#include "frontend/ast.hpp"

#include "common/common.hpp"
#include "common/errors.hpp"

#include <cassert>
// #include <cstdio>

const std::shared_ptr<Var> &null_var() {
  static std::shared_ptr<Var> null = nullptr;
  return null;
}

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
  if (TypeCase(scalar_type, ast::ScalarType *, ptr)) {
    tp.base_type = scalar_type->type();
  } else if (TypeCase(array_type, ast::ArrayType *, ptr)) {
    tp.base_type = array_type->base_type();
    if (array_type->first_dimension_omitted())
      tp.dims.push_back(0);
    for (auto &dim : array_type->dimensions()) {
      auto val = eval(dim.get());
      if (!val)
        throw CompileError{"array dimensions should be compile-time constants"};
      if (val->type == Float)
        throw CompileError{"array dimensions must be integers"};
      if (val->iv < 0)
        throw CompileError{"array dimensions must be non-negative"};
      tp.dims.push_back(val->iv);
    }
  } else {
    assert(false);
  }

  // TODO: visit ast::Initializer
  ConstValue init_val;
  auto &initializer = node.init();
  if (initializer) {
    auto &value = initializer->value();
    if (value.index() != 0)
      throw CompileError {"not implemented yet"};
    auto expr = std::get<std::unique_ptr<ast::Expression>>(value).get();

    auto opt_val = eval(expr);
    if (!opt_val && tp.is_const)
      throw CompileError {"initializer cannot be evaluated at compile time"};
    if (opt_val)
      init_val = opt_val.value();
  }

  // if (initializer)
  //   printf("var name: %s value: %d\n", node.ident().name().c_str(), init_val.iv);
  auto var = std::make_shared<Var>(std::move(tp), std::move(init_val));
  sym_tab.add(node.ident().name(), std::move(var));
}

void Typer::visit_function(const ast::Function &) {}

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
    return ConstValue {int_literal->value()};
  if (TypeCase(float_literal, const ast::FloatLiteral *, node))
    return ConstValue {float_literal->value()};

  // no constpexr functions
  if (TypeCase(call, const ast::Call *, node))
    return std::nullopt;
  if (TypeCase(lval, const ast::LValue *, node)) {
    visit_lvalue(*lval);
    auto &var = lval->var;
    if (!var->type.is_const)
      return std::nullopt;
    return var->val;
  }
  if (TypeCase(unary, const ast::UnaryExpr *, node)) {
    auto op = unary->op();
    auto val = eval(unary->operand().get());
    if (!val)
      return std::nullopt;
    switch (op) {
    case UnaryOp::Add:
      return val;
      break;
    case UnaryOp::Not:
      if (val->type == Float)
        throw CompileError {"unary not is not supported for float"};
      return ConstValue {!val->iv};
      break;
    case UnaryOp::Sub:
      if (val->type == Int)
        return ConstValue {-val->fv};
      if (val->type == Float)
        return ConstValue {-val->iv};
      break;
    }
  }
  if (TypeCase(binary, const ast::BinaryExpr *, node)) {
    auto op = binary->op();
    auto lhs = eval(binary->lhs().get());
    auto rhs = eval(binary->rhs().get());
    if (!lhs || !rhs)
      return std::nullopt;
    if (op != BinaryOp::Add) // TODO
      throw CompileError {"not implemented yet"};
    if (lhs->type != Int && rhs->type != Int) // TODO
      throw CompileError {"not implemented yet"};
    return ConstValue {lhs->iv + rhs->iv};
  }
  return std::nullopt;
}

} // namespace frontend

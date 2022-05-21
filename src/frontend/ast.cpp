#include "frontend/ast.hpp"
#include "common/utils.hpp"

#include <cassert>
#include <ostream>
#include <string_view>

using namespace frontend::ast;
using std::ostream;

using AstScalarType = frontend::ast::ScalarType;

std::string_view op_string(UnaryOp op) {
  switch (op) {
  case UnaryOp::Add:
    return "+";
  case UnaryOp::Sub:
    return "-";
  case UnaryOp::Not:
    return "!";
  }
  return "<unknown_unary_op>";
}

std::string_view op_string(BinaryOp op) {
  unsigned i = static_cast<unsigned>(op);
  if (op >= BinaryOp::NR_OPS)
    return "<unknown_binary_op>";
  static constexpr std::string_view op_strs[] = {
    "+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "&&", "||"
  };
  return op_strs[i];
}

std::string_view type_string(::ScalarType type) {
  switch (type) {
  case Int:
    return "int";
  case Float:
    return "float";
  default:
    break;
  }
  return "<unknown_scalar_type>";
}

std::string_view type_string(const AstScalarType *type) {
  if (!type)
    return "void";
  return type_string(type->type());
}

ostream &operator<<(ostream &os, const std::unique_ptr<AstScalarType> &type) {
  os << type_string(type.get());
  return os;
}

ostream &operator<<(ostream &os, const Identifier &ident) {
  os << ident.name();
  return os;
}

ostream &operator<<(ostream &os, const ArrayType &type) {
  os << type_string(type.base_type());
  int n_dims = type.dimensions().size();
  if (type.first_dimension_omitted())
    n_dims++;
  for (int i = 0; i < n_dims; ++i)
    os << "[]";
  // TODO: dimension的具体信息
  return os;
}

ostream &operator<<(ostream &os, const std::unique_ptr<SysYType> &type) {
  if (!type) {
    os << "void";
    return os;
  }

  auto raw = type.get();
  if (auto scalar_type = dynamic_cast<AstScalarType *>(raw)) {
    os << type_string(scalar_type);
  } else if (auto array_type = dynamic_cast<ArrayType *>(raw)) {
    os << *array_type;
  } else {
    assert(false);
  }
  return os;
}

ostream &operator<<(ostream &os, const Parameter &param) {
  os << param.type() << ' ' << param.ident();
  return os;
}

void AstScalarType::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << type_string(this) << '\n';
}

void ArrayType::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << *this << '\n';
}

void Identifier::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << *this << '\n';
}

void Parameter::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << *this << '\n';
}

void LValue::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "LValue " << m_ident << '\n';
  // TODO: indices
}

void UnaryExpr::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "UnaryExpr " << op_string(m_op) << '\n';
  assert(m_operand);
  m_operand->print(out, indent + INDENT_LEN);
}

void BinaryExpr::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "BinaryExpr " << op_string(m_op) << '\n';
  assert(m_lhs);
  assert(m_rhs);
  m_lhs->print(out, indent + INDENT_LEN);
  m_rhs->print(out, indent + INDENT_LEN);
}

void IntLiteral::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "IntLiteral " << m_value << '\n';
}

void FloatLiteral::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "FloatLiteral " << m_value << '\n';
}

void StringLiteral::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "StringLiteral " << m_value << '\n';
}

void Call::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Call " << m_func << '\n';
  for (auto &arg : m_args) {
    if (arg.index() == 0) {
      auto &expr = std::get<std::unique_ptr<Expression>>(arg);
      assert(expr);
      expr->print(out, indent + INDENT_LEN);
    } else {
      auto &literal = std::get<StringLiteral>(arg);
      literal.print(out, indent + INDENT_LEN);
    }
  }
}

void ExprStmt::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "ExprStmt\n";
  if (m_expr)
    m_expr->print(out, indent + INDENT_LEN);
}

void Assignment::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Assignment\n";
  assert(m_lhs);
  m_lhs->print(out, indent + INDENT_LEN);
  assert(m_rhs);
  m_rhs->print(out, indent + INDENT_LEN);
}

void Initializer::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Initializer\n";
  if (m_value.index() == 0) {
    auto &expr = std::get<std::unique_ptr<Expression>>(m_value);
    assert(expr);
    expr->print(out, indent + INDENT_LEN);
  } else {
    auto &initializers = std::get<std::vector<std::unique_ptr<Initializer>>>(m_value);
    for (auto &initializer : initializers) {
      assert(initializer);
      initializer->print(out, indent + INDENT_LEN);
    }
  }
}

void Declaration::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Declaration ";
  if (m_const_qualified)
    out << "const ";
  out << m_type << ' ' << m_ident << '\n';
  if (m_init)
    m_init->print(out, indent + INDENT_LEN);
}

void Block::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Block\n";
  for (auto &child : m_children) {
    if (child.index() == 0) {
      auto &decl = std::get<std::unique_ptr<Declaration>>(child);
      assert(decl);
      decl->print(out, indent + INDENT_LEN);
    } else {
      auto &stmt = std::get<std::unique_ptr<Statement>>(child);
      assert(stmt);
      stmt->print(out, indent + INDENT_LEN);
    }
  }
}

void IfElse::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "IfElse\n";
  
  print_indent(out, indent + INDENT_LEN);
  out << "Cond\n";
  assert(m_cond);
  m_cond->print(out, indent + INDENT_LEN * 2);
  
  print_indent(out, indent + INDENT_LEN);
  out << "Then\n";
  assert(m_then);
  m_then->print(out, indent + INDENT_LEN * 2);

  if (m_else) {
    print_indent(out, indent + INDENT_LEN);
    out << "Else\n";
    m_else->print(out, indent + INDENT_LEN * 2);
  }
}

void While::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "While\n";
  
  print_indent(out, indent + INDENT_LEN);
  out << "Cond\n";
  assert(m_cond);
  m_cond->print(out, indent + INDENT_LEN * 2);

  print_indent(out, indent + INDENT_LEN);
  out << "Body\n";
  assert(m_body);
  m_body->print(out, indent + INDENT_LEN * 2);
}

void Break::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Break\n";
}

void Continue::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Continue\n";
}

void Return::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Return\n";
  if (m_res)
    m_res->print(out, indent + INDENT_LEN);
}

void Function::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "Function " << m_type << ' ' << m_ident;
  out << '(';
  for (int i = 0; i < m_params.size(); ++i) {
    if (i)
      out << ", ";
    assert(m_params[i]);
    out << *m_params[i];
  }
  out << ")\n";
  assert(m_body);
  m_body->print(out, indent + INDENT_LEN);
}

void CompileUnit::print(std::ostream &out, unsigned int indent) const {
  print_indent(out, indent);
  out << "CompileUnit\n";
  for (auto &child : m_children) {
    if (child.index() == 0) {
      auto &decl = std::get<std::unique_ptr<Declaration>>(child);
      decl->print(out, indent + INDENT_LEN);
    } else {
      auto &func = std::get<std::unique_ptr<Function>>(child);
      func->print(out, indent + INDENT_LEN);
    }
  }
}

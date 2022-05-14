#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "common/Display.hpp"

namespace frontend {

enum class UnaryOp { Add, Sub, Not };

enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Eq,
  Neq,
  Lt,
  Gt,
  Leq,
  Geq,
  And,
  Or
};

class SysYType : public Display {
public:
  virtual ~SysYType() = default;
};

class ScalarType : public SysYType {
public:
  enum Type { Int, Float };

  explicit ScalarType(Type type);
  virtual ~ScalarType() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Type m_type;
};

class ArrayType : public SysYType {
public:
  ArrayType(ScalarType type, std::vector<unsigned> dimensions,
            bool omit_first_dimension);
  virtual ~ArrayType() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  ScalarType m_type;
  std::vector<unsigned> m_dimensions;
  bool m_omit_first_dimension;
};

class Identifier : public Display {
public:
  explicit Identifier(std::string name);
  virtual ~Identifier() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::string m_name;
};

class Parameter : public Display {
public:
  Parameter(std::unique_ptr<SysYType> type, Identifier ident);
  virtual ~Parameter() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<SysYType> m_type;
  Identifier m_ident;
};

class AstNode : public Display {
public:
  virtual ~AstNode() = default;
};

class NumberLiteral;

class Expression : public AstNode {
public:
  virtual ~Expression() = default;

  NumberLiteral const *value() const;

protected:
  std::unique_ptr<ScalarType> m_type;
  std::unique_ptr<NumberLiteral> m_value;
};

class LValue : public Expression {
public:
  LValue(Identifier ident, std::vector<std::unique_ptr<Expression>> indices);
  virtual ~LValue() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Identifier m_ident;
  std::vector<std::unique_ptr<Expression>> m_indices;
};

class UnaryExpr : public Expression {
public:
  UnaryExpr(UnaryOp op, std::unique_ptr<Expression> operand);
  virtual ~UnaryExpr() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  UnaryOp m_op;
  std::unique_ptr<Expression> m_operand;
};

class BinaryExpr : public Expression {
public:
  BinaryExpr(BinaryOp op, std::unique_ptr<Expression> lhs,
             std::unique_ptr<Expression> rhs);
  virtual ~BinaryExpr() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  BinaryOp m_op;
  std::unique_ptr<Expression> m_lhs, m_rhs;
};

class Literal : public Expression {
public:
  virtual ~Literal() = default;
};

class NumberLiteral : public Literal {
public:
  virtual ~NumberLiteral() = default;
};

class IntLiteral : public NumberLiteral {
public:
  using Value = std::int32_t;
  static_assert(sizeof(Value) == 4);

  virtual ~IntLiteral() = default;

  Value value() const;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Value m_value;
};

class FloatLiteral : public NumberLiteral {
public:
  using Value = float;
  static_assert(sizeof(Value) == 4);

  virtual ~FloatLiteral() = default;

  Value value() const;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Value m_value;
};

class StringLiteral : AstNode {
public:
  using Value = std::string;

  virtual ~StringLiteral() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Value m_value;
};

class Call : public Expression {
public:
  using Argument = std::variant<std::unique_ptr<Expression>, StringLiteral>;

  Call(Identifier func, std::vector<Argument> args);
  virtual ~Call() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Identifier m_func;
  std::vector<Argument> m_args;
};

class Statement : public AstNode {
public:
  virtual ~Statement() = default;
};

class ExprStmt : public Statement {
public:
  explicit ExprStmt(std::unique_ptr<Expression> expr);
  virtual ~ExprStmt() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<Expression> m_expr;
};

class Assignment : public Statement {
public:
  Assignment(std::unique_ptr<LValue> lhs, std::unique_ptr<Expression> rhs);
  virtual ~Assignment() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<LValue> m_lhs;
  std::unique_ptr<Expression> m_rhs;
};

class Initializer : public AstNode {
public:
  using Value = std::variant<std::unique_ptr<Expression>,
                             std::vector<std::unique_ptr<Initializer>>>;

  explicit Initializer(std::unique_ptr<Expression> value);
  explicit Initializer(std::vector<std::unique_ptr<Initializer>> values);
  virtual ~Initializer() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  Value m_value;
};

class Declaration : public AstNode {
public:
  Declaration(std::unique_ptr<SysYType> type, Identifier ident,
              std::unique_ptr<Initializer> init);
  virtual ~Declaration() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<SysYType> m_type;
  Identifier m_ident;
  std::unique_ptr<Initializer> m_init;
};

class Block : public Statement {
public:
  using Child =
      std::variant<std::unique_ptr<Declaration>, std::unique_ptr<Statement>>;

  explicit Block(std::vector<Child> children);
  virtual ~Block() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::vector<Child> m_children;
};

class IfElse : public Statement {
public:
  IfElse(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> then,
         std::unique_ptr<Statement> else_);
  virtual ~IfElse() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<Expression> m_cond;
  std::unique_ptr<Statement> m_then, m_else;
};

class While : public Statement {
public:
  While(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> body);
  virtual ~While() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<Expression> m_cond;
  std::unique_ptr<Statement> m_body;
};

class Break : public Statement {
public:
  virtual ~Break() = default;

  void print(std::ostream &out, unsigned indent) const override;
};

class Continue : public Statement {
public:
  virtual ~Continue() = default;

  void print(std::ostream &out, unsigned indent) const override;
};

class Return : public Statement {
public:
  explicit Return(std::unique_ptr<Expression> res);
  virtual ~Return() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<Expression> m_res;
};

class Function : public AstNode {
public:
  Function(std::unique_ptr<ScalarType> type, Identifier ident,
           std::vector<std::unique_ptr<Parameter>> params,
           std::unique_ptr<Block> body);
  virtual ~Function() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::unique_ptr<ScalarType> m_type;
  Identifier m_ident;
  std::vector<std::unique_ptr<Parameter>> m_params;
  std::unique_ptr<Block> m_body;
};

class CompileUnit : public AstNode {
public:
  using Child =
      std::variant<std::unique_ptr<Declaration>, std::unique_ptr<Function>>;

  explicit CompileUnit(std::vector<Child> children);
  virtual ~CompileUnit() = default;

  void print(std::ostream &out, unsigned indent) const override;

private:
  std::vector<Child> m_children;
};

} // namespace frontend

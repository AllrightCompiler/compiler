#pragma once

#include "frontend/ast.hpp"
#include "frontend/symbol_table.hpp"

#include <optional>

namespace frontend {

class Typer {
  SymbolTable sym_tab;

  void visit_declaration(const ast::Declaration &);
  void visit_function(const ast::Function &);
  void attach_symbol(const ast::LValue &);

  void visit_initializer(
      const std::vector<std::unique_ptr<ast::Initializer>> &init_list,
      const Type &type, int depth, std::map<int, ConstValue> &arr_val,
      int &index);
  
  void visit_statement(const ast::Statement &);

  // return type of expression
  std::optional<Type> visit_expr_aux(const ast::Expression *);
  std::optional<Type> visit_expr(const ast::Expression *);
  std::optional<Type> visit_expr(const std::unique_ptr<ast::Expression> &p) {
    return visit_expr(p.get());
  }

  std::optional<ConstValue> eval(const ast::Expression *);
  std::optional<ConstValue> eval(const std::unique_ptr<ast::Expression> &p) {
    return eval(p.get());
  }

  // implicit cast for scalar const value
  ConstValue implicit_cast(int dst_type, ConstValue val) const;
  // return std::nullopt if the scalar expression cannot be evaluated at compile
  // time
  std::optional<ConstValue>
  parse_scalar_init(const std::unique_ptr<ast::Expression> &expr,
                    const Type &type);

  Type parse_type(const std::unique_ptr<ast::SysYType> &p);

public:
  Typer();
  void visit_compile_unit(const ast::CompileUnit &);
};

ConstValue eval(BinaryOp, const ConstValue &, const ConstValue &);
bool type_compatible(const Type &, const Type &);

} // namespace frontend

#pragma once

#include "frontend/symbol_table.hpp"
#include "frontend/ast.hpp"

#include <optional>

namespace frontend {

class Typer {
  SymbolTable sym_tab;

  void visit_declaration(const ast::Declaration &);
  void visit_function(const ast::Function &);
  void visit_lvalue(const ast::LValue &);

  void visit_initializer(
      const std::vector<std::unique_ptr<ast::Initializer>> &init_list,
      const Type &type, int depth, std::map<int, ConstValue> &arr_val,
      int &index);

  // should use visitor pattern
  std::optional<ConstValue> eval(const ast::Expression *);
  std::optional<ConstValue> eval(const std::unique_ptr<ast::Expression> &p) {
    return eval(p.get());
  }

  // implicit cast for scalar const value
  ConstValue implicit_cast(ScalarType dst_type, ConstValue val) const;
  // evaluated to constant value -> true, otherwise false
  bool parse_scalar_init(const std::unique_ptr<ast::Expression> &expr,
                         const Type &type, ConstValue &init_val);

public:
  void visit_compile_unit(const ast::CompileUnit &);
};

ConstValue eval(BinaryOp, const ConstValue &, const ConstValue &);

} // namespace frontend

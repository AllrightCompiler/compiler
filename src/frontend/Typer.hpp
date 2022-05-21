#pragma once

#include "frontend/ast.hpp"
#include "common/symbol_table.hpp"

#include <optional>

namespace frontend {

class Typer {
  SymbolTable sym_tab;

  void visit_declaration(const ast::Declaration &);
  void visit_function(const ast::Function &);
  void visit_lvalue(const ast::LValue &);

  // should use visitor pattern
  std::optional<ConstValue> eval(const ast::Expression *);

public:
  void visit_compile_unit(const ast::CompileUnit &);
};

}

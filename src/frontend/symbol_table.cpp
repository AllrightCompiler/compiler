#include "frontend/symbol_table.hpp"

namespace frontend {

const std::shared_ptr<Var> &null_var() {
  static std::shared_ptr<Var> null = nullptr;
  return null;
}

}

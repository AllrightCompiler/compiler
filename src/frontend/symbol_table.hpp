#pragma once

#include <string>
#include <unordered_map>

#include "common/common.hpp"

namespace frontend {

using std::string;
using VarTable = std::map<string, std::shared_ptr<Var>>;

inline Var *lookup_var(const VarTable &table, const string &name) {
  auto it = table.find(name);
  return it != table.end() ? it->second.get() : nullptr;
}

const std::shared_ptr<Var> &null_var();

inline const std::shared_ptr<Var> &get_var(const VarTable &table,
                                           const string &name) {
  auto it = table.find(name);
  return it != table.end() ? it->second : null_var();
}

struct Scope {
  VarTable vars;
  bool is_loop;

  Scope(bool is_loop = false) : is_loop{is_loop} {}

  Var *lookup_var(const string &name) const {
    return frontend::lookup_var(vars, name);
  }
  const std::shared_ptr<Var> &get_var(const string &name) const {
    return frontend::get_var(vars, name);
  }
};

class ScopeStack {
  std::vector<Scope> scopes;

public:
  ScopeStack() { open(); }

  void open(bool is_loop = false) { scopes.emplace_back(is_loop); }

  void close() { scopes.pop_back(); }

  Scope &current() { return scopes.back(); }

  Var *lookup(const string &name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto var = it->lookup_var(name);
      if (var)
        return var;
    }
    return nullptr;
  }

  const std::shared_ptr<Var> &get(const string &name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto &var = it->get_var(name);
      if (var)
        return var;
    }
    return null_var();
  }

  void add(const string &name, std::shared_ptr<Var> &&var) {
    current().vars[name] = var;
  }

  void add(const string &name, const std::shared_ptr<Var> &var) {
    current().vars[name] = var;
  }

  const Scope *nearest_loop() const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      if (it->is_loop)
        return &(*it);
    }
    return nullptr;
  }
};

struct SymbolTable {
  struct Function {
    std::optional<int> ret_type; // std::nullopt代表void
    std::vector<Type> param_types;
    std::vector<string> param_names;
    ScopeStack scopes;
  };

  struct LibFunction {
    std::optional<int> ret_type;
    std::vector<Type> param_types;
    bool variadic;
  };

  VarTable global_vars;
  std::unordered_map<string, Function> functions;
  std::unordered_map<string, LibFunction> lib_funcs;
  Function *cur_func;

  SymbolTable() : cur_func{nullptr} {}

  bool is_global_scope() const { return cur_func == nullptr; }

  void add(const string &name, std::shared_ptr<Var> &&var) {
    if (cur_func) {
      cur_func->scopes.add(name, var);
    } else {
      global_vars[name] = var;
    }
  }

  Var *lookup_var(const string &name) {
    if (cur_func) {
      auto var = cur_func->scopes.lookup(name);
      if (var)
        return var;
    }
    return frontend::lookup_var(global_vars, name);
  }

  const std::shared_ptr<Var> &get_var(const string &name) {
    if (cur_func) {
      auto &var = cur_func->scopes.get(name);
      if (var)
        return var;
    }
    return frontend::get_var(global_vars, name);
  }

  VarTable &current_vars() {
    if (cur_func) {
      return cur_func->scopes.current().vars;
    } else {
      return global_vars;
    }
  }
};

} // namespace frontend

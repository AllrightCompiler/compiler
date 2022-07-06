#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cassert>

#define TypeCase(res, type, expr) if (auto res = dynamic_cast<type>(expr))

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
  Or,
  NR_OPS
};

enum ScalarType {
  Int,
  Float,
  String,
};

struct Type {
  ScalarType base_type;
  bool is_const;
  std::vector<int> dims; // 数组第一维可以是0

  int nr_dims() const { return dims.size(); }
  bool is_array() const { return dims.size() > 0; }
  int size() const {
    int size = 4;
    for (int n : dims)
      size *= n;
    return size;
  }

  Type() {}
  Type(ScalarType btype) : base_type{btype}, is_const{false} {}
  Type(ScalarType btype, bool const_qualified)
      : base_type{btype}, is_const{const_qualified} {}
  Type(ScalarType btype, std::vector<int> &&dimensions)
      : base_type{btype}, is_const{false}, dims{std::move(dimensions)} {}
};

// std::variant过于难用，这里直接用union
// 此ConstValue特指标量
struct ConstValue {
  ScalarType type;
  union {
    int iv;
    float fv;
  };

  ConstValue() {}
  ConstValue(int v) : type{Int} { iv = v; }
  ConstValue(float v) : type{Float} { fv = v; }

  bool operator == (const ConstValue &b) const {
    if (type != b.type) return false;
    if (type == Int) return iv == b.iv;
    if (type == Float) return fv == b.fv;
    assert(false);
  }
};

// variable or constant
struct Var {
  Type type;
  std::optional<ConstValue> val;
  std::unique_ptr<std::map<int, ConstValue>>
      arr_val; // index -> value，未记录的项全部初始化为0

  Var() {}
  Var(Type &&type_) : type{type_} {}
  Var(Type &&type_, std::optional<ConstValue> &&value)
      : type{type_}, val{value} {}
};

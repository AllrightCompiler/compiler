#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
  bool operator==(const Type &b) const {
    if (base_type != b.base_type)
      return false;
    if (nr_dims() != b.nr_dims())
      return false;
    for (int i = 0; i < nr_dims(); i++) {
      if (dims[i] != b.dims[i])
        return false;
    }
    return true;
  }
  bool operator!=(const Type &b) const { return !this->operator==(b); }
  Type get_pointer_type() const {
    Type new_type = *this;
    new_type.dims.insert(new_type.dims.begin(), 0);
    return new_type;
  }
  bool is_pointer() const {
    return dims.size() > 0 && dims[0] == 0;
  }
  bool is_pointer_to_scalar() const {
    return dims.size() == 1 && dims[0] == 0;
  }

  Type() {}
  Type(ScalarType btype) : base_type{btype}, is_const{false} {}
  Type(ScalarType btype, bool const_qualified)
      : base_type{btype}, is_const{const_qualified} {}
  Type(ScalarType btype, std::vector<int> dimensions)
      : base_type{btype}, is_const{false}, dims{std::move(dimensions)} {}
  Type(Type type, std::vector<int> dimensions)
      : base_type{type.base_type}, is_const{false}, dims{
                                                        std::move(dimensions)} {
    assert(!type.is_array());
  }
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

  // check ConstValue == 0 / 1
  bool equals(int x) const {
    if (type == Int) {
      return iv == x;
    } else {
      return fv == x;
    }
  }

  bool is_opposite(const ConstValue &b) const {
    if (type != b.type)
      return false;
    if (type == Int)
      return iv + b.iv == 0;
    if (type == Float)
      return fv + b.fv == 0;
    assert(false);
    return false;
  }

  bool operator==(const ConstValue &b) const {
    if (type != b.type)
      return false;
    if (type == Int)
      return iv == b.iv;
    if (type == Float)
      return fv == b.fv;
    assert(false);
    return false;
  }
  bool operator!=(const ConstValue &b) const { return !this->operator==(b); }

  std::string to_string() const {
    if (type == Int)
      return std::to_string(iv);
    if (type == Float)
      return std::to_string(fv);
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
  Var(Type type_) : type{std::move(type_)} {}
  Var(Type type_, std::optional<ConstValue> value)
      : type{std::move(type_)}, val{std::move(value)} {}
};

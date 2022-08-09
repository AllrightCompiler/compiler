#pragma once

#include <functional>
#include <ostream>
#include <utility>

using std::ostream;

inline ostream &error(ostream &os) {
  os << "\033[0;31m[error]\033[0m ";
  return os;
}

inline ostream &warn(ostream &os) {
  os << "\033[0;33m[warn]\033[0m ";
  return os;
}

inline ostream &info(ostream &os) {
  os << "\033[0;32m[info]\033[0m ";
  return os;
}

inline ostream &debug(ostream &os) {
#ifdef DEBUG
  os << "\033[0;34m[debug]\033[0m ";
#endif
  return os;
}

constexpr int INDENT_LEN = 2;

inline void print_indent(std::ostream &os, int indent) {
  for (int i = 0; i < indent; ++i)
    os << ' ';
}

// boost.container_hash.hash
inline std::size_t hash_combine(std::size_t seed, std::size_t const value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  return seed;
}

namespace std {
template <typename First, typename Second> struct hash<pair<First, Second>> {
  size_t operator()(pair<First, Second> const &p) const {
    return hash_combine(hash<First>{}(p.first), hash<Second>{}(p.second));
  }
};
} // namespace std

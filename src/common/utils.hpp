#pragma once

#include <ostream>

using std::ostream;

inline ostream &error(ostream &os) {
  os << "\e[0;31m[error]\033[0m ";
  return os;
}

inline ostream &warn(ostream &os) {
  os << "\e[1;33m[warn]\033[0m ";
  return os;
}

constexpr int INDENT_LEN = 2;

inline void print_indent(std::ostream &os, int indent) {
  for (int i = 0; i < indent; ++i)
    os << ' ';
}


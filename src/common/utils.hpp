#pragma once

#include <ostream>

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

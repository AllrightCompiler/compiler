#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP

#include <algorithm>
#include <string_view>

inline bool has_option(int const argc, char *argv[],
                       std::string_view const option) {
  auto const begin = argv + 1;
  auto const end = argv + argc;
  return std::find(begin, end, option) != end;
}

inline char const *get_option(int const argc, char *argv[],
                              std::string_view const option) {
  auto const begin = argv + 1;
  auto const end = argv + argc;
  auto const iter = std::find(begin, end, option) + 1;
  if (iter < end) {
    return *iter;
  } else {
    return nullptr;
  }
}

#endif // ARGPARSE_HPP

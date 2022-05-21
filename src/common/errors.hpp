#pragma once

#include <stdexcept>

struct CompileError : public std::runtime_error {
  CompileError(const std::string &what) : std::runtime_error{what} {}
};

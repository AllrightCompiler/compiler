#pragma once

#include <ostream>

class Display {
public:
  virtual ~Display() = default;

  virtual void print(std::ostream &out, unsigned indent) const = 0;
};

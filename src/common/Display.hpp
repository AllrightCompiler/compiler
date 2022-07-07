#pragma once

#include <ostream>

class Display {
public:
  virtual ~Display() = default;

  virtual void print(std::ostream &out, unsigned indent) const = 0;

  friend std::ostream &operator<<(std::ostream &out, Display const &self) {
    self.print(out, 0);
    return out;
  }
};

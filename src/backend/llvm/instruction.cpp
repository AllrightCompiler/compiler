#include "backend/llvm/instruction.hpp"
#include "backend/llvm/program.hpp"

namespace llvm {

using namespace ir;

void SimpleJump::emit(std::ostream &os) const {
  os << "br label " << label_name(target->label);
}

void SimpleBranch::emit(std::ostream &os) const {
  os << "br " << type_string(val.type) << " " << reg_name(val) << ", label "
     << label_name(true_target->label) << ", label "
     << label_name(false_target->label);
}

void SimplePhi::emit(std::ostream &os) const {
  write_reg(os) << "phi " << type_string(dst.type);
  for (auto it = srcs.begin(); it != srcs.end(); ++it) {
    if (it != srcs.begin())
      os << ", ";
    os << "[" << reg_name(it->second) << ", " << label_name(it->first->label)
       << "]";
  }
}

} // namespace llvm

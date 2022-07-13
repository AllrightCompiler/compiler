#include "backend/llvm/instruction.hpp"
#include "backend/llvm/program.hpp"

namespace llvm {

using namespace ir;

void SimpleJump::emit(std::ostream &os) const {
  os << "br label " << label_name(target->label);
}

void SimpleBranch::emit(std::ostream &os) const {
  os << "br i1 " << reg_name(val) << ", label "
     << label_name(true_target->label) << ", label "
     << label_name(false_target->label);
}

void SimplePhi::emit(std::ostream &os) const {
  write_reg(os) << "phi " << type_string(dst.type) << ' ';
  for (auto it = srcs.begin(); it != srcs.end(); ++it) {
    if (it != srcs.begin())
      os << ", ";
    os << "[" << reg_name(it->second) << ", " << label_name(it->first->label)
       << "]";
  }
}

void PtrCast::emit(std::ostream &os) const {
  write_reg(os) << "bitcast " << type_string(src_type) << ' ' << reg_name(src)
                << " to " << type_string(dst_type);
}

void IntCast::emit(std::ostream &os) const {
  write_reg(os);
  if (op == Zext)
    os << "zext i1 " << reg_name(src) << " to i32";
  else
    os << "trunc i32 " << reg_name(src) << " to i1";
}

void ZeroCmp::emit(std::ostream &os) const {
  write_reg(os);
  auto ops = (op == Ne) ? "ne" : "eq";
  if (dst.type == Float)
    os << "fcmp o" << ops << " float 0.0, " << reg_name(src);
  else
    os << "icmp " << ops << " i32 0, " << reg_name(src);
}

} // namespace llvm

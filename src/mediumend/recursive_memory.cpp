#include "common/ir.hpp"
#include "mediumend/optimizer.hpp"

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;

using std::unordered_set;

static ir::Program *cur_prog = nullptr;

void recursive_memory(Function *func) {
  auto mem_name = "mem_" + func->name;
  auto raw_entry = func->bbs.front().get();
  auto type = Type{func->sig.ret_type.value(), std::vector<int>{1000}};
  cur_prog->global_vars[mem_name] = std::make_shared<Var>(type);
  BasicBlock *new_entry = new BasicBlock();
  BasicBlock *ret_bb = new BasicBlock();
  new_entry->func = func;
  new_entry->label = "cur_call_entry";
  ret_bb->func = func;
  ret_bb->label = "cur_ret";
  Reg addr_reg = func->new_reg(ScalarType::String);
  Reg gep_reg = func->new_reg(ScalarType::String);
  Reg val_reg = func->new_reg(func->sig.ret_type.value());
  new_entry->push_back(new ir::insns::LoadAddr(addr_reg, mem_name));
  new_entry->push_back(
      new ir::insns::GetElementPtr(gep_reg, type, addr_reg, {Reg(Int, 1)}));
  new_entry->push_back(
      new ir::insns::Load(val_reg, gep_reg));
  new_entry->push_back(new ir::insns::Branch(val_reg, ret_bb, raw_entry));
  
  new_entry->succ.insert(raw_entry);
  raw_entry->prev.insert(new_entry);

  ret_bb->push_back(new ir::insns::Return(val_reg));

  new_entry->succ.insert(ret_bb);
  ret_bb->prev.insert(new_entry);

  for(auto &bb : func->bbs){
    TypeCase(ret, ir::insns::Return *, bb->insns.back().get()){
      auto new_store = new ir::insns::Store(gep_reg, ret->val.value());
      new_store->bb = bb.get();
      new_store->add_use_def();
      bb->insert_before_ter(new_store);
    }
  }

  func->bbs.emplace_front(new_entry);
  func->bbs.emplace_back(ret_bb);
}

void recursive_memory(ir::Program *prog) {
  cur_prog = prog;
  unordered_set<Function *> cursive_calls;
  for (auto &func : prog->functions) {
    if (func.second.sig.param_types.size() != 1 ||
        !func.second.sig.ret_type.has_value()) {
      continue;
    }
    int length = 0;
    for (auto &bb : func.second.bbs) {
      length += bb->insns.size();
      for (auto &insn : bb->insns) {
        TypeCase(call, ir::insns::Call *, insn.get()) {
          if (!prog->functions.count(call->func)) {
            // 库函数直接跳过
            continue;
          }
          if (call->func == func.first) {
            cursive_calls.insert(&func.second);
          }
        }
      }
    }
  }
  for (auto func : cursive_calls) {
    recursive_memory(func);
  }
}

} // namespace mediumend
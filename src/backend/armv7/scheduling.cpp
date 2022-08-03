#include "backend/armv7/instruction.hpp"
#include "backend/armv7/passes.hpp"

namespace armv7 {

// NOTE: 所有普通指令默认不带条件码，不设置flags

FUInfo RType::info() const {
  bool f = dst.is_float();
  switch (op) {
  case Add:
  case Sub:
    if (f)
      return {4, {FU::FpAsimd}}; // F0/F1
    return {1, {FU::Integer}};
  case Mul:
    if (f)
      return {4, {FU::FpAsimd}}; // F0/F1
    return {3, {FU::MultiCycle}};
  case Div:
    // FP divide, S-form: latency 6-11
    if (f)
      return {9, {FU::FpAsimd}}; // F0
    // latency: 4-12
    return {8, {FU::MultiCycle}};
  }
}

FUInfo IType::info() const {
  return {1, {FU::Integer}};
}

FUInfo FullRType::info() const {
  if (s2.is_imm_shift() || s2.is_reg_shift())
    return {2, {FU::MultiCycle}};
  return {1, {FU::Integer}};
}

FUInfo Move::info() const {
  if (is_transfer_vmov()) // vfp <- core / core <- vfp
    return {5, {FU::Load}};
  if (dst.is_float()) // vfp <- vfp
    return {3, {FU::FpAsimd}};
  return {1, {FU::Integer}};
}

// NOTE: 连续的movw/movt可以被合并，时延1周期，吞吐量4条/周期
FUInfo MovW::info() const {
  return {1, {FU::Integer}};
}

FUInfo MovT::info() const {
  return {1, {FU::Integer}};
}

FUInfo LoadAddr::info() const {
  return {1, {FU::Integer}};
}

// TODO: 将浮点vcmp分离
FUInfo Compare::info() const {
  if (s1.is_float())
    return {3, {FU::FpAsimd}}; // F1
  if (s2.is_imm_shift() || s2.is_reg_shift())
    return {2, {FU::MultiCycle}};
  return {1, {FU::Integer}};
}

// NOTE: 给出的是L1-D cache命中时的时延
FUInfo Load::info() const {
  int latency = 4;
  if (dst.is_float())
    ++latency;
  return {latency, {FU::Load}};
}

FUInfo Store::info() const {
  return {1, {FU::Store}};
}

FUInfo FusedMul::info() const {
  return {3, {FU::MultiCycle}};
}

FUInfo Branch::info() const {
  return {1, {FU::Branch}};
}

FUInfo RegBranch::info() const {
  return {1, {FU::Branch}};
}

FUInfo LoadStackAddr::info() const {
  // latency: 1~2
  return {1, {FU::Integer}};
}

FUInfo LoadStack::info() const {
  return {5, {FU::Integer, FU::Load}};
}

FUInfo StoreStack::info() const {
  return {2, {FU::Integer, FU::Store}};
}

FUInfo AdjustSp::info() const {
  return {1, {FU::Integer}};
}

FUInfo Push::info() const {
  int n = (srcs.size() + 1) / 2;
  return {n, {FU::Store, FU::Integer}};
}

FUInfo Pop::info() const {
  int base_latency = 3;
  if (dsts[0].is_float())
    ++base_latency;
  int n = (dsts.size() + 1) / 2;
  return {base_latency + n, {FU::Load, FU::Integer}};
}

FUInfo Call::info() const {
  return {1, {FU::Integer, FU::Branch}};
}

FUInfo Return::info() const {
  return {1, {FU::Branch}};
}

FUInfo CountLeadingZero::info() const {
  return {1, {FU::Integer}};
}

FUInfo PseudoCompare::info() const {
  auto info = cmp->info();
  info.latency += 2;

  auto &units = info.units;
  if (std::find(units.begin(), units.end(), FU::Integer) == units.end())
    units.push_back(FU::Integer);
  return info;
}

FUInfo Convert::info() const {
  return {3, {FU::FpAsimd}}; // F0
}

FUInfo Vneg::info() const {
  return {3, {FU::FpAsimd}};
}

struct DGNode {
  Instruction *ins;
  int degree, delay;
  std::set<DGNode *> pred, succ;
  std::vector<int> dep_seq_no;
};

struct FunctionalUnit {
  FU type;
  DGNode *current;
  int finish_cycle;

  FunctionalUnit(FU type) : type{type}, current{nullptr} {}
};

class ListScheduler {
  BasicBlock &bb;
  std::list<std::unique_ptr<DGNode>> nodes;
  std::list<std::unique_ptr<Instruction>> pinned;
  
  void add_edge(DGNode *from, DGNode *to) {
    from->succ.insert(to);
    to->pred.insert(from);
  }

  void pin_terminators() {
    bool branch = false;
    auto &insns = bb.insns;
    while (!insns.empty()) {
      auto last = std::prev(insns.end());
      auto transfer = [&]() {
        pinned.emplace_front(last->release());
        insns.erase(last);
      };

      if ((*last)->is<Branch>()) {
        transfer();
        branch = true;
        continue;
      }
      if ((*last)->is<Compare>() && branch)
        transfer();
      if ((*last)->is<Return>())
        transfer();
      break;
    }
  }

  void build_dependence_graph() {
    std::map<Reg, std::vector<DGNode *>> users; // 读取 (使用) 此寄存器的DAG节点
    std::map<Reg, DGNode *> producers; // 最后写入 (定义) 此寄存器的DAG节点

    // 需要特殊处理的指令：call, load (含LoadStack), store (含StoreStack), sp调整类 (含push、pop)
    // 所有call, load, store, sp调整禁止跨越函数调用边界
    // load可以重排，但不能跨越store
    // store与store不能重排 (内存WAW)
    // sp调整类指令严格保序
    DGNode *call, *store, *sp_ops;
    call = store = sp_ops = nullptr;
    std::vector<DGNode *> loads;

    for (auto &insn : bb.insns) {
      auto ins = insn.release();
      auto node = new DGNode;
      node->ins = ins;
      nodes.emplace_back(node);

      auto def = ins->def();
      auto use = ins->use();
      for (Reg u : use) {
        // RAW依赖 (true dependence)
        auto it = producers.find(u);
        if (it != producers.end())
          add_edge(it->second, node);
      }
      for (Reg d : def) {
        // WAR依赖 (antidependence)
        auto it = users.find(d);
        if (it != users.end())
          for (auto user : it->second)
            add_edge(user, node);
        // WAW依赖
        if (producers.count(d))
          add_edge(producers.at(d), node);
      }

      for (Reg u : use)
        users[u].push_back(node);
      for (Reg d : def) {
        users.erase(d);
        producers[d] = node;
      }

      // TODO: refactor
      if (ins->is<Call>()) {
        if (call)
          add_edge(call, node);
        if (store) {
          add_edge(store, node);
          store = nullptr;
        }
        if (sp_ops) {
          add_edge(sp_ops, node);
          sp_ops = nullptr;
        }
        for (auto load : loads)
          add_edge(load, node);
        loads.clear();
        call = node;
      } else if (ins->is<Load>() || ins->is<LoadStack>()) {
        if (call)
          add_edge(call, node);
        if (store)
          add_edge(store, node);
        loads.push_back(node);
      } else if (ins->is<Store>() || ins->is<StoreStack>()) {
        if (call)
          add_edge(call, node);
        if (store)
          add_edge(store, node);
        for (auto load : loads)
          add_edge(load, node);
        loads.clear();
        store = node;
      } else if (ins->is<AdjustSp>() || ins->is<Push>() || ins->is<Pop>()) {
        if (call)
          add_edge(call, node);
        if (sp_ops)
          add_edge(sp_ops, node);
        sp_ops = node;
      }
    }
    bb.insns.clear();
  }

  // 计算各节点到基本块结束至少需要的周期数
  // 即各节点到各个根的关键路径长度
  void compute_delay() {
    std::vector<DGNode *> q;
    // 从各个根 (出度为0的节点) 开始
    for (auto &n : nodes) {
      n->delay = 0;
      n->degree = n->succ.size();
      if (n->degree == 0) {
        auto [latency, _] = n->ins->info();
        n->delay = latency;
        q.push_back(n.get());
      }
    }
    while (!q.empty()) {
      auto v = q.back();
      q.pop_back();
      for (auto u : v->pred) {
        auto [latency, _] = u->ins->info();
        u->delay = std::max(u->delay, latency + v->delay);
        if (--u->degree == 0)
          q.push_back(u);
      }
    }
  }

  // priority越大，越优先调度
  double get_priority(const DGNode *node) const {
    constexpr double ALPHA = 0.5;
    int seq_no = bb.insns.size();
    int sum = 0;
    for (int i : node->dep_seq_no)
      sum += seq_no - i;
    return node->delay + ALPHA * sum;
  }

  void schedule() {
    int nr_active = 0;
    std::vector<FunctionalUnit> units{
      {FU::Branch}, {FU::Integer}, {FU::Integer}, {FU::MultiCycle},
      {FU::FpAsimd}, {FU::FpAsimd}, {FU::Load}, {FU::Store},
    };

    std::vector<DGNode *> ready;
    for (auto &n : nodes) {
      n->degree = n->pred.size();
      if (n->degree == 0)
        ready.push_back(n.get());
    }

    int cycle = 0;
    while (!ready.empty() || nr_active > 0) {
      std::sort(ready.begin(), ready.end(), [this](const DGNode *a, const DGNode *b) {
        return get_priority(a) > get_priority(b);
      });

      for (auto it = ready.begin(); it != ready.end();) {
        auto node = *it;
        auto [latency, target_units] = node->ins->info();
        std::vector<FunctionalUnit *> vacant_units;
        for (auto unit_type : target_units) {
          bool found = false;
          for (auto &unit : units) {
            if (unit.type == unit_type && !unit.current) {
              vacant_units.push_back(&unit);
              found = true;
              break;
            }
          }
          if (!found)
            break;
        }

        if (vacant_units.size() < target_units.size()) {
          ++it;
          continue;
        }

        // 更新指令序号
        int seq_no = bb.insns.size();
        for (auto n : node->succ)
          n->dep_seq_no.push_back(seq_no);
        bb.insns.emplace_back(node->ins);

        for (auto pu : vacant_units) {
          ++nr_active;
          pu->current = node;
          pu->finish_cycle = cycle + latency;
        }
        it = ready.erase(it);
      }

      ++cycle;
      std::set<DGNode *> retired;
      for (auto &unit : units) {
        if (unit.finish_cycle == cycle && unit.current) {
          --nr_active;
          retired.insert(unit.current);
          unit.current = nullptr;
        }
      }
      for (auto u : retired)
        for (auto v : u->succ)
          if (--v->degree == 0)
            ready.push_back(v);
    }
  }

public:
  ListScheduler(BasicBlock &bb) : bb{bb} {}
  void operator()() {
    pin_terminators();
    build_dependence_graph();
    compute_delay();
    schedule();
    bb.insns.splice(bb.insns.end(), pinned);
  }
};

void do_list_scheduling(Function &f) {
  for (auto &bb : f.bbs) {
    ListScheduler scheduler{*bb};
    scheduler();
  }
}

} // namespace armv7

#include "backend/armv7/ColoringRegAllocator.hpp"
#include "backend/armv7/arch.hpp"

#include <algorithm>

#include <iostream>

namespace armv7 {

std::set<Reg> ColoringRegAllocator::adjacent(Reg n) const {
  // adjList[n] \ (selectStack ∪ coalescedNodes)
  std::set<Reg> res;
  auto it = adj_list.find(n);
  if (it == adj_list.end())
    return {};
  auto &nodes = it->second;
  auto pred = [this](Reg r) {
    return !coalesced_nodes.count(r) &&
           (std::find(select_stack.begin(), select_stack.end(), r) ==
            select_stack.end());
  };
  std::copy_if(nodes.begin(), nodes.end(), std::inserter(res, res.begin()),
               pred);
  return res;
}

std::set<Move *> ColoringRegAllocator::node_moves(Reg n) const {
  // moveList[n] ∩ (activeMoves ∪ worklistMoves)
  std::set<Move *> res;
  auto it = move_list.find(n);
  if (it == move_list.end())
    return {};
  auto &moves = it->second;
  auto pred = [this](Move *m) {
    return active_moves.count(m) || worklist_moves.count(m);
  };
  std::copy_if(moves.begin(), moves.end(), std::inserter(res, res.begin()),
               pred);
  return res;
}

bool ColoringRegAllocator::move_related(Reg n) const {
  return !(node_moves(n).empty());
}

bool ColoringRegAllocator::ok(Reg t, Reg r) const {
  return degree.at(t) < K || !t.is_virt() || adj_set.count({t, r});
}

bool ColoringRegAllocator::conservative(const std::set<Reg> &nodes) const {
  int k = 0;
  for (Reg n : nodes) {
    int d = degree.at(n);
    if (d >= K)
      ++k;
  }
  return k < K;
}

Reg ColoringRegAllocator::get_alias(Reg n) const {
  if (coalesced_nodes.count(n))
    return get_alias(alias.at(n));
  return n;
}

void ColoringRegAllocator::add_edge(Reg u, Reg v) {
  if (u != v && !adj_set.count({u, v})) {
    adj_set.insert({u, v});
    adj_set.insert({v, u});
    if (u.is_virt()) {
      adj_list[u].insert(v);
      degree[u]++;
    }
    if (v.is_virt()) {
      adj_list[v].insert(u);
      degree[v]++;
    }
  }
}

void ColoringRegAllocator::build() {
  adj_list.clear();
  adj_set.clear();
  degree.clear();

  constexpr int inf = 1 << 30;
  for (int i = 0; i <= 12; ++i)
    degree[Reg{General, i}] = inf;
  degree[Reg{General, lr}] = inf;

  for (int i = 0; i < NR_FPRS; ++i)
    degree[Reg{Fp, i}] = inf;

  for (auto &bb : f->bbs) {
    auto live = bb->live_out;
    for (auto it = bb->insns.rbegin(); it != bb->insns.rend(); ++it) {
      auto ins = it->get();

      auto def = ins->def();
      auto use = ins->use();
      TypeCase(mov, Move *, ins) {
        // NOTE: phi函数解构产生的mov的源寄存器和目的寄存器不应被合并
        if (mov->is_reg_mov() && !f->phi_moves.count(mov)) {
          auto consider = false;
          for (Reg u : use) {
            if (this->reg_filter(u)) {
              consider = true;
              live.erase(u);
            }
          }
          for (Reg n : def) {
            if (this->reg_filter(n)) {
              consider = true;
              move_list[n].insert(mov);
            }
          }
          for (Reg n : use) {
            if (this->reg_filter(n)) {
              move_list[n].insert(mov);
            }
          }
          if (consider) {
            worklist_moves.insert(mov);
          }
        }
      }

      for (Reg d : def) {
        if (this->reg_filter(d)) {
          live.insert(d);
        }
      }
      for (Reg d : def) {
        if (this->reg_filter(d)) {
          for (Reg l : live) {
            add_edge(l, d);
          }
        }
      }
      // live := use(I) ∪ (live - def(I))
      for (Reg d : def) {
        if (this->reg_filter(d)) {
          live.erase(d);
        }
      }
      for (Reg u : use) {
        if (this->reg_filter(u)) {
          live.insert(u);
        }
      }
    }
  }
}

void ColoringRegAllocator::make_worklist() {
  // TODO: 增量式的initial计算 (见论文)
  std::set<Reg> vregs;
  for (auto &bb : f->bbs) {
    for (auto &insn : bb->insns) {
      auto def = insn->def();
      auto use = insn->use();
      for (Reg d : def)
        if (d.is_virt() && this->reg_filter(d))
          vregs.insert(d);
      for (Reg u : use)
        if (u.is_virt() && this->reg_filter(u))
          vregs.insert(u);
    }
  }

  for (Reg n : vregs) {
    // std::cout << "node " << n << " degree=" << degree[n] << " K=" << K <<
    // '\n';
    if (degree[n] >= K)
      spill_worklist.insert(n);
    else if (move_related(n))
      freeze_worklist.insert(n);
    else
      simplify_worklist.insert(n);
  }
}

void ColoringRegAllocator::simplify() {
  auto it = simplify_worklist.begin();
  auto n = *it;
  simplify_worklist.erase(it);
  select_stack.push_back(n);

  // std::cout << "simplify: DecrementDegree for adjacent(" << n << ")\n";
  for (Reg m : adjacent(n))
    decrement_degree(m);
}

void ColoringRegAllocator::decrement_degree(Reg m) {
  // std::cout << "DecrementDegree: test " << m << " degree=" << degree[m] <<
  // '\n';
  if (degree[m]-- == K) {
    auto nodes = adjacent(m);
    nodes.insert(m);
    enable_moves(nodes);
    // std::cout << "DecrementDegree: remove " << m << " from spillWorklist\n";

    spill_worklist.erase(m);
    if (move_related(m))
      freeze_worklist.insert(m);
    else
      simplify_worklist.insert(m);
  }
}

void ColoringRegAllocator::enable_moves(const std::set<Reg> &nodes) {
  for (Reg n : nodes) {
    for (auto m : node_moves(n)) {
      if (active_moves.count(m)) {
        active_moves.erase(m);
        worklist_moves.insert(m);
      }
    }
  }
}

void ColoringRegAllocator::coalesce() {
  auto m = *worklist_moves.begin();
  Reg dst = m->dst;
  Reg src = *(m->use().begin());
  Reg u = get_alias(dst);
  Reg v = get_alias(src);
  if (!v.is_virt())
    std::swap(u, v);

  worklist_moves.erase(m);
  if (u == v) {
    // printf("coalesced move+: ");
    // m->emit(std::cout);
    // printf("\n\n");

    coalesced_moves.insert(m);
    add_work_list(u);
  } else if (!v.is_virt() || adj_set.count({u, v})) {
    constrained_moves.insert(m);
    add_work_list(u);
    add_work_list(v);
  } else {
    bool should_combine;
    auto nodes = adjacent(v);
    if (u.is_virt()) {
      nodes.merge(adjacent(u));
      should_combine = conservative(nodes);
    } else {
      should_combine = std::all_of(nodes.begin(), nodes.end(),
                                   [this, u](Reg t) { return ok(t, u); });
    }

    if (should_combine) {
      // printf("coalesced move+: ");
      // m->emit(std::cout);
      // printf("\n\n");

      coalesced_moves.insert(m);
      combine(u, v);
      add_work_list(u);
    } else {
      active_moves.insert(m);
    }
  }
}

void ColoringRegAllocator::add_work_list(Reg u) {
  if (u.is_virt() && !move_related(u) && degree[u] < K) {
    freeze_worklist.erase(u);
    simplify_worklist.insert(u);
  }
}

void ColoringRegAllocator::combine(Reg u, Reg v) {
  // std::cout << "combine " << u << " & " << v << '\n';

  if (freeze_worklist.count(v))
    freeze_worklist.erase(v);
  else
    spill_worklist.erase(v);
  coalesced_nodes.insert(v);
  alias[v] = u;

  // FIX: nodeMoves -> moveList
  // nodeMoves[u] := nodeMoves[u] ∪ nodeMoves[v]
  auto &moves = move_list[u];
  for (auto m : move_list[v])
    moves.insert(m);

  // std::cout << "combine: DecrementDegree for adjacent(" << v << ")\n";
  for (Reg t : adjacent(v)) {
    add_edge(t, u);
    decrement_degree(t);
  }
  if (degree[u] >= K && freeze_worklist.count(u)) {
    freeze_worklist.erase(u);
    spill_worklist.insert(u);
  }
}

void ColoringRegAllocator::freeze() {
  Reg u = *freeze_worklist.begin();
  freeze_worklist.erase(u);
  simplify_worklist.insert(u);
  freeze_moves(u);
}

void ColoringRegAllocator::freeze_moves(Reg u) {
  for (auto m : node_moves(u)) {
    if (active_moves.count(m))
      active_moves.erase(m);
    else
      worklist_moves.erase(m);
    frozen_moves.insert(m);

    Reg v = (u == m->dst) ? *(m->use().begin()) : m->dst;
    if (node_moves(v).empty() && degree[v] < K) {
      freeze_worklist.erase(v);
      simplify_worklist.insert(v);
    }
  }
}

double ColoringRegAllocator::get_basic_spill_cost(Reg r) {
  if (spilling_regs.count(r))
    return 1e100;

  if (f->reg_val.count(r)) {
    auto &val = f->reg_val.at(r);
    switch (val.index()) {
    case RegValueType::Imm: {
      // 存活到寄存器分配阶段的立即数寄存器不能作为imm8m嵌入指令中
      // 需要至少1条指令重新加载
      uint32_t x = static_cast<uint32_t>(std::get<Imm>(val));
      return x >= 0x10000 ? 2.0 : 1.0;
    }
    case RegValueType::GlobalName:
      return 2.0;
    case RegValueType::StackAddr:
      // 多数栈上地址可以嵌入至ldr/str中，因此认为rematerialize所需代价少于1条指令
      return 0.9;
    default:
      assert(false);
    }
  }
  return 5.0;
}

void ColoringRegAllocator::select_spill() {
  // printf(">>> select_spill candidates: ");
  // int i = 0;
  // for (auto it = spill_worklist.begin(); it != spill_worklist.end();
  //      ++it, ++i) {
  //   if (i)
  //     printf(", ");
  //   std::cout << *it;
  // }
  // printf("\n");

  // TODO: heuristic
  // Reg u = *std::max_element(
  //     spill_worklist.begin(), spill_worklist.end(),
  //     [this](const Reg &a, const Reg &b) { return degree[a] < degree[b]; });

  std::vector<std::pair<double, Reg>> spill_costs;
  for (Reg r : spill_worklist) {
    auto basic_cost = get_basic_spill_cost(r);
    auto cost = basic_cost / degree.at(r);
    spill_costs.push_back({cost, r});
  }
  // auto [_, u] = *std::min_element(spill_costs.begin(), spill_costs.end());
  auto [_, u] = *std::min_element(spill_costs.begin(), spill_costs.end(),
                                  [](const auto &p1, const auto &p2) {
                                    auto [c1, r1] = p1;
                                    auto [c2, r2] = p2;
                                    if (c1 != c2)
                                      return c1 < c2;
                                    // NOTE: cost相同时优先选择编号较小的虚拟寄存器
                                    // 以避免spill最近新生成的虚拟寄存器
                                    // 由于编号实际是负数，这里反过来写
                                    return r2 < r1;
                                  });

  // if (spilling_regs.count(u)) {
  //   std::cerr << "bad choice from " << spill_worklist.size() << " regs\n";
  // }

  // std::cout << "<<< spill target: " << u << " from " << spill_worklist.size()
  // << " candidates\n";
  spill_worklist.erase(u);
  simplify_worklist.insert(u);
  freeze_moves(u);
}

std::map<Reg, int> ColoringRegAllocator::assign_colors() {
  std::map<Reg, int> color;
  while (!select_stack.empty()) {
    Reg n = select_stack.back();
    select_stack.pop_back();

    if (!n.is_virt())
      continue;

    std::set<int> avail_colors;
    if (is_gp_pass) {
      for (int i = 0; i <= 12; ++i)
        avail_colors.insert(r0 + i);
      avail_colors.insert(lr);
    } else {
      for (int i = 0; i < NR_FPRS; ++i)
        avail_colors.insert(s0 + i);
    }

    for (Reg w : adj_list[n]) {
      Reg u = get_alias(w);
      if (!u.is_virt())
        avail_colors.erase(u.id);
      else if (colored_nodes.count(u))
        avail_colors.erase(color[u]);
    }

    if (avail_colors.empty())
      spilled_nodes.insert(n);
    else {
      colored_nodes.insert(n);
      color[n] = *avail_colors.begin();
    }
  }

  for (Reg n : coalesced_nodes) {
    if (n.is_virt()) {
      Reg u = get_alias(n);
      color[n] = u.is_virt() ? color[u] : u.id;
    }
  }

  // printf("color assignment: \n");
  // for (auto &[r, id] : color)
  //   std::cout << r << " --> " << id << '\n';
  return color;
}

void ColoringRegAllocator::add_spill_code(const std::set<Reg> &nodes) {
  if (is_gp_pass) {
    // NOTE: 在*每个*def后添加store，每个use前插入load
    for (Reg r : nodes) {
      StackObject *obj = nullptr;
      const RegValue *val = nullptr;
      if (f->reg_val.count(r)) {
        val = &(f->reg_val.at(r));
      } else {
        obj = new StackObject;
        obj->size = 4;
        f->stack_objects.emplace_back(obj);
        f->normal_objs.push_back(obj);
      }

      for (auto &bb : f->bbs) {
        auto &insns = bb->insns;
        for (auto it = insns.begin(); it != insns.end();) {
          auto ins = it->get();
          auto def = ins->def();
          auto use = ins->use();

          // def 和 use 分别使用不同新虚拟寄存器的方案
          // // TODO: 需要把def和use分开，之后处理
          // assert(!def.count(r) || !use.count(r));

          // if (use.count(r)) {
          //   Reg tmp = f->new_reg(r.type);
          //   insns.emplace(it, new LoadStack{tmp, obj, 0});
          //   ins->replace_reg(r, tmp); // TODO: 只替换属于use集合的目标寄存器
          //   spilling_regs.insert(tmp);
          // }

          // ++it;
          // if (def.count(r)) {
          //   Reg tmp = f->new_reg(r.type);
          //   insns.emplace(it, new StoreStack{tmp, obj, 0});
          //   ins->replace_reg(r, tmp); // TODO: 只替换属于def集合的目标寄存器
          //   spilling_regs.insert(tmp);
          // }

          Reg tmp;
          if (def.count(r) || use.count(r)) {
            tmp = f->new_reg(r.type);
            ins->replace_reg(r, tmp);
            spilling_regs.insert(tmp);
          }

          auto insn_to_remove = insns.end();
          if (use.count(r)) {
            if (obj) {
              auto mov = dynamic_cast<Move *>(ins);
              if (mov && mov->is_reg_mov()) {
                Reg dst = mov->dst;
                insns.emplace(it, new LoadStack{dst, obj, 0});
                insn_to_remove = it;
              } else {
                insns.emplace(it, new LoadStack{tmp, obj, 0});
              }
            } else {
              // f->reg_val[tmp] = f->reg_val.at(r);
              switch (val->index()) {
              case RegValueType::Imm:
                if (!def.count(r)) // 跳过movt
                  f->emit_imm(insns, it, tmp, std::get<Imm>(*val));
                break;
              case RegValueType::GlobalName:
                insns.emplace(it,
                              new LoadAddr{tmp, std::get<GlobalName>(*val)});
                break;
              case RegValueType::StackAddr:
                insns.emplace(
                    it, new LoadStackAddr{tmp, std::get<StackAddr>(*val), 0});
                break;
              }
            }
          }

          ++it;
          if (def.count(r)) {
            if (obj) {
              // NOTE: 针对switch的特殊处理，不在基本块终结指令后添加store
              if (it != insns.end()) {
                auto mov = dynamic_cast<Move *>(ins);
                if (mov && mov->is_reg_mov()) {
                  Reg src = mov->src.get<RegImmShift>().r;
                  insn_to_remove = std::prev(it);
                  insns.emplace(it, new StoreStack{src, obj, 0});
                } else {
                  insns.emplace(it, new StoreStack{tmp, obj, 0});
                }
              }
            } else
              insn_to_remove = std::prev(it);
          }

          if (insn_to_remove != insns.end())
            insns.erase(insn_to_remove);
        }
      }
    }
  } else {
    // spill 到通用虚拟寄存器，到 gp_pass 一并处理
    // 指令插入位置同 gp_pass
    for (auto r : nodes) {
      auto const gr = f->new_reg(RegType::General);
      for (auto &bb : f->bbs) {
        auto &instrs = bb->insns;
        auto last_def = instrs.cend();
        for (auto iter = instrs.cbegin(); iter != instrs.cend(); ++iter) {
          auto instr = iter->get();
          if (instr->def().count(r)) {
            last_def = iter;
            continue;
          }
          if (instr->use().count(r)) {
            instrs.emplace(iter, new Move{r, Operand2::from(gr)});
          }
        }
        if (last_def != instrs.cend()) {
          instrs.emplace(std::next(last_def), new Move{gr, Operand2::from(r)});
        }
      }
    }
  }
}

void ColoringRegAllocator::init(Function &func, bool is_gp_pass) {
  this->f = &func;
  this->is_gp_pass = is_gp_pass;
  if (is_gp_pass) {
    K = 14; // 16个通用寄存器去掉sp和pc
    this->reg_filter = [](Reg const &reg) { return !reg.is_float(); };
  } else {
    K = NR_FPRS; // 32个单精度vfp寄存器
    this->reg_filter = [](Reg const &reg) { return reg.is_float(); };
  }

  alias.clear();
  move_list.clear();

  // 每次循环后3个xxx_worklist和select_stack总是空的
  spilled_nodes.clear();
  colored_nodes.clear();
  coalesced_nodes.clear();

  coalesced_moves.clear();
  constrained_moves.clear();
  frozen_moves.clear();
  worklist_moves.clear();
  active_moves.clear();

  spilling_regs.clear();
}

void ColoringRegAllocator::replace_virtual_regs(
    const std::map<Reg, int> &reg_map) {
  for (auto &bb : f->bbs) {
    for (auto &insn : bb->insns) {
      auto reg_ptrs = insn->reg_ptrs();

      // auto ins = dynamic_cast<Move *>(insn.get());
      // bool verbose = coalesced_moves.count(ins);
      // if (verbose) {
      //   printf("replacing coalesced move: ");
      //   ins->emit(std::cout);
      //   printf("\n\n");
      // }

      for (auto p : reg_ptrs) {
        Reg old = *p;
        if (reg_map.count(old))
          *p = Reg{old.type, reg_map.at(old)};
        // if (verbose) {
        //   std::cout << old << " --> " << *p << '\n';
        // }
      }

      // if (verbose) {
      //   printf("replaced coalesced move: ");
      //   ins->emit(std::cout);
      //   printf("\n\n");
      // }
    }
  }
}

void ColoringRegAllocator::do_reg_alloc(Function &func, bool is_gp_pass) {
  init(func, is_gp_pass);

  std::map<Reg, int> color;
  bool done = false;

  // int i = 0;
  do {
    // printf("function %s loop %d\n", f->name.c_str(), ++i);

    f->do_liveness_analysis(this->reg_filter);
    build();
    make_worklist();

    do {
      if (!simplify_worklist.empty())
        simplify();
      else if (!worklist_moves.empty())
        coalesce();
      else if (!freeze_worklist.empty())
        freeze();
      else if (!spill_worklist.empty())
        select_spill();
    } while (!simplify_worklist.empty() || !worklist_moves.empty() ||
             !freeze_worklist.empty() || !spill_worklist.empty());

    color = assign_colors();
    if (!spilled_nodes.empty()) {
      // printf("%ld regs spilled\n", spilled_nodes.size());
      // printf("spilled regs:\n");
      // for (Reg n : spilled_nodes)
      //   std::cout << ">>> " << n << '\n';
      // printf("\n\n\n");
      // f->emit(std::cout);

      // 这里不清空spilling_regs，其内容应当累积
      add_spill_code(spilled_nodes);
      spilled_nodes.clear();
      colored_nodes.clear();
      coalesced_nodes.clear();
      continue;
    }
    if (is_gp_pass && !f->check_and_resolve_stack_store())
      continue;
    done = true;
  } while (!done);

  replace_virtual_regs(color);

  // printf("\n\nafter replacement\n\n");
  // f->emit(std::cout);
}

} // namespace armv7

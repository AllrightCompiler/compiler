#include "backend/armv7/ColoringRegAllocator.hpp"
#include "backend/armv7/arch.hpp"

#include <algorithm>

// #include <iostream>

namespace armv7 {

std::set<Reg> ColoringRegAllocator::adjacent(Reg n) const {
  // adjList[n] \ (selectStack ∪ coalescedNodes)
  std::set<Reg> res;
  auto &nodes = adj_list.find(n)->second;
  auto pred = [this](Reg r) {
    return !coalesced_nodes.count(r) &&
           (std::find(select_stack.begin(), select_stack.end(), r) ==
            select_stack.end());
  };
  // std::copy_if(nodes.begin(), nodes.end(), res.begin(), pred);
  for (Reg n : nodes)
    if (pred(n))
      res.insert(n);
  return res;
}

std::set<Move *> ColoringRegAllocator::node_moves(Reg n) const {
  // moveList[n] ∩ (activeMoves ∪ worklistMoves)
  std::set<Move *> res;
  auto &moves = move_list.find(n)->second;
  auto pred = [this](Move *m) {
    return active_moves.count(m) || worklist_moves.count(m);
  };
  // std::copy_if(moves.begin(), moves.end(), res.begin(), pred);
  for (auto m : moves)
    if (pred(m))
      res.insert(m);
  return res;
}

bool ColoringRegAllocator::move_related(Reg n) const {
  return !(node_moves(n).empty());
}

bool ColoringRegAllocator::ok(Reg t, Reg r) const {
  int d = degree.find(t)->second;
  return d < K || !t.is_virt() || adj_set.count({t, r});
}

bool ColoringRegAllocator::conservative(const std::set<Reg> &nodes) const {
  int k = 0;
  for (Reg n : nodes) {
    int d = degree.find(n)->second;
    if (d >= K)
      ++k;
  }
  return k < K;
}

Reg ColoringRegAllocator::get_alias(Reg n) const {
  if (coalesced_nodes.count(n))
    return get_alias(alias.find(n)->second);
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
        if (mov->is_reg_mov()) {
          for (Reg u : use)
            live.erase(u);
          for (Reg n : def)
            move_list[n].insert(mov);
          for (Reg n : use)
            move_list[n].insert(mov);
          worklist_moves.insert(mov);
        }
      }

      for (Reg d : def)
        live.insert(d);
      for (Reg d : def)
        for (Reg l : live)
          add_edge(l, d);
      // live := use(I) ∪ (live - def(I))
      for (Reg d : def)
        live.erase(d);
      for (Reg u : use)
        live.insert(u);
    }
  }
}

void ColoringRegAllocator::make_worklist() {
  // TODO: 也许单独拿出来做寄存器重映射比较好?
  std::set<Reg> vregs;
  for (auto &bb : f->bbs) {
    for (auto &insn : bb->insns) {
      auto def = insn->def();
      auto use = insn->use();
      for (Reg d : def)
        if (d.is_virt())
          vregs.insert(d);
      for (Reg u : use)
        if (u.is_virt())
          vregs.insert(u);
    }
  }

  for (Reg n : vregs) {
    // std::cout << "node " << n << " degree=" << degree[n] << " K=" << K << '\n';
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
  // std::cout << "DecrementDegree: test " << m << " degree=" << degree[m] << '\n';
  if (degree[m]-- == K) {
    auto nodes = adjacent(m);
    nodes.insert(m);
    enable_moves(nodes);
    // std::cout << "DecrementDegree: remove " << m << " from spillWorklist\n";
    
    // NOTE: 这里按照原论文应该移除。实际效果需要进一步确认
    // spill_worklist.erase(m);
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
  // NOTE: 现在只有这个heuristic勉强能用，实际上选择了编号最*小*的虚拟节点(负数的原因)
  // 使用度数仍然会挂掉，问题没有太好的解决
  Reg u = *std::prev(spill_worklist.end());
  // Reg u = *std::max_element(
  //     spill_worklist.begin(), spill_worklist.end(),
  //     [this](const Reg &a, const Reg &b) { return degree[a] < degree[b]; });
  
  // std::cout << "<<< spill target: " << u << '\n';
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
    // NOTE: 在最后的def后添加store，每个use前插入load
    // 这时应该还是单赋值形式，但立即数加载等指令可能会产生连续的def
    for (Reg r : nodes) {
      auto obj = new StackObject;
      obj->size = 4;
      f->stack_objects.emplace_back(obj);
      f->normal_objs.push_back(obj);

      for (auto &bb : f->bbs) {
        auto &insns = bb->insns;
        auto last_def = insns.end();
        for (auto it = insns.begin(); it != insns.end(); ++it) {
          auto ins = it->get();
          auto def = ins->def();
          auto use = ins->use();

          if (def.count(r)) {
            last_def = it;
            continue;
          }
          if (use.count(r))
            insns.emplace(it, new LoadStack{r, obj, 0});
        }

        if (last_def != insns.end())
          insns.emplace(std::next(last_def), new StoreStack{r, obj, 0});
      }
    }
  } else {
    // TODO
  }
}

void ColoringRegAllocator::init(Function &func, bool is_gp_pass) {
  this->f = &func;
  this->is_gp_pass = is_gp_pass;
  if (is_gp_pass) {
    K = 14; // 16个通用寄存器去掉sp和pc
  } else {
    K = NR_FPRS; // 32个单精度vfp寄存器
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
          *p = Reg{old.type, reg_map.find(old)->second};
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

    f->do_liveness_analysis();
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

#include "backend/armv7/ColoringRegAllocator.hpp"

#include <algorithm>

namespace armv7 {

std::set<Reg> ColoringRegAllocator::adjacent(Reg n) const {
  // adjList[n] \ (selectStack ∪ coalescedNodes)
  std::set<Reg> res;
  auto &nodes = adj_list.find(n)->second;
  std::copy_if(nodes.begin(), nodes.end(), res.begin(), [this](Reg r) {
    return !coalesced_nodes.count(r) &&
           (std::find(select_stack.begin(), select_stack.end(), r) ==
            select_stack.end());
  });
  return res;
}

std::set<Move *> ColoringRegAllocator::node_moves(Reg n) const {
  // moveList[n] ∩ (activeMoves ∪ worklistMoves)
  std::set<Move *> res;
  auto &moves = move_list.find(n)->second;
  std::copy_if(moves.begin(), moves.end(), res.begin(), [this](Move *m) {
    return active_moves.count(m) || worklist_moves.count(m);
  });
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
  f->do_liveness_analysis(); // TODO: reg filter
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
  for (Reg m : adjacent(n))
    decrement_degree(m);
}

void ColoringRegAllocator::decrement_degree(Reg m) {
  if (degree[m]-- == K) {
    auto nodes = adjacent(m);
    nodes.insert(m);
    enable_moves(nodes);
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
        worklist_moves.erase(m);
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
  // TODO: heuristic
  Reg u = *spill_worklist.begin();
  spill_worklist.erase(u);
  simplify_worklist.insert(u);
  freeze_moves(u);
}

void ColoringRegAllocator::do_reg_alloc() {
  bool done = false;
  do {
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

    assign_colors();
    if (!spilled_nodes.empty()) {
      rewrite_program(spilled_nodes);
      continue;
    }
    done = true;
  } while (!done);
}

} // namespace armv7

#pragma once

#include "backend/armv7/program.hpp"

namespace armv7 {

// 实现经典的Iterated Register Coalescing论文中的算法

class ColoringRegAllocator {
  // interference graph
  std::map<Reg, std::set<Reg>> adj_list;
  std::set<std::pair<Reg, Reg>> adj_set;

  std::map<Reg, int> degree;
  std::map<Reg, Reg> alias;
  std::map<Reg, std::set<Move *>> move_list;
  std::set<Reg> simplify_worklist;
  std::set<Reg> freeze_worklist;
  std::set<Reg> spill_worklist;
  std::set<Reg> spilled_nodes;
  std::set<Reg> coalesced_nodes;

  std::vector<Reg> colored_nodes;
  std::vector<Reg> select_stack;

  std::set<Move *> coalesced_moves;
  std::set<Move *> constrained_moves;
  std::set<Move *> frozen_moves;
  std::set<Move *> worklist_moves;
  std::set<Move *> active_moves;

  int K; // 可用于分配的物理寄存器数量
  Function *f;

public:
  // functions
  std::set<Reg> adjacent(Reg n) const;
  std::set<Move *> node_moves(Reg n) const;
  bool move_related(Reg n) const;
  bool ok(Reg t, Reg r) const;
  bool conservative(const std::set<Reg> &nodes) const;
  Reg get_alias(Reg n) const;


  // procedures
  void add_edge(Reg u, Reg v);
  void build();
  void make_worklist();
  void simplify();
  void decrement_degree(Reg m);
  void enable_moves(const std::set<Reg> &nodes);
  void coalesce();
  void add_work_list(Reg u);
  void combine(Reg u, Reg v);
  void freeze();
  void freeze_moves(Reg u);
  void select_spill();
  void assign_colors();
  void rewrite_program(const std::set<Reg> &nodes);

public:
  void do_reg_alloc();

};

} // namespace armv7

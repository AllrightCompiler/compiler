#include "mediumend/cfg.hpp"
#include "mediumend/optimizer.hpp"

#include <random>

namespace mediumend {

using ir::BasicBlock;
using ir::Function;
using ir::Reg;

using namespace ir::insns;

// see Static Branch Frequency and Program Profile Analysis
class Context {
  Function &f;
  unordered_map<Reg, ConstValue> constants;
  unordered_set<Reg> likely_bool_results,
      unlikely_bool_results; // for Opcode Heuristic

  // for CFG successor heuristics
  unordered_set<BasicBlock *>
      call_bbs; // basic blocks which contain call instruction
  unordered_set<BasicBlock *>
      store_bbs; // basic blokcs which contain store instruction
  unordered_set<BasicBlock *>
      ret_bbs; // basic blocks which contain return instruction

  // post dominator tree
  PostDominatorTree pdom_tree;

  void collect_info() {
    for (auto &bb : f.bbs) {
      for (auto &insn : bb->insns) {
        auto ins = insn.get();
        // collect constants for Opcode Heuristic
        TypeCase(loadimm, LoadImm *, ins) {
          constants[loadimm->dst] = loadimm->imm;
        }
        else TypeCase(call, Call *, ins) {
          call_bbs.insert(bb.get());
        }
        else TypeCase(store, Store *, ins) {
          store_bbs.insert(bb.get());
        }
        else TypeCase(ret, Return *, ins) {
          ret_bbs.insert(bb.get());
        }
        else TypeCase(binary, Binary *, ins) {
          // TODO: constants on lhs
          Reg lhs = binary->src1, rhs = binary->src2, dst = binary->dst;
          auto op = binary->op;
          if (constants.count(rhs)) {
            if (op == BinaryOp::Eq)
              unlikely_bool_results.insert(dst);
            else if (op == BinaryOp::Neq)
              likely_bool_results.insert(dst);
            else {
              if (constants.at(rhs).iv == 0) {
                if (op == BinaryOp::Lt || op == BinaryOp::Leq)
                  unlikely_bool_results.insert(dst);
                else if (op == BinaryOp::Gt || op == BinaryOp::Geq)
                  likely_bool_results.insert(dst);
              }
            }
          }
        }
      }
    }
  }

  bool loop_branch_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 0.88;
    if (!bb->loop)
      return false;
    auto loop = bb->loop;
    if (br->true_target == loop->header && br->false_target->loop != loop) {
      p1 = p0;
      return true;
    }
    if (br->false_target == loop->header && br->true_target->loop != loop) {
      p1 = 1 - p0;
      return true;
    }
    return false;
  }

  bool opcode_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 0.84;
    if (likely_bool_results.count(br->val)) {
      p1 = p0;
      return true;
    }
    if (unlikely_bool_results.count(br->val)) {
      p1 = 1 - p0;
      return true;
    }
    return false;
  }

  bool loop_exit_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 0.80;
    if (!bb->loop)
      return false;
    auto loop = bb->loop;
    if (br->true_target == loop->header || br->false_target == loop->header)
      return false;                           // already accounted in LBH
    bool b1 = br->true_target->loop == loop;  // true_target in this loop?
    bool b2 = br->false_target->loop == loop; // false_target in this loop?
    if (b1 == b2)
      return false;
    p1 = b1 ? p0 : 1 - p0;
    return true;
  }

  template <typename Apply>
  bool common_heuristic(const Apply &apply, const Branch *br, double &p1,
                        const double p0) {
    if (apply(br->true_target)) {
      p1 = p0;
      return true;
    }
    if (apply(br->false_target)) {
      p1 = 1 - p0;
      return true;
    }
    return false;
  }

  bool loop_header_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 0.75;
    auto is_loop_header = [](BasicBlock *t) {
      return t->loop && t == t->loop->header;
    };
    auto is_loop_preheader = [&](BasicBlock *t) {
      if (t->insns.empty())
        return false;
      auto jump = dynamic_cast<Jump *>(t->insns.back().get());
      if (!jump)
        return false;
      return is_loop_header(jump->target);
    };
    auto apply = [&](BasicBlock *t) {
      return (is_loop_header(t) || is_loop_preheader(t)) &&
             !pdom_tree.pdoms(t, bb);
    };
    return common_heuristic(apply, br, p1, p0);
  }

  bool call_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 1 - 0.78;
    auto apply = [&](BasicBlock *t) {
      return call_bbs.count(t) && !pdom_tree.pdoms(t, bb);
    };
    return common_heuristic(apply, br, p1, p0);
  }

  bool store_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 1 - 0.55;
    auto apply = [&](BasicBlock *t) {
      return store_bbs.count(t) && !pdom_tree.pdoms(t, bb);
    };
    return common_heuristic(apply, br, p1, p0);
  }

  bool return_heuristic(BasicBlock *bb, const Branch *br, double &p1) {
    constexpr double p0 = 1 - 0.72;
    auto apply = [&](BasicBlock *t) { return ret_bbs.count(t); };
    return common_heuristic(apply, br, p1, p0);
  }

public:
  Context(Function &f) : f{f}, pdom_tree{&f} {}

  void compute_branch_probabilities(
      std::map<std::pair<BasicBlock *, BasicBlock *>, double> &probs) {
    for (auto &bb_ptr : f.bbs) {
      auto bb = bb_ptr.get();
      auto terminator = bb->insns.back().get();
      TypeCase(br, Branch *, terminator) {
        double p, p1 = 0.5, p2 = 0.5;

        std::vector<double> hp;
        if (loop_branch_heuristic(bb, br, p))
          hp.push_back(p);
        if (opcode_heuristic(bb, br, p))
          hp.push_back(p);
        if (loop_exit_heuristic(bb, br, p))
          hp.push_back(p);
        if (loop_header_heuristic(bb, br, p))
          hp.push_back(p);
        if (call_heuristic(bb, br, p))
          hp.push_back(p);
        if (store_heuristic(bb, br, p))
          hp.push_back(p);
        if (return_heuristic(bb, br, p))
          hp.push_back(p);

        for (auto p : hp) {
          auto d = p1 * p + p2 * (1 - p);
          p1 = p1 * p / d;
          p2 = p2 * (1 - p) / d;
        }
        probs[{bb, br->true_target}] = p1;
        probs[{bb, br->false_target}] = p2;
      }
      else TypeCase(jump, Jump *, terminator) {
        probs[{bb, jump->target}] = 1.0;
      }
      else TypeCase(sw, Switch *, terminator) {
        int n = sw->targets.size() + 2;
        probs[{bb, sw->default_target}] = 2.0 / n;
        for (auto [_, target] : sw->targets)
          probs[{bb, target}] = 1.0 / n;
      }
    }
  }

  // TODO: implements subroutine `propagate_freq`
  void compute_freq_naive(
      std::map<std::pair<BasicBlock *, BasicBlock *>, double> &edge_freqs,
      const std::map<std::pair<BasicBlock *, BasicBlock *>, double>
          &edge_probs) {
    for (auto &bb : f.bbs)
      bb->freq = 0;

    constexpr int T = 100;
    auto entry = f.bbs.front().get();
    for (int t = 0; t < T; ++t) {
      for (auto bb : f.cfg->rpo) {
        if (bb == entry)
          bb->freq = 1;
        else {
          double f = 0;
          for (auto p : bb->prev)
            f += edge_freqs[{p, bb}];
          bb->freq = f;
        }
        for (auto s : bb->succ) {
          auto e = std::make_pair(bb, s);
          if (edge_probs.count(e))
            edge_freqs[e] = bb->freq * edge_probs.at(e);
          else {
            // s should be a virtual exit
            assert(s->succ.empty());
          }
        }
      }
    }
  }

  // Monte Carlo
  // void compute_freq_naive(
  //     std::map<std::pair<BasicBlock *, BasicBlock *>, double> &edge_freqs,
  //     const std::map<std::pair<BasicBlock *, BasicBlock *>, double>
  //         &edge_probs) {
  //   for (auto &bb : f.bbs)
  //     bb->freq = 0;

  //   constexpr int N = 100000, L = 100;
  //   auto entry = f.bbs.front().get();
  //   std::default_random_engine engine;
  //   std::uniform_real_distribution<double> dist{0.0, 1.0};
  //   for (int n = 0; n < N; ++n) {
  //     auto bb = entry;
  //     for (int i = 0; i < L; ++i) {
  //       bb->freq += 1;
  //       auto terminator = bb->insns.back().get();
  //       TypeCase(jump, Jump *, terminator) {
  //         bb = jump->target;
  //       }
  //       else TypeCase(br, Branch *, terminator) {
  //         auto x = dist(engine);
  //         auto t = edge_probs.at({bb, br->true_target});
  //         if (x < t)
  //           bb = br->true_target;
  //         else
  //           bb = br->false_target;
  //       }
  //       else {
  //         break;
  //       }
  //     }
  //   }

  //   for (auto &bb : f.bbs)
  //     bb->freq /= N;
  // }

  void build() {
    f.loop_analysis();
    pdom_tree.add_virtual_exit();
    pdom_tree.build();
    collect_info();
  }

  void cleanup() {
    pdom_tree.remove_virtual_exit();
  }
};

void estimate_exec_freq(ir::Program *prog) {
  for (auto &[_, f] : prog->functions) {
    Context ctx{f};
    ctx.build();
    ctx.compute_branch_probabilities(f.branch_probs);
    ctx.compute_freq_naive(f.branch_freqs, f.branch_probs);
    ctx.cleanup();
  }
}

} // namespace mediumend

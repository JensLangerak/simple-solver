//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_CARDINALITY_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_CARDINALITY_H_

#include "../propagator_generic.h"
#include "database_cardinality.h"
#include "watched_cardinality_constraint.h"
#include <queue>
namespace Pumpkin {

class SolverState;

class PropagatorCardinality : public PropagatorGeneric {
public:
  explicit PropagatorCardinality(int64_t num_variables);

  ExplanationGeneric *ExplainLiteralPropagation(BooleanLiteral literal,
                                                SolverState &state)
      override; // returns the conjunction that forces the assignment of input
                // literal to true. Assumes the input literal is not undefined.
  ExplanationGeneric *ExplainFailure(SolverState &state)
      override; // returns the conjunction that leads to failure

  ReasonGeneric *ReasonLiteralPropagation(BooleanLiteral literal,
                                          SolverState &state) override;
  ReasonGeneric *ReasonFailure(SolverState &state) override;

  void Synchronise(SolverState &state)
      override; // after the state backtracks, it should call this synchronise
                // method which will internally set the pointer of the trail to
                // the new correct position

  DatabaseCardinality cardinality_database_;

  void SetTrailIterator(TrailList<BooleanLiteral>::Iterator iterator)override;
  bool PropagateOneLiteral(SolverState &state) override;
  void ResetCounts();

  bool CheckCounts(SolverState &state);
  int trigger_count_ = 0;
private:
  // this is the main propagation method. Note that it will change watch lists
  // of true_literal and some other literals and enqueue assignments
  bool PropagateLiteral(BooleanLiteral true_literal,
                        SolverState &state) override;

  WatchedCardinalityConstraint *failure_constraint_;
  BooleanLiteral last_propagated_;
  size_t last_index_;


  void AddEncoding(SolverState &state,
                   WatchedCardinalityConstraint *constraint);
  struct PropagtionElement {
    PropagtionElement(BooleanLiteral lit, int level,
                      PropagatorGeneric *propagator, uint64_t code)
        : lit(lit), level(level), propagator(propagator), code(code){};

    BooleanLiteral lit;
    int level;
    uint64_t code;
    PropagatorGeneric *propagator;

    bool operator<(const PropagtionElement o) const {
      return this->level < o.level;
    }

    bool operator>(const PropagtionElement o) const {
      return this->level > o.level;
    }
  };
  int PropagateLiterals(
      std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                          std::greater<PropagtionElement>>
          queue,
      SolverState &state, int min_var_index);
  int PropagateLiteral2(
      BooleanLiteral true_literal, SolverState &state,
      std::priority_queue<PropagtionElement, std::vector<PropagtionElement>,
                          std::greater<PropagtionElement>> &queue, int min_var);

};
} // namespace Pumpkin
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_PROPAGATOR_CARDINALITY_H_

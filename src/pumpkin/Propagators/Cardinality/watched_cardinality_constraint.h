//
// Created by jens on 16-10-20.
//

#ifndef SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
#define SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_

#include "../../Basic Data Structures/boolean_literal.h"
#include <vector>
namespace Pumpkin {
class ExplanationCardinalityConstraint;
class SolverState;
class WatchedCardinalityConstraint {
public:
  WatchedCardinalityConstraint(std::vector<BooleanLiteral> &literals, int min, int max);
  ExplanationCardinalityConstraint * ExplainLiteralPropagation(BooleanLiteral literal, SolverState &state) const; //returns the conjunction that forces the assignment of input literal to true. Assumes the input literal is not undefined.
  ExplanationCardinalityConstraint * ExplainFailure(SolverState &state) const; //returns the conjunction that leads to failure

  std::vector<BooleanLiteral> literals_;
  int min_;
  int max_;

  int true_count_;
  int false_count_;
};
}
#endif // SIMPLESATSOLVER_SRC_PUMPKIN_PROPAGATORS_CARDINALITY_WATCHED_CARDINALITY_CONSTRAINT_H_
//
// Created by jens on 03-09-20.
//

#include "clause.h"
namespace simple_sat_solver {
Clause Clause::NewClause(simple_sat_solver::Solver &S,
                                                               simple_sat_solver::Vec<simple_sat_solver::Lit> ps,
                                                               bool learnt) {
  // TODO
  return Clause();
}
bool Clause::Locked(simple_sat_solver::Solver *s) {
  return false; //TODO
}
void Clause::Remove(simple_sat_solver::Solver *s) {
//TODO
}
bool Clause::Simplify(simple_sat_solver::Solver *S) {
  return false;//TODO
}
bool Clause::Propagate(simple_sat_solver::Solver *S, simple_sat_solver::Lit p) {
  return false;//TODO
}
void Clause::CalcReason(simple_sat_solver::Solver *S,
                                           simple_sat_solver::Lit p,
                                           simple_sat_solver::Vec<simple_sat_solver::Lit> out_reason) {
//TODO
}
Clause::Clause() {
//TODO
}
void Clause::Undo(Solver *s, Lit p) {
//TODO
}
}
//
// Created by jens on 03-09-20.
//

#include "clause.h"

#include <algorithm>
#include <iostream>

#include "solver.h"

namespace simple_sat_solver {
void Clause::Remove(Solver &s) {
  s.RemoveFromWatchList(lits_[watchA_], this);
  if (watchA_ != watchB_) {
    s.RemoveFromWatchList(lits_[watchB_], this);
  }
}

bool Clause::Simplify(Solver &s) {
  if (lits_.size() == 1) {
    if (!s.SetLitTrue(lits_[0], this)) {
      return false;
    }
  }
  int index = -2;
  for (int i = 0; i < lits_.size(); i++) {
    if (s.GetLitValue(lits_[i]) == LBool::kTrue) {
      index = -1;
      break;
    } else if (s.GetLitValue(lits_[i]) == LBool::kUnknown) {
      if (index >= 0) {
        index = -1;
        break;
      } else {
        index = i;
      }
    }
  }

  if (index >= 0) {
    if (!s.SetLitTrue(lits_[index], this)) {
      return false;
    }
  }

  return index != -2;
}

// Watchers are updated and unit is checked.
// We want to watch a literal that is true or unknown.
// If literal is true, we watch it in case its value gets changed (after undo)
// If literal is unknown, if gets a value then either the clause becomes true
// or the clause might become unit. Or the clause becomes false.
// We will allways watch two different literals, unless there is only one
// literal not false.
bool Clause::Propagate(Solver &s, Lit p) {
  if (!(lits_[watchA_] == p || lits_[watchB_] == p))
    throw "Illegal state: notified by unregistered watch";
  // One of the watchers is true, keep watching current literal.
  if (s.GetLitValue(lits_[watchA_]) == LBool::kTrue ||
      s.GetLitValue(lits_[watchB_]) == LBool::kTrue) {
    s.AddWatch(p, this);
    return true;
  }
  // Lit is made false, search for a lit that is true or unknown.
  // Make sure that the lit is not watched by the other watcher.
  for (int i = 0; i < lits_.size(); i++) {
    if (i == watchA_ || i == watchB_)
      continue;
    if (s.GetLitValue(lits_[i]) != LBool::kFalse) {
      Lit lNew = lits_[i];
      if (p == lits_[watchA_])
        watchA_ = i;
      else
        watchB_ = i;

      s.AddWatch(lNew, this);
      return true;
    }
  }

  // Clause has become either false or unit
  s.AddWatch(p, this);

  // uupdate activity
  activity_ += s.constrIncActivity_;
  if (activity_ > 1e100)
    s.RescaleClauseActivity();

  // make sure that watchA is the lit that decides false or unit
  if (p == lits_[watchA_]) {
    Var temp = watchA_;
    watchA_ = watchB_;
    watchB_ = temp;
  }

  // For debug purposes, check if it is indeed unit or false.
  for (Lit l : lits_) {
    if (l == lits_[watchA_])
      continue;
    if (s.GetLitValue(l) != LBool::kFalse)
      throw "Clause is not unit";
  }

  // Clause is unit or false, SetLit will fail when false
  return s.SetLitTrue(lits_[watchA_], this);
}

Clause::Clause(bool learnt) : learnt_(learnt), activity_(1.0), lock_(false) {}

Clause::Clause(Vec<Lit> lits, bool learnt, Solver &s) : Clause(learnt) {
  // Remove duplicates
  std::sort(lits.begin(), lits.end(), [](Lit l, Lit r) { return l.x < r.x; });
  for (Lit l : lits) {
    if (lits_.empty() || lits_.back() != l) {
      lits_.push_back(l);
    }
  }

  // Set watchers
  if (lits.empty()) {
    watchA_ = -1;
    watchB_ = -1;
  } else if (lits.size() == 1) {
    watchA_ = 0;
    watchB_ = 0;
    Lit l = lits_[0];
    s.AddWatch(l, this);
  } else {
    watchA_ = 0;
    watchB_ = 1;
    Lit lA = lits_[watchA_];
    Lit lB = lits_[watchB_];

    s.AddWatch(lA, this);
    s.AddWatch(lB, this);
  }
}

Clause::~Clause() = default;

void Clause::PrintConstraint() const {
  std::cout << "Clause: ";
  for (auto l : lits_) {
    if (l.complement)
      std::cout << "~";

    std::cout << l.x << " ";
  }
  std::cout << std::endl;
}

void Clause::PrintFilledConstraint(const Vec<LBool> &vars) const {
  std::cout << "Clause: ";
  for (auto l : lits_) {
    LBool lb = vars[l.x];
    if (l.complement)
      std::cout << "~";
    std::cout << l.x;
    if (lb != LBool::kUnknown) {
      bool b = lb == LBool::kTrue;
      b = l.complement ? !b : b;
      std::cout << (b ? "T" : "F");
    }
    std::cout << " ";
  }
  std::cout << std::endl;
}

Vec<Lit> Clause::CalcReason() const { return lits_; }

Vec<Lit> Clause::CalcReason(Lit p) const {
  Vec<Lit> reason;
  for (Lit l : lits_) {
    if (l.x == p.x)
      continue;
    else
      reason.push_back(l);
  }
  return reason;
}

Clause::Clause(const Vec<Lit> &lits, bool learnt, Solver &s, Lit unitLit,
               Lit mostRecentLearnt)
    : Clause(learnt) {
  lits_ = lits;

  // Get indices for the unit and watch Literal
  int indexUnit = -1;
  int mostRecentLearntIndex = -1;
  for (int i = 0; i < lits_.size(); i++) {
    if (lits_[i] == unitLit)
      indexUnit = i;
    if (lits_[i] == mostRecentLearnt)
      mostRecentLearntIndex = i;
    if (indexUnit != -1 && mostRecentLearntIndex != -1)
      break;
  }

  watchA_ = indexUnit;
  watchB_ = mostRecentLearntIndex;
  // No watchB_ if there is only one literal
  if (watchB_ == -1) {
    if (lits.size() == 1)
      watchB_ = 0;
    else
      throw "Clause is not unit";
  }

  s.AddWatch(lits_[watchA_], this);
  s.AddWatch(lits_[watchB_], this);
}

void Clause::Lock() { lock_ = true; }

void Clause::Unlock() { lock_ = false; }

bool Clause::Locked() const { return lock_; }

void Clause::RescaleActivity() { activity_ *= 1e-100; }

bool Clause::Value(const Solver &s) const {
  for (Lit l : lits_) {
    if (s.GetLitValue(l) == LBool::kTrue)
      return true;
  }
  return false;
}

void Clause::CheckWatchers(const Solver *s) const {
  Lit a = lits_[watchA_];
  Lit b = lits_[watchB_];

  int ia = s->LitIndex(a);
  int ib = s->LitIndex(b);
  Vec<Clause *> wl = s->watches_[ia];
  bool wba = false;
  bool wbb = false;
  for (auto c : wl) {
    if (((Clause *)c) == this) {
      wba = true;
      break;
    }
  }
  wl = s->watches_[ib];
  for (auto c : wl) {
    if (((Clause *)c) == this) {
      wbb = true;
      break;
    }
  }

  if (!(wba && wbb)) {
    throw "ERROR";
  }
  if (s->GetLitValue(a) == LBool::kTrue || s->GetLitValue(b) == LBool::kTrue)
    return;
  if (s->GetLitValue(a) == LBool::kFalse ||
      s->GetLitValue(b) == LBool::kFalse) {
    std::queue<Lit> q = s->propagationQueue_;
    bool ba = s->GetLitValue(a) != LBool::kFalse;
    bool bb = s->GetLitValue(b) != LBool::kFalse;

    while (!q.empty()) {
      if (q.front() == a)
        ba = true;
      if (q.front() == b)
        bb = true;
      q.pop();
    }
    if (!(ba && bb))
      throw "Error";
  }
  if (watchA_ == watchB_) {
    if (s->GetLitValue(a) == LBool::kTrue)
      return;
    for (int i = 0; i < lits_.size(); i++) {
      if (i == watchA_ || i == watchB_)
        continue;

      throw "Error";
    }
  }
}
} // namespace simple_sat_solver
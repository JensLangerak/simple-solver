//
// Created by jens on 15-09-20.
//

#include "encoder.h"

namespace simple_sat_solver::sudoku {

SatProblem Encoder::Encode(const Sudoku &sudoku) {
  Encoder e(sudoku.sub_size);
  e.CreateCellConstraints();
  e.CreateRowConstraints();
  e.CreateColumnConstraints();
  e.CreateSubGridConstraints();
  e.AddGivenConstraints(sudoku.cells);
  return e.problem_;
}
void Encoder::CreateCellConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int y = 0; y < size_; y++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int v = 1; v <= size_; v++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateRowConstraints() {
  for (int x = 0; x < size_; x++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int y = 0; y < size_; y++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateColumnConstraints() {
  for (int y = 0; y < size_; y++) {
    for (int v = 1; v <= size_; v++) {
      std::vector<int> exactlyOne;
      exactlyOne.reserve(size_);
      for (int x = 0; x < size_; x++) {
        exactlyOne.push_back(VarIndex(x, y, v));
      }
      CreateUniqueConstraints(exactlyOne);
    }
  }
}
void Encoder::CreateSubGridConstraints() {
  for (int v = 1; v <= size_; v++) {
    for (int subX = 0; subX < subSize_; subX++) {
      for (int subY = 0; subY < subSize_; subY++) {
        std::vector<int> exactlyOne;
        exactlyOne.reserve(size_);
        for (int x = 0; x < subSize_; x++) {
          for (int y = 0; y < subSize_; y++) {
            exactlyOne.push_back(
                VarIndex(x + subX * subSize_, y + subY * subSize_, v));
          }
        }
        CreateUniqueConstraints(exactlyOne);
      }
    }
  }
}
void Encoder::CreateUniqueConstraints(const std::vector<int> &vars) {
  std::vector<solver::Lit> atLeastOne;
  for (int var : vars) {
    atLeastOne.emplace_back(var, false);
  }
  problem_.clauses.push_back(atLeastOne);
  for (int i = 0; i < vars.size() - 1; i++) {
    for (int j = i + 1; j < vars.size(); j++) {
      std::vector<solver::Lit> atMostOne;
      atMostOne.emplace_back(vars[i], true);
      atMostOne.emplace_back(vars[j], true);
      problem_.clauses.push_back(atMostOne);
    }
  }
}
void Encoder::AddGivenConstraints(const std::vector<int> &vars) {
  for (int i = 0; i < vars.size(); i++) {
    if (vars[i] < 1)
      continue;
    int x = i % size_;
    int y = i / size_;
    std::vector<solver::Lit> unit;
    unit.emplace_back(VarIndex(x, y, vars[i]), false);
    problem_.clauses.push_back(unit);
  }
}
Sudoku Encoder::Decode(int subSize,
                                 std::vector<solver::LBool> &solution) {
  int size = subSize * subSize;
  std::vector<int> res;
  res.reserve(size * size);
  for (int i = 0; i < size * size; i++) {
    bool added = false;
    for (int v = 1; v <= size; v++) {
      if (solution[i * size + v - 1] == solver::LBool::kTrue) {
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }


  return Sudoku(subSize, res);
}
Sudoku Encoder::Decode(int subSize, std::vector<bool> &solution) {
  int size = subSize * subSize;
  std::vector<int> res;
  res.reserve(size * size);
  for (int i = 0; i < size * size; i++) {
    bool added = false;
    for (int v = 1; v <= size; v++) {
      if (solution[i * size + v - 1]){
        if (added)
          throw "Solution is not valid!";
        added = true;
        res.push_back(v);
      }
    }
    if (!added)
      res.push_back(-1);
  }


  return Sudoku(subSize, res);
}
} // namespace simple_sat_solver::sudoku
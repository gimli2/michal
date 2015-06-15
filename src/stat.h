// prevent redefinition + #endif on the end of the file
#ifndef STAT_H
#define STAT_H

#include <vector>
#include "base.h"

class StatResultComparatorSize {
public:
  bool operator()(const Result&, const Result&);
//    bool operator()(Result, Result);
};
class StatResultComparatorCenterDist {
  Pos center;
public:
  StatResultComparatorCenterDist(Pos);
  bool operator()(const Result&, const Result&);
};

struct StatResult {
  bool is_match;
  Result bestres;
};

std::vector<Result> filterResultsByDistanceFromCenter(std::vector<Result>);
std::vector<Result> filterResultsBySize(std::vector<Result>);

StatResult findAndDecideBestStat(std::vector<Result> res, int tries);

#endif

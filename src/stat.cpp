#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>


#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <algorithm>

#include "base.h"
#include "stat.h"

using namespace cv;
using namespace std;

bool StatResultComparatorSize::operator()(const Result& r1, const Result& r2) {
  int w1 = r1.left + r1.right;
  int h1 = r1.bottom + r1.top;
  int s1 = w1+h1;

  int w2 = r2.left + r2.right;
  int h2 = r2.bottom + r2.top;
  int s2 = w2+h2;

  return s1 < s2;
}

StatResultComparatorCenterDist::StatResultComparatorCenterDist(Pos center):center(center) {
}

bool StatResultComparatorCenterDist::operator()(const Result& r1, const Result& r2) {
  int w1 = r1.center.x - center.x;
  int h1 = r1.center.y + center.y;
  int s1 = w1*w1+h1*h1;


  int w2 = r2.center.x - center.x;
  int h2 = r2.center.y + center.y;
  int s2 = w2*w2+h2*h2;

  return s1 < s2;
}

vector<Result> filterResultsByDistanceFromCenter(vector<Result> res) {
  Pos centeravg(0,0);

  for(vector<Result>::iterator it = res.begin(); it != res.end(); ++it) {
    centeravg.y += (*it).center.y;
    centeravg.x += (*it).center.x;
  }
  centeravg.x /= res.size();
  centeravg.y /= res.size();



  std::sort(res.begin(), res.end(), StatResultComparatorCenterDist(centeravg));

  vector<Result> res_filtered;

  int s = res.size();
  vector<Result>::iterator b = res.begin();
  vector<Result>::iterator e = res.end()-s/4;

  for(vector<Result>::iterator it = b; it != e; ++it)
    res_filtered.push_back(*it);

  return res_filtered;
}

vector<Result> filterResultsBySize(vector<Result> res) {
  std::sort(res.begin(), res.end(), StatResultComparatorSize());

  vector<Result> res_filtered;

  int s = res.size();
  vector<Result>::iterator b = res.begin()+s/8;
  vector<Result>::iterator e = res.end()-s/8;

  for(vector<Result>::iterator it = b; it != e; ++it)
    res_filtered.push_back(*it);

  return res_filtered;
}


StatResult findAndDecideBestStat(vector<Result> res, int tries) {
  StatResult ret;
  ret.is_match = false;

  if(res.size() <= 0)
    return ret;

  tries++;


  //filter by distance from average center
  if(res.size() > 5) {
//	cout << "FILTER CENTER BEFORE "<<res.size()<<endl;
    res = filterResultsByDistanceFromCenter(res);
//	cout << "FILTER CENTER AFTER "<<res.size()<<endl;
  }



  //filter by size
  if(res.size() > 5) {
//	cout << "FILTER SIZE BEFORE "<<res.size()<<endl;
    res = filterResultsBySize(res);
//	cout << "FILTER SIZE AFTER "<<res.size()<<endl;
  }

  //arithmetic average of all centers

  int av_w = 0;	//average width
  int av_h = 0;	//average height
  Pos centeravg(0,0);
//    int cnt = 0;

//    vector<Result>::iterator = res.size();
  for(vector<Result>::iterator it = res.begin(); it != res.end(); ++it) {
    av_w += (*it).left + (*it).right;
    av_h += (*it).bottom + (*it).top;

    centeravg.x += (*it).center.x;
    centeravg.y += (*it).center.y;
  }
  av_w /= res.size();
  av_h /= res.size();

  centeravg.x /= res.size();
  centeravg.y /= res.size();


  ret.is_match = true;
  ret.bestres = Result(centeravg, av_h/2, av_h/2, av_w/2, av_w/2);

  return ret;
}

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>


#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "base.h"
#include "hardness.h"

using namespace cv;
using namespace std;

HardnessResult computeHardness(Result res, double HV, double pxsize) {
  double dw = res.left + res.right;
  double dh = res.top + res.bottom;

//    double pxsize = 0.1277;	//objektiv 40x
//    double pxsize = 0.1277*1E-6;	//objektiv 40x
//    double pxsize = 0.0841*1E-6;	//objektiv 60x

  dw *= pxsize;
  dh *= pxsize;	//prepocitam velikost na metry

//   cout <<"dw: "<< dw <<endl;

//    double HV = 0.1;	//hmotnost hrotu
  double F = HV*9.823;	//m*g

//    double d2 = dw*dw + dh*dh;	//diagonala nadruhou

  double hardness = 0.1891*F/(dw*dh);

  HardnessResult r;
  r.hardness = hardness;
  r.w = dw;
  r.h = dh;

  return r;
//    return hardness;
}

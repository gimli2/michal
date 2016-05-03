#ifndef EVALUATE_H
#define EVALUATE_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "base.h"

using namespace cv;
using namespace std;

bool resultCompare(Point2f center, Result result_hough, Result result_convex, Result & result, float tolerance);
bool symmetrize(Point2f center, Result & result, float tolerance);


#endif
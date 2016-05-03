#ifndef CONVEXHULL_H
#define CONVEXHULL_H

#include <opencv/cv.h>

#include "base.h"

using namespace cv;
using namespace std;

Result ConvexHullMethod(Mat image, Point2f center);

#endif
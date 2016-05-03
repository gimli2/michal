#ifndef HOUGH_H
#define HOUGH_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "base.h"

using namespace cv;
using namespace std;

Result HoughTransf(Mat & image, Point2f center, int hough_threshold, bool probabilistic, float groups_min_distance, float max_gab_tolerance);

#endif
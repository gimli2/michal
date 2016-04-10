#ifndef DETECT_H
#define DETECT_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "base.h"

using namespace cv;
using namespace std;

Result DetectByTemplate(Mat & image, Mat & temp, int match_method);

#endif
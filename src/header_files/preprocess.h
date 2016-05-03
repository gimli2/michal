#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

void removeNoise(Mat & image, int operation, int kernel_shape, int kernel_size);

#endif
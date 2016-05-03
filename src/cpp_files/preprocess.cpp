#include <iostream>
#include <vector>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../header_files/preprocess.h"

using namespace cv;
using namespace std;

void removeNoise(Mat & image, int operation, int kernel_shape, int kernel_size){
	bitwise_not(image, image); //invert image

	Mat kernel = getStructuringElement(kernel_shape, Size(kernel_size, kernel_size));
	morphologyEx(image, image, operation, kernel);

	// return image invertion back to normal
	bitwise_not(image, image);
}
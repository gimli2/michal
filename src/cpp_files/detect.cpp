#include <iostream>
#include <vector>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../header_files/base.h"
#include "../header_files/detect.h"

using namespace cv;
using namespace std;

Mat TemplateDetect(Mat & image, Mat & temp, int match_method);

Result DetectByTemplate(Mat & image, Mat & temp, int match_method){
	
	// Detect the template position
	Mat normalized = TemplateDetect(image, temp, match_method);

	// Localize the best match position
	double minVal, maxVal;
	Point minLoc, maxLoc, matchLoc;
	minMaxLoc(normalized, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	// For SQDIFF and SQDIFF_NORMED, the best matches are lower values.
	if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
		matchLoc = minLoc;
	else
		matchLoc = maxLoc;


	// debug info
	if (cfg.debug_level > 0){
		cout << "Best detected template position is at ";
		cout << "(" << matchLoc.x + temp.cols / 2 << ", " << matchLoc.y + temp.rows / 2 << ")" << endl;
	}
	if (cfg.debug_level >= 2){
		Mat image_output;
		cvtColor(image, image_output, CV_GRAY2BGR);
		line(image_output, Point(matchLoc.x + temp.cols / 2, matchLoc.y), Point(matchLoc.x + temp.cols / 2, matchLoc.y + temp.rows), Scalar(0, 255, 0), 2, 8, 0);
		line(image_output, Point(matchLoc.x, matchLoc.y + temp.rows / 2), Point(matchLoc.x + temp.cols, matchLoc.y + temp.rows / 2), Scalar(0, 255, 0), 2, 8, 0);

		// Save result
		string s = to_string(match_method);
		SaveImg(image_output, "", s + "detectin", (cfg.debug_level >= 4));
	}

	return Result((float)matchLoc.y, (float)matchLoc.y + temp.rows, (float)matchLoc.x, (float)matchLoc.x + temp.cols);
}
Mat TemplateDetect(Mat & image, Mat & temp, int match_method){

	// Create the result matrix
	Mat result;
	int result_cols = image.cols - temp.cols + 1;
	int result_rows = image.rows - temp.rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);

	// Do the matching and mormalize result
	matchTemplate(image, temp, result, match_method);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	return result;
}
#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>

#include "../header_files/base.h"
#include "../header_files/convexHull.h"

using namespace cv;
using namespace std;

Result ConvexHullMethod(Mat image, Point2f center){
	// input image in gray style with applyed treshold

	if (cfg.debug_level > 0){
		cout << endl << "Convex hull method:" << endl;
	}

	Result result;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat image_output;
	cvtColor(image, image_output, CV_GRAY2BGR);


	// find  contours in binary image
	findContours(image, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	vector<vector<Point> > hull(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
	{
		// filter small areas
		if (contourArea(contours[i]) > 500){

			// Find the convex hull of a point set.
			convexHull(contours[i], hull[i]);

			approxPolyDP(hull[i], hull[i], 0.1 * arcLength(hull[i], true), true);

			if (hull[i].size() == 4){
				int left = 0, right = 0, top = 0, bottom = 0;
				for (size_t j = 1; j < hull[i].size(); j++)
				{
					if (hull[i][j].x < hull[i][left].x)
						left = j;
					if (hull[i][j].x > hull[i][right].x)
						right = j;
					if (hull[i][j].y < hull[i][top].y)
						top = j;
					if (hull[i][j].y > hull[i][bottom].y)
						bottom = j;
				}

				// Select the hull with the center inside
				if ((left != right && left != top && left != bottom &&
					right != top && right != bottom && top != bottom)
					&&
					(hull[i][left].x < center.x && hull[i][right].x > center.x
					&& hull[i][top].y < center.y && hull[i][bottom].y > center.y))
				{
					result.setPosition((float)hull[i][top].y, (float)hull[i][bottom].y, (float)hull[i][left].x, (float)hull[i][right].x);

					// debug info
					if (cfg.debug_level > 0){
						cout << "top-left point = (" << (int)result.left << ", " << (int)result.top << ")" << endl;
						cout << "bottom-right point = (" << (int)result.right << ", " << (int)result.bottom << ")" << endl;
					}
					if (cfg.debug_level >= 2){
						drawContours(image_output, hull, -1, Scalar(0, 0, 255), 2, 8);
						DrawResult(image_output, result, Scalar(0, 255, 0));
						SaveImg(image_output, "", "ConvexHull", (cfg.debug_level >= 3));
					}

					return result;
				}
			}
		}
	}

	if (cfg.debug_level > 0){
		cout << "Result not found." << endl;
	}
	return result;
}
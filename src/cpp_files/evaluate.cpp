#include <iostream>
#include <stdio.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../header_files/evaluate.h"
#include "../header_files/base.h"

using namespace cv;
using namespace std;

bool resultCompare(Point2f center, Result result_hough, Result result_convex, Result & result, float tolerance){
	if (result_hough.top == 0 && result_hough.bottom == 0 && result_hough.left == 0 && result_hough.right == 0)
		return false;
	if (result_convex.top == 0 && result_convex.bottom == 0 && result_convex.left == 0 && result_convex.right == 0)
		return false;

	float dist = result_hough.top - result_convex.top;
	if (abs(dist) > tolerance){
		// top doesn't match 

		dist = result_hough.bottom - result_convex.bottom;
		if (abs(dist) > tolerance){
			// bottom also doesn't mach - detection unsuccessfull
			return false;
		}
		else{
			result.bottom = result_convex.bottom;
		}

		// Compute top from bottom and center
		result.top = center.y - (result.bottom - center.y);
	}
	else{
		result.top = result_convex.top;

		dist = result_hough.bottom - result_convex.bottom;
		if (abs(dist) > tolerance){
			// bottom doesn't mach - calculate from top and center
			result.bottom = center.y + (center.y - result.top);
		}
		else{
			result.bottom = result_convex.bottom;
		}
	}



	dist = result_hough.left - result_convex.left;
	if (abs(dist) > tolerance){
		// Left side doesn't match 

		dist = result_hough.right - result_convex.right;
		if (abs(dist) > tolerance){
			// right also doesn't mach - detection unsuccessfull
			return false;
		}
		else{
			result.right = result_convex.right;
		}

		// Compute left from right and center
		result.left = center.x - (result.right - center.x);
	}
	else{
		result.left = result_convex.left;

		dist = result_hough.right - result_convex.right;
		if (abs(dist) > tolerance){
			// Right side doesn't mach - calculate from top and center
			result.right = center.x + (center.x - result.left);
		}
		else{
			result.right = result_convex.right;
		}
	}

	if (cfg.debug_level > 0){
		cout << endl << "Result compare :" << endl;
		cout << "Hought transform: (" << (int)result_hough.left << ", " << (int)result_hough.top << "), (" << (int)result_hough.right << ", " << (int)result_hough.bottom << ")" << endl;
		cout << "Convex hull: (" << (int)result_convex.left << ", " << (int)result_convex.top << "), (" << (int)result_convex.right << ", " << (int)result_convex.bottom << ")" << endl;
		cout << "result: (" << (int)result.left << ", " << (int)result.top << "), (" << (int)result.right << ", " << (int)result.bottom << ")" << endl;
	}
	return true;
}

bool symmetrize(Point2f center, Result & result, float tolerance){
	// symmetrize the result by the center (if off tolerance)

	// x part
	float dist = abs((center.x - result.left) - (result.right - center.x));
	if (dist > tolerance){
		float bigger;
		if ((center.x - result.left) > (result.right - center.x)){
			bigger = (center.x - result.left);
			if (abs((center.y - result.top) - bigger) > tolerance && abs((result.bottom - center.y) - bigger) > tolerance){
				result.left = center.x - (result.right - center.x);
			}
		}
		else{
			bigger = (result.right - center.x);
			if (abs((center.y - result.top) - bigger) > tolerance && abs((result.bottom - center.y) - bigger) > tolerance){
				result.right = center.x + (center.x - result.left);
			}
		}
	}

	// y part
	dist = abs((center.y - result.top) - (result.bottom - center.y));
	if (dist > tolerance){
		float bigger;
		if ((center.y - result.top) > (result.bottom - center.y)){
			bigger = (center.y - result.top);
			if (abs((center.x - result.left) - bigger) > tolerance && abs((result.right - center.x) - bigger) > tolerance){
				result.top = center.y - (result.bottom - center.y);
			}
		}
		else{
			bigger = (result.bottom - center.y);
			if (abs((center.x - result.left) - bigger) > tolerance && abs((result.right - center.x) - bigger) > tolerance){
				result.bottom = center.y + (center.y - result.top);
			}
		}
	}

	return true;
}
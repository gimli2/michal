#include <iostream>
#include <vector>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../header_files/base.h"
#include "../header_files/hough.h"

using namespace cv;
using namespace std;

Point2f computeIntersect(Vec4i a, Vec4i b);
Point2f computeIntersect(Vec2f line1, Vec2f line2);
vector<Point2f> lineToPointPair(Vec2f line);
bool acceptLinePair(Vec2f line1, Vec2f line2, float minTheta);

vector< Group* > sortGroups(vector<Point2f> points, float min_distance);
bool punctureFind(Mat & image, vector< Group* > group_blocks, Point2f center, Result & result, float gab_tolerance);
vector<Point2f> removeGroups(Mat & image, vector<Point2f> points, float resolution);
bool whiteLineArea(Mat & image, Point2f from, Point2f to);

void DrawLines(Mat & image, vector<Vec2f> lines);
void DrawLines(Mat & image, vector<Vec4i> lines);
void DrawIntersections(Mat & image, vector<Point2f> intersections);
void DrawGroups(Mat & image, vector< Group* > group_blocks);


float pointsDistance(Point2f p1, Point2f p2) {
	float x = p1.x - p2.x;
	float y = p1.y - p2.y;
	return sqrt(x*x + y*y);
}

template <typename T>
void DebugLines(Mat & image, vector<T> lines, vector<Point2f> intersections){
	Mat image_draw;
	switch (cfg.debug_level)
	{
	case (4) :
	case (3) :
	case (2) :
		cout << "Detected " << lines.size() << " lines." << endl;

		cvtColor(image, image_draw, CV_GRAY2BGR);
		DrawLines(image_draw, lines);
		SaveImg(image_draw, "", "hough-lines", (cfg.debug_level >= 4));

		cvtColor(image, image_draw, CV_GRAY2BGR);
		DrawIntersections(image_draw, intersections);
		SaveImg(image_draw, "", "hough", (cfg.debug_level >= 4));
		break;
	case (1):
		cout << "Detected " << lines.size() << " lines." << endl;
		break;
	default:
		break;
	}
}

Result HoughTransf(Mat & image, Point2f center, int hough_threshold, bool probabilistic, float groups_min_distance, float max_gab_tolerance){
	//hough_threshold = Accumulator threshold parameter. Only those lines are returned that get enough votes.
	if (cfg.debug_level > 0){
		cout << endl << "Hough transformation method:" << endl;
	}

	//resize(image, image, Size(0, 0), 0.25, 0.25);

	Mat blured;
	GaussianBlur(image, blured, Size(7, 7), 2.0, 2.0);

	Mat edges;
	Canny(blured, edges, 66.0, 133.0, 3);

	vector<Point2f> intersections;
	if (probabilistic){
		vector<Vec4i> lines;
		int minLineLength = 50;
		int maxLineGap = 10;
		HoughLinesP(edges, lines, 1, CV_PI / 180, hough_threshold, minLineLength, maxLineGap);

		// compute the intersections of the detected lines
		for (size_t i = 0; i < lines.size(); i++){
			for (size_t j = 0; j < lines.size(); j++){
				Point2f intersec = computeIntersect(lines[i], lines[j]);
				intersections.push_back(intersec);
			}
		}
		DebugLines(image, lines, intersections);
	}
	else{
		vector < Vec2f > lines;
		HoughLines(edges, lines, 1, CV_PI / 180, hough_threshold, 0, 0);

		// compute the intersections of the detected lines
		for (size_t i = 0; i < lines.size(); i++){
			for (size_t j = 0; j < lines.size(); j++){
				Vec2f line1 = lines[i];
				Vec2f line2 = lines[j];
				if (acceptLinePair(line1, line2, (float)CV_PI / 32)){
					Point2f intersection = computeIntersect(line1, line2);
					intersections.push_back(intersection);
				}
			}
		}
		DebugLines(image, lines, intersections);
	}

	// filter and sort the intersections groups
	Result result;
	float gab_tolerance = 0.;
	vector< Group* > group_blocks = sortGroups(intersections, groups_min_distance);
	for (; gab_tolerance <= max_gab_tolerance; gab_tolerance += 5){
		if (punctureFind(image, group_blocks, center, result, gab_tolerance))
			break;
	}

	// write and save results
	if (cfg.debug_level > 0 && gab_tolerance > max_gab_tolerance){
		cout << "Hough transform method: " << endl;
		cout << "The result not found." << endl;
		group_blocks.clear();
		return result;
	}
	if (cfg.debug_level > 0){
		cout << "Hough resuls: " << endl;
		cout << "top-left point = (" << (int)result.left << ", " << (int)result.top << ")" << endl;
		cout << "bottom-right point = (" << (int)result.right << ", " << (int)result.bottom << ")" << endl;
	}
	if (cfg.debug_level >= 2){
		cout << "gab tolerance for detection was: " << gab_tolerance << endl;
		Mat image_output;
		cvtColor(image, image_output, CV_GRAY2BGR);
		DrawIntersections(image_output, intersections);
		DrawGroups(image_output, group_blocks);
		line(image_output, Point2f(center.x, center.y - 25), Point2f(center.x, center.y + 25), Scalar(0, 0, 255));
		line(image_output, Point2f(center.x - 25, center.y), Point2f(center.x + 25, center.y), Scalar(0, 0, 255));
		SaveImg(image_output, "", "hough-groupsS", (cfg.debug_level >= 4));
		DrawResult(image_output, result, Scalar(0, 255, 0));
		SaveImg(image_output, "", "hough-result", (cfg.debug_level >= 3));
	}

	group_blocks.clear();
	return result;
}
bool acceptLinePair(Vec2f line1, Vec2f line2, float minTheta)
{
	float theta1 = line1[1], theta2 = line2[1];

	if (theta1 < minTheta){
		theta1 += (float)CV_PI; // dealing with 0 and 180 ambiguities...
	}

	if (theta2 < minTheta){
		theta2 += (float)CV_PI; // dealing with 0 and 180 ambiguities...
	}

	return abs(theta1 - theta2) > minTheta;
}
Point2f computeIntersect(Vec4i a, Vec4i b) {
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
		Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return Point2f(-1, -1);
}
Point2f computeIntersect(Vec2f line1, Vec2f line2){
	vector<Point2f> p1 = lineToPointPair(line1);
	vector<Point2f> p2 = lineToPointPair(line2);

	float x1 = p1[0].x, y1 = p1[0].y, x2 = p1[1].x, y2 = p1[1].y, x3 = p2[0].x, y3 = p2[0].y, x4 = p2[1].x, y4 = p2[1].y;

	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
		Point2f pt;
		pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
		pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
		return pt;
	}
	else
		return Point2f(-1, -1);
}
vector<Point2f> lineToPointPair(Vec2f line)
{
	// split the line into two points
	vector<Point2f> points;

	float r = line[0], t = line[1];
	float cos_t = cos(t), sin_t = sin(t);
	float x0 = r*cos_t, y0 = r*sin_t;
	float alpha = 1000;

	points.push_back(Point2f(x0 + alpha*(-sin_t), y0 + alpha*cos_t));
	points.push_back(Point2f(x0 - alpha*(-sin_t), y0 - alpha*cos_t));

	return points;
}

vector< Group* > sortGroups(vector<Point2f> points, float min_distance) {
	// sort the lines intersections groups into Groups
	vector< Group* > group_blocks;
	vector< vector<Point2f> > groups;
	unsigned int points_index = 0;
	unsigned int group_index = 0;
	unsigned int in_group_index = 0;

	while (points.size() != 0){
		Point2f p = *points.begin();
		Group* g = new Group(p.y, p.y, p.x, p.x);
		g->points = 1;

		vector<Point2f> group_i;
		groups.push_back(group_i);
		groups[group_index].push_back(p);
		points.erase(points.begin());

		in_group_index = 0;
		while (in_group_index < groups[group_index].size()){

			points_index = 0;
			while (points_index < points.size()){
				p = points[points_index];
				// add points in minimum range distance to current group
				if (pointsDistance(groups[group_index][in_group_index], p) <= min_distance){
					groups[group_index].push_back(points[points_index]);
					if (g->position->top > p.y)
						g->position->top = p.y;
					if (g->position->bottom < p.y)
						g->position->bottom = p.y;
					if (g->position->left > p.x)
						g->position->left = p.x;
					if (g->position->right < p.x)
						g->position->right = p.x;

					g->points++;
					points.erase(points.begin() + points_index);
				}
				else{
					points_index++;
				}
			}
			in_group_index++;
		}
		group_blocks.push_back(g);
		group_index++;
	}

	return group_blocks;
}

bool punctureFind(Mat & image, vector< Group* > group_blocks, Point2f center, Result & result, float gab_tolerance){
	// Find the puncture position by center and intersections groups
	if (group_blocks.size() == 0)
		return false;

	Group *top_g = NULL, *bottom_g = NULL, *left_g = NULL, *right_g = NULL;
	result.top = 0;
	result.bottom = 10 * center.y;
	result.left = 0;
	result.right = 10 * center.x;

	for (Group* g : group_blocks){
		if (g->position->left - gab_tolerance < center.x && g->position->right + gab_tolerance > center.x){
			if (g->position->bottom < center.y && g->position->bottom > result.top){
				result.top = g->position->bottom;
				top_g = g;
			}
			if (g->position->top > center.y && g->position->top < result.bottom){
				result.bottom = g->position->top;
				bottom_g = g;
			}
		}

		if (g->position->top - gab_tolerance < center.y && g->position->bottom + gab_tolerance > center.y){
			if (g->position->right < center.x && g->position->right > result.left){
				result.left = g->position->right;
				left_g = g;
			}
			if (g->position->left > center.x && g->position->left < result.right){
				result.right = g->position->left;
				right_g = g;
			}
		}
	}

	if (top_g != NULL && bottom_g != NULL && left_g != NULL && right_g != NULL){
		float white_extension = 5.;
		while (whiteLineArea(image, 
			Point2f(top_g->position->left - white_extension, result.top + 1), 
			Point2f(top_g->position->right + white_extension, result.top + 1)))
			result.top++;
		while (whiteLineArea(image, 
			Point2f(bottom_g->position->left - white_extension, result.bottom - 1), 
			Point2f(bottom_g->position->right + white_extension, result.bottom - 1)))
			result.bottom--;
		while (whiteLineArea(image, 
			Point2f(result.left + 1, left_g->position->top - white_extension), 
			Point2f(result.left + 1, left_g->position->bottom + white_extension)))
			result.left++;
		while (whiteLineArea(image, 
			Point2f(result.right - 1, right_g->position->top - white_extension), 
			Point2f(result.right - 1, right_g->position->bottom + white_extension)))
			result.right--;

		return true;
	}
	else
		return false;
}
bool whiteLineArea(Mat & image, Point2f from, Point2f to){
	// check for white space between two points
	bool horizontal = (from.y == to.y);
	
	float i_start;
	float i_end;
	if (horizontal){
		i_start = (from.x < 0) ? 0 : from.x;
		i_end = (to.x >= image.cols) ? image.cols - 1 : to.x;
	}
	else{
		i_start = (from.y < 0) ? 0 : from.y;
		i_end = (to.y >= image.rows) ? image.rows - 1 : to.y;
	}
	int j = (int)((horizontal) ? from.y : from.x);

	for (int i = (int)i_start; i < (int)i_end; i++){
		if (horizontal){
			if (image.at<uchar>(j, i) == 0){
				return false;
				break;
			}
		}
		else{
			if (image.at<uchar>(i, j) == 0){
				return false;
				break;
			}
		}
	}
	return true;
}

// ----------------------  DRAWING FUNCTIONS -------------------------
void DrawLines(Mat & image, vector<Vec2f> lines){
	// Draw Hough detection lines into image - regular
	if (image.data == NULL)
		return;
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(image, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
	}
}
void DrawLines(Mat & image, vector<Vec4i> lines){
	// Draw Hough detection lines into image - probabilistic
	if (image.data == NULL)
		return;

	for (size_t i = 0; i < lines.size(); i++) {
		line(image, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(0, 0, 255), 3, 8);
	}
}
void DrawIntersections(Mat & image, vector<Point2f> intersections){
	// Draw lines intersections
	if (intersections.size() > 0 && image.data != NULL){
		vector<Point2f>::iterator i;
		for (i = intersections.begin(); i != intersections.end(); ++i){
			circle(image, *i, 1, Scalar(0, 255, 0), 3);
		}
	}
}
void DrawGroups(Mat & image, vector< Group* > group_blocks){
	if (image.data == NULL)
		return;
	
	// draw group blocks into image
	for (size_t i = 0; i < group_blocks.size(); i++) {
		rectangle(image, Point2f(group_blocks[i]->position->left, group_blocks[i]->position->top),
			Point2f(group_blocks[i]->position->right, group_blocks[i]->position->bottom), Scalar(0, 0, 255), 2, 8);
	}
}
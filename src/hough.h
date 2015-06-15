// prevent redefinition + #endif on the end of the file
#ifndef HOUGH_H
#define HOUGH_H

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <string>

#include "base.h"


#include <opencv/cv.h>
#include <opencv/highgui.h>


cv::Point2f computeIntersect(cv::Vec4i a,
                             cv::Vec4i b);

//cv::Vec2d linesIntersection;

float lineAngle(cv::Vec4i l);

bool lineOrientation(cv::Vec4i l);

bool isLineDiagonal(cv::Vec4i l);

float pointsDistance(cv::Point2f p1, cv::Point2f p2);

bool isInsideImage(cv::Point2f p, const cv::Mat& image);

void preprocessCV(cv::Mat& image);

void drawCross(cv::Point2f p, cv::Mat& image);

std::vector<cv::Point2f> removeGroups(std::vector<cv::Point2f> points, float resolution);

void parseHough(
  cv::Mat& image,
  cv::Mat& original_image,
  Result& retres,
  float& is_found,
  int minLineLength,
  int maxLineGap,
  int threshold_val,
  bool dump_debug,
  std::string debug_prefix
);

void findHoughResults(cv::Mat im, std::vector<Result>& ret, int & added, int& tries);

#endif

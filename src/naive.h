// prevent redefinition + #endif on the end of the file
#ifndef NAIVE_H
#define NAIVE_H

#include <string>
#include <vector>

#include "base.h"

#define M_PI 3.14159265358979323846

void removeSmallContinuousAreas(cv::Mat& image, unsigned int maxareasize);

void preprocess(cv::Mat& image);




void findDimensionsForCenter(cv::Mat& image, Pos& center, std::vector<Result>& results);

void findXYLimits(cv::Mat& image,std::vector<Part>& xparts,std::vector<Part>& yparts,int thresholdoffset, bool dump_debug, std::string debug_prefix);

void parsePossibleOnePart(cv::Mat& image, std::vector<Result>& results, const Part& xpart, const Part& ypart);

void parse(cv::Mat& image, cv::Mat& original_image,Result& retres,float& is_found, int thresholdoffset, bool dump_debug, std::string debug_prefix);

/**
 *  @brief Get all results of parametrised native
 *  @param im image to search results
 *  @param ret out parameter list of results to add
 *  @param added out parameter of count of added results of native
 *  @param tries out parameter of count of tried results of native
 */
void findNaiveResults(cv::Mat im, std::vector<Result>& ret, int & added, int& tries);

#endif

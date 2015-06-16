// prevent redefinition + #endif on the end of the file
#ifndef BASE_H
#define BASE_H

#pragma pack()
#include <vector>
#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#ifdef _WIN32
	double round(double d);
#endif

//#ifdef WIN32
//#define ourImread(filename, isColor) cvLoadImage(filename, isColor)
//#else
#define ourImread(filename, isColor) imread(filename, isColor)
//#endif


#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
    #include <direct.h>
    #define getCurrentDir _getcwd
#else
    #include <unistd.h>
    #define getCurrentDir getcwd
#endif

#define M_PI 3.14159265358979323846

struct config{
  std::string fin;
  std::string fout;
  std::string ftextout;
  int dbglev;
  double ppFilterTolerance;
  int ppHistogramSmooth;
  int ppBlur;
  long ppSmallAreasSize;
};

struct Pos{
    int x;
    int y;
    Pos(int x, int y);
    Pos();
};

struct Result{
    Pos center;
    int top;
    int bottom;
    int left;
    int right;
    float vaha;
    Result(Pos center, int top, int bottom, int left, int right, float vaha = 1);
    Result();
};

// TODO: rename to interval
struct Part{
    int from;
    int to;
    Part(int from, int to);
    Part();
};

extern config cfg;
extern int dbglev;
extern std::string DEBUG_DIR;
extern std::vector<cv::Mat> imgsToShow;

void drawResultIntoImage(cv::Mat& image, Result r, cv::Scalar color);

unsigned long long compareWithRec(const cv::Mat& image, const cv::Mat& tpl, int startx, int starty, int endx, int endy);

int compareLong (const void * a, const void * b);

bool fileExists(const std::string& name);

std::string removeExtension(const std::string& filename);

int findBestResultIdx(cv::Mat& image, std::vector<Result> results);

cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows);
#endif

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <climits>

#include <sys/stat.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "base.h"

#ifdef _WIN32
double round(double d) {
  return std::floor(d + 0.5);
}
#endif




#define M_PI 3.14159265358979323846

using namespace cv;
using namespace std;

#ifdef DEBUG
int dbglev = 1000;
#else
int dbglev = 0;
#endif

string DEBUG_DIR = "debug";

Pos::Pos(int x, int y): x(x), y(y) {}
Pos::Pos() {}


Result::Result(Pos center, int top, int bottom, int left, int right, float vaha):center(center), top(top), bottom(bottom), left(left), right(right), vaha(vaha) {}
Result::Result() {}

Part::Part(int from, int to): from(from), to(to) {}
Part::Part() {}

int avg(int* arr, int num) {
  int sum = 0;
  for(int i = 0; i < num; i++) {
    sum += arr[i];
  }
  return sum/num;
}

int maxDiffIdx(int* arr, int num, int val) {
  int maxval = INT_MIN;
  int idx = -1;
  for(int i = 0; i < num; i++) {
    int diff = arr[i] - val;
    if(diff < 0)
      diff = -diff;
    if(diff > maxval) {
      maxval = diff;
      idx = i;
    }
  }
  return idx;
}

vector<int> filter(vector<int> inp, int num, int wantednum) {
  vector<int> out;
  for(int i = num; i > wantednum; i--) {
    int maxidx = maxDiffIdx(&(inp[0]), i, avg(&(inp[0]),i));
    out.clear();

    for(int j = 0; j < i; j++) {
      if(j != maxidx) {
        out.push_back(inp[j]);
      }
    }
    inp = out;
  }
  return out;
}

void drawResultIntoImage(Mat& image, Result r, Scalar color) {
  line( image, Point( r.center.x - r.left, r.center.y - r.top), Point( r.center.x - r.left,  r.center.y + r.bottom), color,  2, 8 );
  line( image, Point( r.center.x + r.right, r.center.y - r.top), Point( r.center.x + r.right,  r.center.y + r.bottom), color,  2, 8 );
  line( image, Point( r.center.x - r.left , r.center.y - r.top), Point( r.center.x + r.right ,  r.center.y - r.top), color,  2, 8 );
  line( image, Point( r.center.x - r.left , r.center.y + r.bottom), Point( r.center.x + r.right ,  r.center.y + r.bottom), color,  2, 8 );
}



unsigned long long compareWithRec(const Mat& image, const Mat& tpl, int startx, int starty, int endx, int endy) {
  if(startx < 0)
    startx = 0;
  if(starty < 0)
    starty = 0;
  if(endx >= image.cols)
    endx = image.cols-1;
  if(endy >= image.rows)
    endy = image.rows-1;

  //musi to mit aspon 10px v x i v y
  if(startx >= endx-10ll || starty >= endy-10ll)
    return ULLONG_MAX;

  Mat cropped = image(Rect(startx, starty, endx-startx, endy-starty));

  resize(cropped, cropped, tpl.size());


  unsigned long long diff = 0;
  //compute sad
  for(int i=0; i<tpl.rows; i++) {
    for(int j=0; j<tpl.cols; j++) {


      unsigned char c1 = tpl.at<uchar>(i,j);
      unsigned char c2 = cropped.at<uchar>(i,j);

      unsigned long long d;
      if(c1 > c2)
        d = c1-c2;
      else
        d = c2-c1;

      diff+=d;
    }
  }
  return diff/((unsigned long long)(image.rows*image.cols));
}

int compareLong (const void * a, const void * b) {
  long long diff = ( *(long long*)a - *(long long*)b );
  if(diff > 0ll)
    return 1;
  else if(diff < 0ll)
    return -1;
  return 0;
}

bool fileExists(const string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

string removeExtension(const string& filename) {
  size_t lastdot = filename.find_last_of(".");
  if (lastdot == string::npos) return filename;
  return filename.substr(0, lastdot);
}

int findBestResultIdx(Mat& image, vector<Result> results) {
  vector<int> results_filtered;

  unsigned long long bestval = ULLONG_MAX;
  int bestidx = -1;

  // try to load template
  char cCurrentPath[FILENAME_MAX];
  getCurrentDir(cCurrentPath, sizeof(cCurrentPath));
  std::string filename = cCurrentPath;
  filename += "/templates/vryp_ctverec.png";
  Mat tpl = ourImread(filename.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
  if(!tpl.data) {
    cerr << "An output template " << filename.c_str() << " does not exist." << endl;
    exit(1);
  }

  int i = 0;
  for(vector<Result>::iterator it = results.begin(); it != results.end(); ++it) {

    vector<int> distances;
    distances.push_back((*it).top);
    distances.push_back((*it).bottom);
    distances.push_back((*it).left);
    distances.push_back((*it).right);
    vector<int> out = filter(distances, 4, 3);

    int average = avg(&(out[0]), 3);

    unsigned long long diff = compareWithRec(image, tpl, (*it).center.x - (*it).left, (*it).center.y - (*it).top, (*it).center.x + (*it).right, (*it).center.y + (*it).bottom);


    float weight = (*it).vaha;
    if(weight < 1.) {
      if(weight > 0.) {
        if(weight < 0.4)
          weight = 0.4;

        diff*=(unsigned long long)(((1./weight)*10000)/10000);
      }
    }

    if(diff < bestval) {
      //find best result
      bestval = diff;
      bestidx = i;
//	    bestavg = average;
    }

    results_filtered.push_back(average);

    i++;
  }

  return bestidx;

}

/**
    * @brief makeCanvas Makes composite image from the given images
    * @param vecMat Vector of Images.
    * @param windowHeight The height of the new composite image to be formed.
    * @param nRows Number of rows of images. (Number of columns will be calculated
    *              depending on the value of total number of images).
    * @return new composite image.
    */
cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows) {
  int N = vecMat.size();
  nRows  = nRows > N ? N : nRows;
  int edgeThickness = 10;
  int imagesPerRow = ceil(double(N) / nRows);
  int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
  int maxRowLength = 0;

  std::vector<int> resizeWidth;
  for (int i = 0; i < N;) {
    int thisRowLen = 0;
    for (int k = 0; k < imagesPerRow; k++) {
      double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
      int temp = int( ceil(resizeHeight * aspectRatio));
      resizeWidth.push_back(temp);
      thisRowLen += temp;
      if (++i == N) break;
    }
    if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
      maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
    }
  }
  int windowWidth = maxRowLength;
  cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3, Scalar(0, 0, 0));

  for (int k = 0, i = 0; i < nRows; i++) {
    int y = i * resizeHeight + (i + 1) * edgeThickness;
    int x_end = edgeThickness;
    for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
      int x = x_end;
      cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
      cv::Size s = canvasImage(roi).size();
      // change the number of channels to three
      cv::Mat target_ROI(s, CV_8UC3);
      if (vecMat[k].channels() != canvasImage.channels()) {
        if (vecMat[k].channels() == 1) {
          cv::cvtColor(vecMat[k], target_ROI, CV_GRAY2BGR);
        }
      }
      cv::resize(target_ROI, target_ROI, s);
      if (target_ROI.type() != canvasImage.type()) {
        target_ROI.convertTo(target_ROI, canvasImage.type());
      }
      target_ROI.copyTo(canvasImage(roi));
      x_end += resizeWidth[k] + edgeThickness;
    }
  }
  return canvasImage;
}

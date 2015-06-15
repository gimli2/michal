#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <sstream>
#include <climits>


#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "hough.h"
#include "base.h"


#define M_PI 3.14159265358979323846

using namespace cv;
using namespace std;

/**
 *  Get intersection point of two lines
 */
Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b) {
  int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3], x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

  if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))) {
    cv::Point2f pt;
    pt.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
    pt.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;
    return pt;
  } else
    return cv::Point2f(-1, -1);
}

/**
 *  Get angle of line in degrees
 */
float lineAngle(Vec4i l) {
  Point p1, p2;
  p1=Point(l[0], l[1]);
  p2=Point(l[2], l[3]);
  float angle = atan2(p1.y - p2.y, p1.x - p2.x)*180./M_PI;
  return angle;
}

/**
 *  Is line from left-bottom to right-top or left-top to right-bottom
 */
bool lineOrientation(Vec4i l) {
  float angle = lineAngle(l);
  int rotation_from_diagonal = (((int)angle)%90);
  return rotation_from_diagonal > 0;
}

/**
 *  Is line diagonal
 */
bool isLineDiagonal(Vec4i l) {
  float angle = lineAngle(l);
  int rotation_from_diagonal = (((int)angle)%90);
  if(rotation_from_diagonal<0)
    rotation_from_diagonal+=90;
  return rotation_from_diagonal >= 30 && rotation_from_diagonal <= 60;
}

/**
 *  Get distance of two points in pixels
 */
float pointsDistance(Point2f p1, Point2f p2) {
  float x = p1.x - p2.x;
  float y = p1.y - p2.y;
  return sqrt(x*x + y*y);
}

/**
 *  Check if point is inside image
 */
bool isInsideImage(Point2f p, const Mat& image) {
  return p.x < image.cols && p.x >= 0 && p.y >= 0 && p.y < image.rows;
}

/**
 *  draw point into image as cross
 */
void drawCross(Point2f p, Mat& image) {
  line( image, Point(p.x-10, p.y-10),
        Point(p.x+10, p.y+10), Scalar(0,0,255), 3, 8 );
  line( image, Point(p.x+10, p.y-10),
        Point(p.x-10, p.y+10), Scalar(0,0,255), 3, 8 );
}

/**
 *  preprocess image with OpenCV functions
 */
void preprocessCV(Mat& image) {
  int blursize;


  //blur image
  blursize = 6;
  blur(image, image, Size(blursize, blursize));


  //do OPENNING
  int morph_type = MORPH_OPEN;
  int erosion_size;
  Mat element;
  erosion_size=23;
  element = getStructuringElement( morph_type,
                                   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                   Point( erosion_size, erosion_size ) );
  morphologyEx( image, image, morph_type , element);


  //normalize result - we want to have the whitest part of image white ( 255 ) and the darkest part black ( 0 )
  normalize(image, image, 0, 255, NORM_MINMAX);

  element = getStructuringElement( morph_type,
                                   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                   Point( erosion_size, erosion_size ) );
  morphologyEx( image, image, morph_type , element);


  //show only the 0-124
  bitwise_not ( image, image );
  threshold(image, image, 125, 255, THRESH_TOZERO);
  bitwise_not ( image, image );
}

/**
 *  group near points into one
 *  @param points vector of points
 *  @param resolution maximal distance of two points, which can be condensated into one
 *  @return vector of condensated points
 */
vector<Point2f> removeGroups(vector<Point2f> points, float resolution) {
  vector<int> groupped(points.size(),-1);

  vector<Point2f> groups_centers;
  vector<int> groups_sizes;


  bool is_change = true;
  do {
    is_change = false;

    for( size_t i = 0; i < groups_centers.size(); i++ ) {
      //find nearest point
      int nearest_point = -1;
      float nearest_distance = FLT_MAX;
      for(size_t j = 0; j < points.size(); j++ ) {
        if(groupped[j] >= 0)
          continue;

        if(pointsDistance(points[j], groups_centers[i]) < nearest_distance) {
          nearest_distance = pointsDistance(points[j], groups_centers[i]);
          nearest_point = j;
        }
      }

      if(nearest_point >= 0) {
        if(pointsDistance(points[nearest_point], groups_centers[i]) < resolution+0.1*groups_sizes[i]) {
          //add to group
          groupped[nearest_point] = i;

          float xcenter = (groups_centers[i].x*groups_sizes[i] + points[nearest_point].x)/(groups_sizes[i]+1);
          float ycenter = (groups_centers[i].y*groups_sizes[i] + points[nearest_point].y)/(groups_sizes[i]+1);
          groups_centers[i] = Point2f(xcenter, ycenter);

          groups_sizes[i]++;
          is_change = true;
        }
      }

      if(is_change)
        break;
    }
    if(is_change)
      continue;


    //try to make new group
    for( size_t i = 0; i < points.size(); i++ ) {
      //only ungroupped elements
      if(groupped[i] >= 0)
        continue;

      //find nearest point
      int nearest_point = -1;
      float nearest_distance = FLT_MAX;
      for(size_t j = 0; j < points.size(); j++ ) {
        if(groupped[j] >= 0)
          continue;

        if(pointsDistance(points[j], points[i]) < nearest_distance) {
          nearest_distance = pointsDistance(points[j], points[i]);
          nearest_point = j;
        }
      }


      if(nearest_point >= 0) {
        if(pointsDistance(points[nearest_point], points[i]) < resolution) {
          int newgroupidx = groups_centers.size();

          float xcenter = (points[nearest_point].x + points[i].x)/2.;
          float ycenter = (points[nearest_point].y + points[i].y)/2.;

          groups_centers.push_back(Point2f(xcenter, ycenter));
          groups_sizes.push_back(2);

          groupped[i] = newgroupidx;
          is_change = true;
        }
      }

      if(is_change)
        break;

    }
  } while(is_change);

  vector<Point2f> ret;
  for( size_t i = 0; i < points.size(); i++ ) {
    //only ungroupped elements
    if(groupped[i] >= 0)
      continue;
    ret.push_back(points[i]);
  }

  for( size_t i = 0; i < groups_centers.size(); i++ ) {
    ret.push_back(groups_centers[i]);
  }

  return ret;
}


/**
 *  compute intersects to get all possible corners
 *  @return vector of intersection points
 */
vector<Point2f> getOrthogonalIntersections(const vector<Vec4i>& lines, Mat& original_image) {
  vector<Point2f> points;
  points.clear();
  for( size_t i = 0; i < lines.size(); i++ ) {
    if(!isLineDiagonal(lines[i]))
      continue;
    for( size_t j = 0; j < lines.size(); j++ ) {
      if(!isLineDiagonal(lines[j]))
        continue;

      Point2f intersec = computeIntersect(lines[i], lines[j]);
      if(!isInsideImage(intersec, original_image))
        continue;

      //only orthogonal lines
      if(lineOrientation(lines[i]) == lineOrientation(lines[j])) {
        continue;
      }

      points.push_back(intersec);
    }
  }
  return points;
}

void parseHough(Mat& image, Mat& original_image, Result& retres, float& is_found, int minLineLength, int maxLineGap, int threshold_val, bool dump_debug, string debug_prefix) {

  //apply prewitt operator
  float prewittkern[]= {
    -1.,  0, 1.,
    -1.,  0, 1.,
    -1.,  0, 1.
  };

  int kernelSize;
  Mat kernel;
  Mat ret;
  Mat draw;
  Mat draw_inverted;



  Mat image_transformed;

  image.convertTo(image_transformed, CV_32F);


  kernelSize=3;
  kernel = Mat(kernelSize,kernelSize, CV_32F, prewittkern);

  //apply prewitt operator
  cv::filter2D(image_transformed, ret, -1, kernel);



  //find minimum and maximum intensities
  double minVal, maxVal;
  minMaxLoc(ret, &minVal, &maxVal);


  //convert to 0-255 grayscale
  ret.convertTo(draw, CV_8U, 255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));


  //invert result to next image
  subtract(Scalar::all(255.),draw,draw_inverted);

  //threshold
  threshold( draw, draw, 127, 255,THRESH_TRUNC);
  threshold( draw_inverted, draw_inverted, 127, 255,THRESH_TRUNC);


  //mix in inverted and original images
  float alpha = 0.5;
  float beta = ( 1.0 - alpha );
  addWeighted( draw, alpha, draw_inverted, beta, 0.0, draw);



  int erosion_size = 1;
  Mat element = getStructuringElement(cv::MORPH_CROSS,
                                      cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                      cv::Point(erosion_size, erosion_size) );

  //erode image to make lines more visible
  erode(draw,draw,element);

  threshold( draw, draw, threshold_val, 255,THRESH_BINARY_INV );

  Mat dst;
  draw.convertTo(dst, CV_8U);

  vector<Vec4i> lines;
  lines.clear();

  //get lines from hough line transform
  HoughLinesP( dst, lines, 1, CV_PI/45, 100, minLineLength, maxLineGap );


  Point p1, p2;
  //draw lines into image
  for( size_t i = 0; i < lines.size(); i++ ) {

    if(!isLineDiagonal(lines[i])) {
      continue;
    }

    line( original_image, Point(lines[i][0], lines[i][1]),
          Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
  }

  //compute intersects to get all possible corners
  vector<Point2f> points = getOrthogonalIntersections(lines, original_image);

  if(dump_debug) {
    Mat im;
    image.copyTo(im);

    //draw possible centers of corners
    for( size_t i = 0; i < points.size(); i++ )
      drawCross(points[i], im);

    stringstream ss;
    ss << debug_prefix.c_str() << "_points_before.jpg";
    imwrite(ss.str().c_str(), im);
  }

  if(dbglev > 0) {
    cout << " ------------ "<< endl;
    cout << " Hough_points before groupping: "<<points.size()<<endl;
  }

  //condensate almost similar points (near) to lower points
  points = removeGroups(points,15.);


  if(dump_debug) {
    Mat im;
    image.copyTo(im);

    //draw possible centers of corners
    for( size_t i = 0; i < points.size(); i++ )
      drawCross(points[i], im);

    stringstream ss;
    ss << debug_prefix.c_str() << "_points_after.jpg";
    imwrite(ss.str().c_str(), im);
  }



  if(dbglev > 0) {
    cout << " Hough_points after groupping: "<<points.size()<<endl;
  }
  vector<Result> results;

  int tries = 0;
  const int MAX_TRIES = 500;

  if(points.size() < 50)	//kdyz je jich vic, tak uz je to moc a dlouho to trva :(
    for( size_t i = 0; i < points.size() && tries < MAX_TRIES; i++ )
      //left corner
    {


      float startx = points[i].x;
      for( size_t j = 0; j < points.size() && tries < MAX_TRIES; j++ )
        //right corner
      {

        float endx = points[j].x;
        //is on right side
        if(endx < startx+40)
          continue;

        float ydiff = points[i].y - points[j].y;
        if(ydiff < 0)
          ydiff = -ydiff;

        for( size_t k = 0; k < points.size() && tries < MAX_TRIES; k++ )
          //top corner
        {
          //is not same as left or right
          if(k == j || k == i)
            continue;

          //is between left and right
          if(points[k].x < startx || points[k].x > endx)
            continue;

          float starty = points[k].y;

          Vec4i lineBetween;
          lineBetween[0] = points[k].x;
          lineBetween[1] = points[k].y;
          lineBetween[2] = points[i].x;
          lineBetween[3] = points[i].y;
          //line from left to top point must be diagonal
          if(!isLineDiagonal(lineBetween))
            continue;

          lineBetween[2] = points[j].x;
          lineBetween[3] = points[j].y;
          //line from right to top point must be diagonal
          if(!isLineDiagonal(lineBetween))
            continue;


          for( size_t l = 0; l < points.size() && tries < MAX_TRIES; l++ )
            //bottom bod
          {
            //is not same as previous
            if(l == k || l == j || l == i)
              continue;

            //between left and right
            if(points[l].x < startx || points[l].x > endx)
              continue;

            float endy = points[l].y;
            //bottom must be under top
            if(endy < starty+40)
              continue;

            float xdiff = points[k].x - points[l].x;
            if(xdiff < 0)
              xdiff = -xdiff;

            float ratio = (endx-startx)/(endy-starty);
            //shall be ideally square
            if(ratio < 0.5 || ratio > 2)
              continue;

            lineBetween[0] = points[l].x;
            lineBetween[1] = points[l].y;
            if(!isLineDiagonal(lineBetween))
              continue;

            Pos center((endx+startx)/2,(endy+starty)/2);

            //if result is not ideal square >> make penalty for it
            float weight = 1.;
            if(xdiff > 40.)
              weight *= (1/sqrt((xdiff-20.)/20));
            if(ydiff > 40.)
              weight *= (1/sqrt((ydiff-20.)/20));

            tries++;
            //make possible result from these corners i,j,k,l
            results.push_back(Result(center, center.y - starty,endy-center.y,center.x-startx,endx-center.x, weight));
          }
        }
      }
    }

  //find best match from possible results
  int bestidx = findBestResultIdx(image, results);

  if(dbglev > 0) {
    cout << "tries: "<<tries<<endl;
  }

  if(bestidx >= 0) {
    is_found = true;
    retres = results[bestidx];

    Result bestres = results[bestidx];

    if(dbglev > 0) {
//	    cout << " ------------ "<< endl;

      cout << " hough - best result [" << bestres.center.x<<","<< bestres.center.y << "] "<< bestres.top << ","<< bestres.bottom<< ","<< bestres.left<< ","<< bestres.right << endl;

      cout << " writing it to original image "<< endl;
    }

    Scalar c = Scalar(255, 0, 0);
    drawResultIntoImage(image, bestres, c);

    if(dbglev > 0) {
      cout << " writing it to original image "<< endl;
    }

    c = Scalar(255, 255, 0);
    drawResultIntoImage(original_image, bestres, c);

  } else {
    is_found = false;
  }
}

/**
 *  Get all results of hough parametrised transformations
 *  @param im image to search results
 *  @param ret out parameter list of results to add
 *  @param added out parameter of count of added results of Hough transforms
 *  @param tries out parameter of count of tried results of Hough transforms
 */
void findHoughResults(Mat im, vector<Result>& ret, int & added, int& tries) {
  Mat preprocessed;
  im.copyTo(preprocessed);

  preprocessCV(preprocessed);

  Result bestres;
  float is_found;


  for(int i = 20; i <= 70; i+=10) {   //50
    //min line length
    for(int j = 10; j >= 4; j-= 2) { //5
      for(int k = 105; k <= 115; k+=5) {  //110
        stringstream imname;
        imname << DEBUG_DIR << "/hough_" << i << "_" << j << "_" << k;

        //max line gap
        Mat orig,image;
        im.copyTo(orig);
        preprocessed.copyTo(image);

        tries++;
        parseHough(image, orig, bestres, is_found,i,j,k, dbglev > 2, imname.str());
        if(is_found) {
          added++;
          ret.push_back(bestres);
        }

        //Dump output file into debug folder
        if(dbglev > 2) {
          stringstream ss;
          ss << imname.str().c_str() << "_orig.jpg";

          stringstream ss2;
          ss2 << imname.str().c_str() << "_parsed.jpg";

          imwrite(ss.str(), orig);
          imwrite(ss2.str(), image);
        }
      }
    }
  }
}

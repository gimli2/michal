#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <climits>


#include "base.h"
#include "naive.h"
#include "hough.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>


using namespace cv;
using namespace std;

/*****************************************************************************/
/**
 * Remove smaller areas than maxareasize from image.
 * Area is considered as an darker than 255
 */
void removeSmallContinuousAreas(Mat& image, unsigned int maxareasize) {
  int sizestatus = image.rows;
  if(image.cols > sizestatus)
    sizestatus = image.cols;

  char** status = new char*[sizestatus];
  for(int i = 0; i < sizestatus; i++) {
    status[i] = new char[sizestatus];
    memset(status[i], 0, sizestatus);
  }

  if(dbglev > 0)
    cout << "Finding continous areas > " << endl;

  vector<vector<Pos> > areas;

  //loop through whole image
  for(int i = 0; i < image.rows; i++) {
    for(int j = 0; j < image.cols; j++) {
      if(status[i][j] != 0)
        continue;
      if(image.at<uchar>(i,j) == 255)
        continue;

      vector<Pos> q;
      q.push_back(Pos(i, j));

      status[i][j] = 1;	//already visited

      unsigned int startidx = 0;

      while(startidx < q.size()) {
        Pos p = q[startidx++];
        for(int k = -1; k <= 1; k++) {
          for(int l = -1; l <= 1; l++) {
            if(k == 0 && l == 0)
              continue;

            int newx = p.x+k;
            int newy = p.y+l;

            if(newx < 0 || newx >= image.rows)
              continue;
            if(newy < 0 || newy >= image.cols)
              continue;

            if(status[newx][newy] != 0)
              continue;


            if(image.at<uchar>(newx,newy) != 255)
              q.push_back(Pos(newx,newy));

            status[newx][newy] = 1;	//already visited
          }
        }
      }

      //push to continous areas list
      areas.push_back(q);
    }
  }

  //loop through all areas
  for(vector<vector<Pos> >::iterator it = areas.begin(); it != areas.end(); ++it) {
    vector<Pos>& q = *it;
    //remove areas smaller than maxareasize
    if(q.size() < maxareasize) {
      for(vector<Pos>::iterator it2 = q.begin(); it2 != q.end(); ++it2) {
        Pos p = *it2;

        image.at<uchar>(p.x,p.y) = 255;
      }
    }
  }

  for(int i = 0; i < sizestatus; i++) {
    delete[] status[i];
  }
  delete[] status;
}
/*****************************************************************************/
/**
 * @brief Preprocess image (extract background)
 * @param image image to preprocess
 */
void preprocess(Mat& image) {
  // how much (%) values below histogram peak treshold will be also filtered out
  double filterTolerance = 0.25;

  //const int blursize = 6;
  //blur(image, image, Size(blursize, blursize));

  // prepare arrays full of 0s for histograms
  int pixelscnt[256];
  memset(pixelscnt, 0, sizeof(*pixelscnt)*256);
  int pixelscntapprox[256];
  memset(pixelscntapprox, 0, sizeof(*pixelscntapprox)*256);

  // calculate histogram of used pixels
  for(int i=0; i<image.rows; i++) {
    for(int j=0; j<image.cols; j++) {
      pixelscnt[(int)image.at<uchar>(i,j)]++;
    }
  }

  int maxVal = INT_MIN;
  int maxIdx = 0;
  // smoothen the histogram and find maximum in smoothen
  int epsilon = 4;
  for(int i = 0; i < 256; i++) {
    int sum = 0;
    int cnt = 0;
    for(int j = i-epsilon; j <= i+epsilon+1; j++) {
      if(j >= 0 && j < 256) {
        sum += pixelscnt[j];
        cnt++;
      }
    }
    //cout << "maxval " << maxVal << " pixel["<<i<<"] " << pixelscntapprox[i] << "sum="<<sum<<" cnt="<<cnt<<endl;
    pixelscntapprox[i] = (sum/cnt);
    if(pixelscntapprox[i] > maxVal) {
      maxVal = pixelscntapprox[i];
      maxIdx = i;
    }
  }

  // colorfull image for histograms
  Mat imc(image.size(), CV_8UC3);
  cvtColor(image, imc, CV_GRAY2BGR);

  // draw histograms
  int xoff = 200;
  rectangle(imc, Rect(xoff, 0, 256, 200), Scalar(0, 0, 0), CV_FILLED, 8, 0);
  for(int i=0; i<256; i++) {
    line(imc,Point(xoff+i,202), Point(xoff+i, 210), Scalar(i, i, i), 2, 8, 0);
  }
  int lastVal = 0;
  for(int i=0; i<256; i++) {
    int val = (int) (200.0 * ((double)pixelscnt[i] / maxVal));
    line(imc,Point(xoff+i-1,200-lastVal), Point(xoff+i, 200-val), Scalar(255, 100, 100), 1, CV_AA, 0);
    lastVal = val;
  }
  lastVal = 0;
  for(int i=0; i<256; i++) {
    int val = (int) (200.0 * ((double) pixelscntapprox[i] / maxVal));
    line(imc,Point(xoff+i-1,200-lastVal), Point(xoff+i, 200-val), Scalar(0, 0, 255), 1, CV_AA, 0);
    lastVal = val;
  }

  // filter out pixels lighter than (most frequent value - 50)
  int absTolerance = (int) ((100.0 * filterTolerance) * 2.56);
  int lowBound = maxIdx - absTolerance;
  line(imc,Point(xoff+lowBound, 0), Point(xoff+lowBound, 200), Scalar(0, 255, 0), 1, CV_AA, 0);
  line(imc,Point(xoff+maxIdx, 180), Point(xoff+maxIdx, 200), Scalar(0, 255, 0), 2, CV_AA, 0);
  cout << "tolerance=" << absTolerance << " maxIdx=" << maxIdx << " lower abs limit=" << lowBound << endl;
  imshow("Result", imc);
  waitKey(0);

  for(int i=0; i<image.rows; i++) {
    for(int j=0; j<image.cols; j++) {
      if((int)image.at<uchar>(i,j) >= lowBound) image.at<uchar>(i,j) = 255;
    }
  }

  imshow("Result", image);
  waitKey(0);

  // blur
  const int blursize2 = 3;
  blur(image, image, Size(blursize2, blursize2));

  // remove small continuous areas
  removeSmallContinuousAreas(image, 30*1000);
  imshow("Result", image);
  waitKey(0);
}

/*****************************************************************************/
/**
 * Find x and y thresholds
 */
void findXYLimits(Mat& image,vector<Part>& xparts,vector<Part>& yparts,int thresholdoffset, bool dump_debug, string debug_prefix) {
  long long * rowssum = new long long[image.rows];
  memset(rowssum, 0, sizeof(*rowssum)*image.rows);
  long long * rowssum2 = new long long[image.rows];
  memset(rowssum2, 0, sizeof(*rowssum)*image.rows);

  long long * colssum = new long long[image.cols];
  memset(colssum, 0, sizeof(*colssum)*image.cols);
  long long * colssum2 = new long long[image.cols];
  memset(colssum2, 0, sizeof(*colssum)*image.cols);

  //calculate sum of rows and cols
  long long totalsum = 0;
  for(int i=0; i<image.rows; i++) {
    for(int j=0; j<image.cols; j++) {
      int val = 255 - (int)image.at<uchar>(i,j);
      totalsum += val;
      rowssum[i] += val;
      colssum[j] += val;
      rowssum2[i] += val;
      colssum2[j] += val;
    }
  }

  if(dump_debug) {
    stringstream ss;
    ss << debug_prefix << "_rows.data";

    ofstream ofs (ss.str().c_str(), ofstream::out);
    for(int i=0; i<image.rows; i++) {
      ofs << i << "\t" << rowssum[i] << "\t" << rowssum2[i]<<endl;
    }
    ofs.close();


    stringstream ss2;
    ss2 << debug_prefix << "_cols.data";

    ofstream ofs2 (ss2.str().c_str(), ofstream::out);
    for(int i=0; i<image.cols; i++) {
      ofs2 << i << "\t" << colssum[i] << "\t" << colssum2[i]<<endl;
    }
    ofs2.close();
  }


  // sort because we want to get median
  qsort (colssum2, image.cols, sizeof(long long), compareLong);
  qsort (rowssum2, image.rows, sizeof(long long), compareLong);

  int totalavg = (int)(totalsum/((long long)(image.rows*image.cols)));


  //calculate threshold values for x and y parts
  const int thresholdrows = rowssum2[image.rows/2]+totalavg+thresholdoffset;
  const int thresholdcols = rowssum2[image.cols/2]+totalavg+thresholdoffset;




  Mat pars;
  if(dump_debug)
    image.copyTo(pars);


  int startpos = 0;

  /********* thresholds for y parts **************/

  bool is = rowssum[0] > thresholdrows;
  if(is)
    startpos = 0;
  for(int i=1; i<image.rows; i++) {
    bool newis = rowssum[i] > thresholdrows;

    if(newis != is) {
      if(dbglev > 10) {
        line( pars, Point( 10, i), Point( image.cols-10, i), Scalar( 0, 0, 0 ),  2, 8 );
      }

      if(!newis)
        yparts.push_back(Part(startpos, i-1));
      else
        startpos = i;
    }
    is = newis;
  }
  if(is) {
    yparts.push_back(Part(startpos, image.rows-1));
  }


  /********* thresholds for x parts **************/
  is = colssum[0] > thresholdcols;
  if(is)
    startpos = 0;
  for(int i=1; i<image.cols; i++) {
    bool newis = colssum[i] > thresholdcols;

    if(newis != is) {
      if(dbglev > 10)
        line( pars, Point( i, 10), Point( i,  image.rows-10), Scalar( 0, 0, 0 ),  2, 8 );

      if(!newis)
        xparts.push_back(Part(startpos, i-1));
      else
        startpos = i;
    }
    is = newis;
  }
  if(is) {
    xparts.push_back(Part(startpos, image.cols-1));
  }



  /********** dump output image into debug folder ************/
  if(dump_debug) {
    stringstream ss;
    ss << debug_prefix << "_thresholds.jpg";
    imwrite(ss.str().c_str(), pars);
  }


  /************* cleanup the mess ****************/
  delete [] rowssum;
  delete [] colssum;
  delete [] rowssum2;
  delete [] colssum2;
}

/*****************************************************************************/
/**
 * Search left, right, top and bottom for the first non white pixel and push the calculated result into results vector
 */
void findDimensionsForCenter(Mat& image, Pos& center, vector<Result>& results) {
  int x = center.x;
  int y = center.y;

  int top = -1;
  int bottom = -1;
  int left = -1;
  int right = -1;

  for(int i = 0; i < 4; i++) {
    x = center.x;
    y = center.y;

    bool white = true;
    bool old_white = true;

    while(1) {
      switch(i) {
      case 0:	//top
        y--;
        break;
      case 1:	//bottom
        y++;
        break;
      case 2:	//left
        x--;
        break;
      case 3:	//right
        x++;
        break;
      }

      if(x < 0 || x >= image.cols)
        break;
      if(y < 0 || y >= image.rows)
        break;

      white = image.at<uchar>(y,x) >= 255;

      if(white && !old_white) {
        switch(i) {
        case 0:	//top
          top = center.y - y;
          break;
        case 1:	//bottom
          bottom = y - center.y;
          break;
        case 2:	//left
          left = center.x - x;
          break;
        case 3:	//right
          right = x - center.x;
          break;
        }
        break;
      }

      old_white = white;
    }
  }

  results.push_back(Result(center, top, bottom, left, right));
}

/*****************************************************************************/
/**
 * parse one possible part of threshold
 */
void parsePossibleOnePart(Mat& image, vector<Result>& results, const Part& xpart, const Part& ypart) {
  int sizestatus = image.rows;
  if(image.cols > sizestatus)
    sizestatus = image.cols;

  char** status = new char*[sizestatus];
  for(int i = 0; i < sizestatus; i++) {
    status[i] = new char[sizestatus];
    memset(status[i], 0, sizestatus*sizeof(char));
  }

  int sizex = (xpart.to-xpart.from);
  int sizey = (ypart.to-ypart.from);

  int centerx = xpart.from + sizex/2;
  int centery = ypart.from + sizey/2;

  queue<Pos> q;

  status[centerx][centery] = 1;
  q.push(Pos(centerx, centery));

  bool doing = true;

  Pos p;
  while(!q.empty() && doing) {
    p = q.front();

    q.pop();

    for(int i = -1; i <= 1 && doing; i++) {
      for(int j = -1; j <= 1 && doing; j++) {
        if(i == 0 && j == 0)
          continue;
        int newx = p.x + i;
        int newy = p.y + j;

        if(newx < xpart.from || newx > xpart.to)
          continue;
        if(newy < ypart.from || newy > ypart.to)
          continue;

        if(status[newx][newy] != 0)
          continue;

        status[newx][newy] = 1;
        q.push(Pos(newx, newy));

        if(image.at<uchar>(newy,newx) == 255u) {
          doing = false;

          p = Pos(newx, newy);
        }
      }
    }
  }

  if(!doing) {	//nalezena dira

    for(int i = 0; i < sizestatus; i++) {
      memset(status[i], 0, sizestatus);
    }

    queue<Pos> q2;
    q2.push(Pos(p.x, p.y));
    status[p.x][p.y] = 1;

    queue<Pos> qcorner;

    while(!q2.empty()) {
      Pos p2 = q2.front();
      q2.pop();

      for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
          if(i == 0 && j == 0)
            continue;
          int newx = p2.x + i;
          int newy = p2.y + j;

          if(newx < xpart.from || newx > xpart.to)
            continue;
          if(newy < ypart.from || newy > ypart.to)
            continue;

          if(status[newx][newy] != 0)
            continue;


          status[newx][newy] = 1;
          if(image.at<uchar>(newy,newx) == 255) {
            q2.push(Pos(newx, newy));


            bool is_corner = false;
            for(int k = -1; k <= 1 && !is_corner; k++) {
              for(int l = -1; l <= 1 && !is_corner; l++) {
                if(k == 0 && l == 0)
                  continue;
                int newx2 = newx+k;
                int newy2 = newy+l;
                if(image.at<uchar>(newy2, newx2) != 255) {
                  is_corner = true;
                  status[newx][newy] = 2;
                  qcorner.push(Pos(newx, newy));
                }
              }
            }
          }

        }
      }
    }

    vector<Pos> centers;

    //find center(s) of an area
    while(!qcorner.empty()) {
      Pos p2 = qcorner.front();
      qcorner.pop();
      for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
          if(i == 0 && j == 0)
            continue;


          int newx = p2.x + i;
          int newy = p2.y + j;

          if(newx < 0 || newy < 0 || newx >= image.rows || newy >= image.cols)
            continue;


          if(status[newx][newy] != 1)
            continue;

          status[newx][newy] = 2;
          qcorner.push(Pos(newx, newy));

          bool is_center = true;
          for(int k = -1; k <= 1; k++) {
            for(int l = -1; l <= 1; l++) {
              if(k == 0 && l == 0)
                continue;
              int newx2 = newx+k;
              int newy2 = newy+l;

              if(newx2 < 0 || newy2 < 0 || newx2 >= image.rows || newy2 >= image.cols)
                continue;

              if(status[newx2][newy2] != 2)
                is_center = false;
            }
          }

          if(is_center) {
            centers.push_back(Pos(newx, newy));
          }

        }
      }
    }

    for(vector<Pos>::iterator it = centers.begin(); it != centers.end(); ++it) {
      findDimensionsForCenter(image, *it, results);
    }

  }

  for(int i = 0; i < sizestatus; i++) {
    delete[] status[i];
  }
  delete[] status;
}

/*****************************************************************************/

void parse(Mat& image, Mat& original_image,Result& retres,float& is_found, int thresholdoffset, bool dump_debug, string debug_prefix) {
  vector<Part> yparts;
  vector<Part> xparts;

  findXYLimits(image,xparts,yparts,thresholdoffset, dump_debug, debug_prefix);


  vector<Result> results;



  //iterate through x and y limits
  for(vector<Part>::iterator xit = xparts.begin(); xit != xparts.end(); ++xit) {
    for(vector<Part>::iterator yit = yparts.begin(); yit != yparts.end(); ++yit) {
      Part xpart = *xit;
      Part ypart = *yit;

      int sizex = (xpart.to-xpart.from);
      int sizey = (ypart.to-ypart.from);

      int size = sizex*sizey;
      if(size < 30*30) {	//too small
        continue;
      }

      float sizediff;
      if(sizex > sizey)
        sizediff = sizex / sizey;
      else
        sizediff = sizey / sizex;

      if(sizediff > 1.2) { //not rectangle
        continue;
      }

      parsePossibleOnePart(image, results, xpart, ypart);
    }
  }


  //find best match from possible results
  int bestidx = findBestResultIdx(image, results);

  if(bestidx >= 0) {
    is_found = true;
    retres = results[bestidx];

    Result bestres = results[bestidx];

    if(dbglev > 0) {
      cout << " ------------ "<< endl;
      cout << " naive - best result [" << bestres.center.x<<","<< bestres.center.y << "] "<< bestres.top << ","<< bestres.bottom<< ","<< bestres.left<< ","<< bestres.right << endl;

    }

    Scalar c = Scalar(0,0,255);
    drawResultIntoImage(image, bestres, c);

    if(dbglev > 0) {
      cout << " writing it to original image "<< endl;
    }

    c = Scalar(0,255,255);
    drawResultIntoImage(original_image, bestres, c);
  } else {
    is_found = false;
  }
}

/*****************************************************************************/
/**
 *  @brief Get all results of parametrised native
 *  @param im image to search results
 *  @param ret out parameter list of results to add
 *  @param added out parameter of count of added results of native
 *  @param tries out parameter of count of tried results of native
 */
void findNaiveResults(Mat im, vector<Result>& ret, int& added, int& tries) {
  Mat preprocessed, druhy;
  im.copyTo(preprocessed);
  im.copyTo(druhy);

  preprocess(preprocessed);
  imshow("Result", preprocessed);
  waitKey(0);

  preprocessCV(druhy);
  imshow("Result", druhy);
  imshow("Result2", preprocessed);
  waitKey(0);

  Result bestres;
  float is_found;

  //500
  for(int i = 300; i <= 900; i+= 200) {
    stringstream naivename;
    naivename<<DEBUG_DIR<< "/naive_" << i;

    Mat orig,image;
    im.copyTo(orig);
    preprocessed.copyTo(image);

    tries++;

    parse(image, orig, bestres, is_found,i, dbglev > 2, naivename.str());
    if(is_found) {
      added++;
      ret.push_back(bestres);
    }

    //Dump output file into debug folder
    if(dbglev > 2) {
      stringstream ss;
      ss << naivename.str() << "_orig.jpg";

      imwrite(ss.str().c_str(), orig);

      stringstream ss2;
      ss2<< naivename.str() << "_parsed.jpg";

      imwrite(ss2.str().c_str(), image);
    }

  }
}

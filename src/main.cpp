#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <climits>

#include <unistd.h>
#include <getopt.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>


#include "base.h"
#include "hough.h"
#include "naive.h"
#include "stat.h"
#include "hardness.h"


#ifdef _WIN32
#include<conio.h>
#endif

using namespace cv;
using namespace std;

std::vector<cv::Mat> imgsToShow;

config cfg;

/**
\brief Print help and usage informations.
*/
void printUsage(int exit_code, char* const* argv) {
  cout << "MICHAL - MICro Hardness AnaLysis." << endl << endl;
  cout << "Aplication for analysis of micro hardness measurements." << endl << endl;
  cout << "Usage: " << argv[0] << " [options] -i INPUTFILE" << endl << endl;
  cout << "Common options:" << endl;
  cout << "  -h  --help" << endl;
  cout << "       Show this help." << endl;
  cout << "  -d  --dbglev" << endl;
  cout << "       Debug level (default 0)." << endl;
  cout << "  -i  --input" << endl;
  cout << "       Image filename to analyse." << endl;
  cout << "  -o  --output" << endl;
  cout << "       Image filename to write detected pattern and results." << endl;
  cout << "  -t  --text-output" << endl;
  cout << "       Filename to write text results." << endl << endl;
  cout << "Preprocess options:" << endl;
  cout << "  -f  --pp-filter-tolerance (default 0.25)" << endl;
  cout << "  -e  --pp-hist-smooth (default 4)" << endl;
  cout << "  -b  --pp-blur (default 3)" << endl;
  cout << "  -s  --pp-small-areas-size (default 30000)" << endl;
  exit(exit_code);
}

/**
\brief Read and parse command line parameters.
*/
void readParams(int argc, char* const* argv, config* cfg) {
  int next_option;
  // ":" after character = parameter needs argument
  const char* const short_options = "hd:i:o:t:";
  const struct option long_options[] = {
    // long-name, obligation, NULL = getopt_long returns val, val to return
    // common
    { "help",       no_argument,        NULL, 'h'},
    { "dbglev",     required_argument,  NULL, 'd'},
    { "input",      required_argument,  NULL, 'i'},
    { "output",     required_argument,  NULL, 'o'},
    { "textoutput", required_argument,  NULL, 't'},
    // preprocess
    { "pp-filter-tolerance", required_argument,  NULL, 'f'},
    { "pp-hist-smooth", required_argument,  NULL, 'e'},
    { "pp-blur", required_argument,  NULL, 'b'},
    { "pp-small-areas-size", required_argument,  NULL, 's'},

    { NULL,         no_argument,        NULL, 0} /* Required at end of array.  */
  };

  // default values
  cfg->dbglev = 0;
  cfg->ppBlur = 3;
  cfg->ppFilterTolerance = 0.25;
  cfg->ppHistogramSmooth = 4;
  cfg->ppSmallAreasSize = 30000;

  // parse command line
  do {
    next_option = getopt_long(argc, argv, short_options, long_options, NULL);
    switch (next_option) {
    case 'h': /* -h or --help */
    case '?':
      printUsage(0, argv);
      break;
    case 'd':
      cfg->dbglev = atoi(optarg);
      break;
    case 'i':
      cfg->fin = optarg;
      break;
    case 'o':
      cfg->fout = optarg;
      break;
    case 't':
      cfg->ftextout = optarg;
      break;
    case 'f':
      cfg->ppFilterTolerance = atof(optarg);
      break;
    case 'e':
      cfg->ppHistogramSmooth = atoi(optarg);
      break;
    case 'b':
      cfg->ppBlur = atoi(optarg);
      break;
    case 's':
      cfg->ppSmallAreasSize = atol(optarg);
      break;
    case -1: /* Done with options.  */
      break;
    default: /* Something else: unexpected.  */
      abort();
    }
  } while (next_option != -1);

  if (!fileExists(cfg->fin)) {
    cerr <<  "Could not find the input image (" << cfg->fin << ")." << endl ;
    exit(-2);
  }
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printUsage(-1, argv);
    return -1;
  }

  readParams(argc, argv, &cfg);

  // -------------- read image sums -------------------
  Mat image;
  image = ourImread(cfg.fin.c_str(), CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
  if(!image.data) {
    cerr <<  "Could not open or find the image" << endl ;
    return -1;
  }

  Mat orig;
  image.copyTo(orig);

  Result bestres;

  vector<Result> ret;
  int added = 0;
  int tries = 0;

  cout << "Image loaded..." << endl;
  findNaiveResults(image, ret, added, tries);
  cout << "Processing image 1/3... (naive)" << endl;
  //findHoughResults(image, ret, added, tries);
  cout << "Processing image 2/3... (Hough)" << endl;
  StatResult statres = findAndDecideBestStat(ret, tries);
  cout << "Processing image 3/3..." << endl;

  // prepare output image
  Mat imgout(image.size(), CV_8UC3);
  cvtColor(image, imgout, CV_GRAY2BGR);

  if(statres.is_match) {
    double HV = 0.1;
    double pxsize = 0.1277E-6;

//	HardnessResult hardness = round(computeHardness(statres.bestres, HV, pxsize)*1E-5)/10.;
    HardnessResult hardness = computeHardness(statres.bestres, HV, pxsize);
    hardness.hardness = round(hardness.hardness*1E-5)/10.;
    hardness.w = round(hardness.w*1E8)/100.;
    hardness.h = round(hardness.h*1E8)/100.;

//	cout << "Micro hardness is: "<<hardness.hardness << endl;
    cout << hardness.hardness << "\t = Micro hardness"<< endl;
    cout << hardness.w << "\t = Width "<<endl;
    cout << hardness.h << "\t = Height "<<endl;

    //FONT_HERSHEY_PLAIN (http://docs.opencv.org/modules/core/doc/drawing_functions.html)
    int fontFace = CV_FONT_HERSHEY_DUPLEX;
    double fontScale = 0.5;
    int thickness = 1;

    stringstream ss;
    ss << "Hardness: "<<hardness.hardness;

    string text = ss.str();
    int baseline=0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;

    //text background
    rectangle( imgout, Point( 0, 0 ),
               Point( 2*40+textSize.width+40, 40+textSize.height),
               Scalar( 255, 255, 255 ), -1, 8);
    // then put the text itself
    putText(imgout, text, Point(20, 20), fontFace, fontScale, Scalar::all(0), thickness, 8);

    ss.str(string());
    ss << "w: "<<hardness.w << "um h: " << hardness.h << "um";
    text = ss.str(); //
    putText(imgout, text, Point(20, 40), fontFace, fontScale, Scalar::all(0), thickness, 8);
    //cv::addText(image, "utf8znaky", cv::Point(100, 50), cv::fontQt("Times"));

    cout << "Successfully found a pattern." << endl;

    if(cfg.fout.length() > 0) {
      string fileout = removeExtension(cfg.fout) + ".txt";
      ofstream ofs(cfg.fout.c_str());
      ofs << hardness.hardness << "\t = Micro hardness"<< endl;
      ofs << hardness.w << "\t = Width "<<endl;
      ofs << hardness.h << "\t = Height "<<endl;
      ofs.close();
    }

    drawResultIntoImage(imgout, statres.bestres, Scalar( 0, 200, 0));

  } else {
    cerr << "Error finding pattern." << endl;
  }

  if(cfg.fout.length() > 0) {
    imwrite(cfg.fout, imgout);
    cout << "Result image (" << cfg.fout << ") written." << endl;
  } else {
    //CV_WINDOW_AUTOSIZE
    namedWindow("Result", CV_WINDOW_AUTOSIZE | CV_GUI_EXPANDED);
    resizeWindow("Result", 900, 800);
    imgsToShow.push_back(imgout);
    imshow("Result", makeCanvas(imgsToShow, 850, 3));
    cout <<  "Press any key to continue" << endl ;
    waitKey(0);
  }

  return 0;
}

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <vector>
#include <queue>
#include <climits>

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

#define dbglev 50

using namespace cv;
using namespace std;

void printUsage() {
    cout <<"MICHAL - MICro Hardness AnaLysis." << endl;
    cout << endl;
    cout <<"Usage:" << endl;
    cout <<"\tmain input.jpg output.jpg ... write results to file. (BATCH MODE)" << endl;
    cout <<"\tmain input.jpg ... display the image and print results to stdout" << endl;
    cout <<"where:" << endl;
    cout <<"\tinput.jpg  is input file for analysis." << endl;
    cout <<"\toutput.jpg is output file where found pattern will be marked." << endl;
}

int main(int argc, char* argv[])
{
    if( argc < 2) {
        printUsage();
        return -1;
    }
    if (!fileExists(argv[1])) {
        cerr <<  "Could not find the input image (" << argv[1] << ")." << endl ;
        return -1;
    }

    // -------------- read image sums -------------------
    Mat image;
    image = ourImread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
    if(! image.data ) {
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

    if(statres.is_match)
    {
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
        rectangle( imgout,
                   Point( 0, 0 ),
                   Point( 2*40+textSize.width+40, 40+textSize.height),
                   Scalar( 255, 255, 255 ),
                   -1,
                   8 );
        // then put the text itself
        putText(imgout, text, Point(20, 20), fontFace, fontScale, Scalar::all(0), thickness, 8);

        ss.str(string());
        ss << "w: "<<hardness.w << "um h: " << hardness.h << "um";
        text = ss.str(); //
        putText(imgout, text, Point(20, 40), fontFace, fontScale, Scalar::all(0), thickness, 8);
        //cv::addText(image, "utf8znaky", cv::Point(100, 50), cv::fontQt("Times"));

        cout << "Successfully found a pattern." << endl;

        if(argc >= 3) {
            string fileout = removeExtension(argv[2]) + ".txt";
            ofstream ofs(fileout.c_str());
            ofs << hardness.hardness << "\t = Micro hardness"<< endl;
            ofs << hardness.w << "\t = Width "<<endl;
            ofs << hardness.h << "\t = Height "<<endl;
            ofs.close();
        }

        drawResultIntoImage(imgout, statres.bestres, Scalar( 0, 200, 0));

    } else {
        cout << "Error finding pattern." << endl;
    }

    if(argc >= 3) {
        imwrite(argv[2], imgout);
        cout << "Result image (" << argv[2] << ") written." << endl;
    } else {
        //CV_WINDOW_AUTOSIZE
        namedWindow("Result", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
        resizeWindow("Result", 900, 800);
        imshow("Result", imgout);
        cout <<  "Press any key to continue" << endl ;
        waitKey(0);
    }

    return 0;
}

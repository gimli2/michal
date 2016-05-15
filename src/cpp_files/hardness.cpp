#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#include "../header_files/hardness.h"
#include "../header_files/base.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

const double HV = 0.1;	//indenter weight
double pxsize = 0.1277E-6; //pixel size base on the lens

HardnessResult computeHardness(Result res) {
	double dw = res.right - res.left;
	double dh = res.bottom - res.top;

	dw *= pxsize;
	dh *= pxsize;	//prepocitam velikost na metry
	dw *= 1e6;
	dh *= 1e6;		//um

	//    double HV = 0.1;	//hmotnost hrotu
	double F = HV*9.823;	//m*g

	double hardness = 0.1891*F / (dw*dh);
	hardness *= 1e6;

	HardnessResult r;
	r.hardness = hardness;
	r.w = dw;
	r.h = dh;

	return r;
}

void setLens(unsigned int magnification){
	/*
	switch (magnification)
	{
	case (40):
		pxsize = 0.1277*1E-6;
		break;
	case (60):
		pxsize = 0.0841*1E-6;
		break;
	default:
		pxsize = 0.1277*1E-6; //objektiv 40x
		break;
	}
	*/
	pxsize = (0.20432 / (magnification*magnification)) *1E-3;
}
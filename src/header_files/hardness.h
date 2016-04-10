#ifndef HARDNESS_H
#define HARDNESS_H

#include "base.h"

struct HardnessResult{
	double w;	//in um
	double h;	//in um
	double hardness;
};

HardnessResult computeHardness(Result res);
void setLens(unsigned int magnification);

#endif
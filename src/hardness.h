// prevent redefinition + #endif on the end of the file
#ifndef HARDNESS_H
#define HARDNESS_H

#include <vector>
#include "base.h"

struct HardnessResult{
    double w;	//in um
    double h;	//in um
    double hardness;
    double relativeX;
    double relativeY;
};

HardnessResult computeHardness(Result res, double HV, double pxsize);

#endif

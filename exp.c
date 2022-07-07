#include <math.h>
#include <stdlib.h>
#include "exp.h"

static double lambda;
static int threshold;

void set_exp_params(double l, int t)
{
    lambda = l;
    threshold = t;
}

double next_exp()
{
    double res;
    do res = -log(drand48()) / lambda; while(res > threshold);
    return res;
}

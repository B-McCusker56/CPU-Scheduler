#include <math.h>
#include <stdlib.h>
#include "exp.h"

static double lambda;
static int threshold;
static double alpha;

void set_exp_params(double l, int t, double a)
{
    lambda = l;
    threshold = t;
    alpha = a;
}

double next_exp()
{
    double res;
    do res = -log(drand48()) / lambda; while(res > threshold);
    return res;
}

double next_tau(double t, double tau)
{
	return alpha * t + (1 - alpha) * tau;
}

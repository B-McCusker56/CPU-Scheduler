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

int next_tau(int t, int tau)
{
	return ceil(alpha * t + (1 - alpha) * tau);
}

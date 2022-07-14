#ifndef EXP_H
#define EXP_H

void set_exp_params(double lambda, int threshold, double alpha);
double next_exp();
int next_tau(int prev_tau, int tau);

#endif

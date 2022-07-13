#ifndef EXP_H
#define EXP_H

void set_exp_params(double lambda, int threshold, double alpha);
double next_exp();
double next_tau(double prev_tau, double tau);

#endif

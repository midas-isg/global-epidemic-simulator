#ifndef RANDLIB_PAR
#define RANDLIB_PAR

long ignbin(long, double);
long ignpoi(double);
long ignbin_mt(long, double,int);
long ignpoi_mt(double, int);
double ranf(void);
double ranf_(void);
double ranf_mt(int);
void set_gen(long, long, int);
void unset_gen(void);
double sexpo_mt(int);
double sexpo(void);
long mltmod(long, long, long );
double snorm(void);
double snorm_mt(int);
double fsign(double, double );
double gengam(double, double);
double gengam_mt(double, double, int);
double sgamma(double);
double sgamma_mt(double, int);

#endif

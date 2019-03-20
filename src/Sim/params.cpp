/* params.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Parameter class, functions for latent and infectious period
/*
/* Copyright 2012, MRC Centre for Outbreak Analysis and Modelling
/* 
/* Licensed under the Apache License, Version 2.0 (the "License");
/* you may not use this file except in compliance with the License.
/* You may obtain a copy of the License at
/*
/*       http://www.apache.org/licenses/LICENSE-2.0
/*
/* Unless required by applicable law or agreed to in writing, software
/* distributed under the License is distributed on an "AS IS" BASIS,
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/* See the License for the specific language governing permissions and
/* limitations under the License.
*/

#include "params.h"

params::params() {}
params::~params() {}

short params::getLatentPeriodLength(int thread_no) { // Returned in hours
  if (latent_period_fixed) return (short) latent_period*24.0;
  else {
    double q=ranf_mt(thread_no)*latent_period_icdf_res;
    int i = (int) floor(q);
    q-=(double)i;
    q=0.5-(log((q*latent_period_icdf[i+1])+((1.0-q)*latent_period_icdf[i])));
    if (q>latent_period_cutoff) q=latent_period_cutoff;
    // Convert to hours
    q*=24.0;
    return (short) q;    
  }
}

short params::getInfectiousPeriodLength(int thread_no) { // Returned in hours
  if (infectious_period_fixed) return (short) infectious_period*24.0;
  else {
    double q=ranf_mt(thread_no)*infectious_period_icdf_res;
    int i = (int) floor(q);
    q-=(double)i;
    q=0.5-(log(q*infectious_period_icdf[i+1]+(1.0-q)*infectious_period_icdf[i]));
    if (q>infectious_period_cutoff) q=infectious_period_cutoff;
    
    // Convert to days
    q*=24.0;
    return (short) q;
  }
}




/* params.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for parameter object
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

#ifndef PARAMS_H
#define PARAMS_H

#include <stdio.h>
#include <math.h>
#include "world.h"
#include "randlib_par.h"

class world;

class params { 
  public:
    bool latent_period_fixed;           // If the latent period is fixed, then...
    double latent_period;               // FIXED latent period (alternative to ICDF)
    double latent_period_cutoff;        // Or... Maximum value of latent period (days)
    int latent_period_icdf_res;         // Number of points in Inverse CDF
    double* latent_period_icdf;         // Inverse CDF of latent period distribution
    double latent_period_mean;          // Mean of latent period (days)
    
    bool infectious_period_fixed;       // Is infectious period fixed?
    double infectious_period;           // FIXED infectious period (days)
    double infectious_period_cutoff;    // OR infectious period maximum value (days)
    int infectious_period_icdf_res;     // Number of points in infectous period inverse cdf
    double* infectious_period_icdf;     // The icdf
    double infectious_period_mean;      // Mean infectious period (days)

    double symptom_delay;               // Delay between infection and symptoms (if symptomatic) (days)

    bool infectiousness_fixed;          // Is infectiousness fixed?
    double infectiousness;              // If so, use fixed value
    int infectiousness_profile_res;     // Infectiousness over time - no. of points
    double* infectiousness_profile;     // Infectiousness
    double infectiousness_mean;         // Mean infectiousness

    int infectionWindow;                // This is the maximum time between making contacts and recovery.
                                        // Calculated, and used to allocate enough space in the rolling event window.
    
    // Simulation meta

    int timesteps_per_day;            // Number of timesteps in a day obviously
    double timestep_hours;            // Length of timestep (hours)
    int seed1,seed2;       // Length of simulation (days), and the two seeds for the random number generator.
    unsigned int no_place_types;      // Number of place-types. Usually 4. (Nursery, Primary, Secondary, Workplace)
    
    int no_seeds;                     // How many seeding events
    double* seed_long;                // Longitude of each seeding (nearest patch is used)
    double* seed_lat;                 // Latitude of each seeding
    int* seed_ts;                     // Timestep number of each seeding (seeds will be sorted in chronological order)
    int* seed_no;                     // Number of infections to seed for this event
    int next_seed;                    // index of the next seeding event to occur

    short int getLatentPeriodLength(int thread_no);        // Sample latent period length
    short int getInfectiousPeriodLength(int thread_no);    // Sample infectious period length

    params();         // Constructor
    ~params();        // Destructor
};

#endif

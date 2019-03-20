/* Includes_n_Definitions.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* general includes & definitions */

#ifndef INCLUDES_N_DEFINITIONS
#define INCLUDES_N_DEFINITIONS



#include <iostream>
#include <fstream>
#include <math.h>
#include <new>
#include <time.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif 


/* number of command line parameters */
#define COMM_PARAMS_NUM 14


/* file name buffer lengths */
#define FN_BUF_LNGTH _MAX_PATH + 1


/* means undefined parameter */
#define UNDEFINED -1


/* enables detailed method call (level 1) logging */
#define OUTPUT_LOG_L1

/* enables detailed method call (level 2) logging */
//#define OUTPUT_LOG_L2

/* enables (level 1) logging for testing purposes */
#define OUTPUT_LOG_TEST_L1

/* enables (level 2) logging for testing purposes */
//#define OUTPUT_LOG_TEST_L2


/* switches on output to console */
#define CON_OUTPUT


/* exception codes */
#define NO_COUNTRY_FOUND         1
#define TABLE_UNDEFINED          2
#define TABLE_TOO_SMALL          3
#define INDX_OUT_OF_RANGE        4
#define INCORRECT_DISTR_VECTOR   5
#define FILE_OPEN_ERROR          6


/* approximate average household size; used only to calculate size of temporary Household table */
#define AHS_TEMP_TBL 4

/* age band for population statistics */
#define AGE_BAND 5.0f

/* tolerance value to compare floats */
#define TOLER_VAL 1.0e-7f

/* lower limit for marginal age distribution deviation correction */
#define CORR_PROB_LIMIT 1e-03f

/* pi number */
#define PI 3.141592653589793

/* the mean radius of Earth */
#define GEO_MRADIUS 6371.009

/*  granularity of geographical patches (data field in Country::HostToEstablishmentMap::GeoPatchIndxMtrx) */
#define HOST_TO_EST_MAP_GEOPATCH_GRANURLTY 15 //15

/* travel distance band for host-to-establishment travel statistics */
#define TRAVEL_DIST_BAND 1.0

/* default age band for host groups in nurseries, schools, colleges, etc. */
#define SCHOOL_HOST_GROUP_AGE_BAND 1.0f

/* absolute value macro */
#define ABS(x) ((x) >= 0 ? (x) : -(x))



#endif

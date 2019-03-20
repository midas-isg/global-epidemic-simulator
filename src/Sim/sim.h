/* sim.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for main simulation algorithm.
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
#ifndef SIM_HEADER
#define SIM_HEADER
  
#define _USEMPI
#define MOVIE
#define ENABLE_LD_TRAVEL
#define CTRL_UNSET 0
#define CTRL_SINGLE_ADDR 1
#define CTRL_FIRST_LINK 2
#define CTRL_MID_LINK 3
#define CTRL_LAST_ADDR 4

#define MSG_TRAVELLER 65535
#define MSG_VISITOR 65534
#define MSG_NULL_VISITOR 65533

#define TRAVELLER 1
#define VISITOR 0
  
#define _USE_OPENMP
  
  #include "simINT64.h"
  #include "params.h"
  #include "patch.h"
  #include "math.h"
  #include "person.h"
  #include "world.h"
  #include "gps_math.h"
  #include "randlib_par.h"
  #include "initialise.h"
  #include "omp.h"
  #include "output.h"
  #include "messages.h"
  #include "vector_replacement.h"
  #include "unit.h"
  #include <signal.h>

  #include <algorithm>
  #include <sstream>
  #include <string>

  class world;
  extern int errline;
  
  void seedInfection(unsigned int count, world *w, int ls_x, int ls_y);
  void seedScheduledInfections(world* w);
  void makePlaceContactRemote(world* w, int thread_no, unsigned char country, unsigned char place_type, unsigned int place_no,
      unsigned int host_no, double t_inf, double infectiousness, double contact_time);
#ifdef MEMORY_CHECK
  void PrintMemoryInfo( world* w, DWORD processID );
#endif



#endif

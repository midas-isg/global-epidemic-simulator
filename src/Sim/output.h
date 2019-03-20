/* output.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for output-related functions
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


#ifndef OUTPUT_H
#define OUTPUT_H

#include "world.h"
#include "lodepng.h"
#include "math.h"
#include "sim.h"
#include "output.h"
#include "gps_math.h"
#include "vector_replacement.h"
#include <sstream>
#include <string>


void writeSnapshot(world *w);
void setInfected(double lon, double lat, int delta, int thread_no, world *w);  
void setImmune(double lon, double lat, int delta, int thread_no, world *w);
void updateImage(world *wo);
void saveImage(world *wo);
void logFlatfile(world *wo);
void initDB(world *w);
void logDB(world *wo);
void statsTimestep(world *wo);
void resetUnitStats(world *wo);
void resetAllUnitStats(world *wo);

#endif

/* initialise.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for all initialisation functions
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

#ifndef INITIALISE_H
#define INITIALISE_H

#include <sstream>
#include <string>
#include "sim.h"
#include "world.h"
#include "patch.h"
#include "person.h"
#include "gps_math.h"
#include "randlib_par.h"
#include "vector_replacement.h"
#include "intervention.h"
#include "unit.h"
#include "place.h"
#include "output.h"
#include "messages.h"

using std::string;

#define BRAZIL 28
#define SWITZERLAND 35
#define CHINA 37
#define GERMANY 51
#define SPAIN 60
#define FRANCE 66
#define UNITED_KINGDOM 70
#define UNITED_STATES 217

void initHouseholds(world *w);
void readInitFiles(world *w);
void loadBinaryInitFile(world* w, string file);
void loadPlaces(world *w,char* file,unsigned char country,unsigned char place_type, int*** no_est_members);
void loadHouseholdFile(world* w, string file,unsigned char country);

#endif

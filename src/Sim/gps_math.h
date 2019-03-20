/* gps_math.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for a few geospatial maths functions
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
#ifndef GPS_MATH_H
#define GPS_MATH_H

#include <stdio.h>
#include <stdlib.h>
#include "world.h"
#include "math.h"



class world;

double lsIndexToLat(double j);  
double lsIndexToLon(double i);
int latToLsIndex(double lat);
int lonToLsIndex(double lon);
bool isBetween(int x, int top, int bottom);
double haversine(double lon1, double lat1, double lon2, double lat2);
double landscan_x_distance(int y, int x1, int x2);
double landscan_y_distance(int x, int y1, int y2);
double landscan_diagonal_distance(int x1, int y1, int x2, int y2);
bool eq(double f1, double f2);
#endif

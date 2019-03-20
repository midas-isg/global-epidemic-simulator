/* gps_math.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* A few geospatial support functions
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

#include "gps_math.h"

double lsIndexToLat(double j) { return 90.0-(j/120.0); }
double lsIndexToLon(double i) { return (i/120.0)-180.0; }
int latToLsIndex(double lat) { return (int) (((90.0-lat)*120.0)); }
int lonToLsIndex(double lon) { return (int) (((lon+180.0)*120.0)); }


bool isBetween(int x, int top, int bottom) {
  return ((x>=top) && (x<=bottom));
}

bool eq(double f1, double f2) {
  return fabs(f1-f2)<1E-10;
}

// Spherical distance between to lat/lon co-ordinates. See Wikipedia!

double haversine(double lon1, double lat1, double lon2, double lat2) {
  const double R = 6371.297; // km
  const double PId180 = 3.14159265359/180.0;
  lat1*=(PId180);
  lat2*=(PId180);
  lon1*=(PId180);
  lon2*=(PId180);
  
  double dLat = (lat2-lat1);
  double dLon = (lon2-lon1);
  double a = sin(dLat/2) * sin(dLat/2) + cos(lat1) * cos(lat2) * sin(dLon/2) * sin(dLon/2); 
  if (a>1) a=1; // Numerical imprecision can cause a>1. (eg, 1 + 1E-12).
  double c = 2 * atan2(sqrt(a), sqrt(1-a));  // atan2 is inf when a>1, PI when a==1.
  return R * c;
}

double landscan_x_distance(int y, int x1, int x2) {
  double lat=lsIndexToLat(y);
  return haversine(lsIndexToLon(x1),lat,lsIndexToLon(x2),lat);

}

double landscan_y_distance(int x, int y1, int y2) {
  double lon=lsIndexToLon(x);
  return haversine(lon,lsIndexToLat(y1),lon,lsIndexToLat(y2));
}

double landscan_diagonal_distance(int x1, int y1, int x2, int y2) {
  return haversine(lsIndexToLon(x1),lsIndexToLat(y1),lsIndexToLon(x2),lsIndexToLat(y2));
}




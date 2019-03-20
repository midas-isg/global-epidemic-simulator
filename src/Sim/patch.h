/* patch.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for local and remote patch classes
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


#ifndef PATCH_H
#define PATCH_H
#include "gps_math.h"
#include "household.h"

class person;
class patch;
class localPatch;
class household;
class world;

class patch {

  public:
    unsigned short x; // Landscan grid x
    unsigned short y; // Landscan grid y
    unsigned short size; // Size in landscan squares
    unsigned short node;
    int unit;
    static double distance(patch *p1, patch *p2);
    static double distance(patch *p1, int x, int y, int size);
    static patch* getCommunityContactPatch(world *w, double r, localPatch* p);


};

class localPatch : public patch {
  public:
    
    float* q_prob;
    int* q_patch;
    unsigned int no_qpatches;

    //household** households;
    //person** people;

    household* households;
    person* people;




    int no_people;
    int no_households;
    int rem_no_households;
    float p_visitor;
    float p_traveller;

    
    
    ~localPatch();
};

#endif

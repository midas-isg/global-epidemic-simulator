/* place.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for susceptible and infectious people
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


#ifndef PERSON_H
#define PERSON_H

#include "vector_replacement.h"
#include "world.h"
#include "randlib_par.h"
#include "gps_math.h"
#include "patch.h"
#include "sim.h"
#include "household.h"
#include "intervention.h"
#include "output.h"

class localPatch;
class infectedPerson;
class person;
class patch;
class travelPlan;
class household;
class world;

class person {
  public:
    household* house;                       // Index of patch->households for the person's household
    float age;                              // The age!
    unsigned short status;                  // Status of infection
    unsigned char place_type;               // Which place is the person member of
    unsigned int place;                     // Index into that place_type (for person's country)
    unsigned short group;                   // Group number within workplace. (65535 = none)
    float susceptibility;                   // Susceptibility!
    float vaccination_mul;                  // Multiply susceptibility by this vaccination factor
    
    bool isSusceptible(int delta_status);
    float getSusceptibility(world* w,int thread_no);


#define STATUS_CONTACTED 1
#define STATUS_SUSCEPTIBLE 2
#define STATUS_IMMUNE 4
#define STATUS_HOUSEHOLD_CONTACTED 8

#define SYMPTOMATIC 16
#define DETECTED 32
#define SEVERE 64
#define PROPHYLAXED 128
#define VACCINATED 256
   
   
    person();
    ~person();
    
};

class infectedPerson {
  public:
    unsigned short n_contacts;
    infectedPerson** contacts;
    unsigned short* contact_order;
    travelPlan* travel_plan;
    float t_contact; // Time of scheduled contact, (simulation time, hours).
    float t_inf; // Length of infectious period, hours.
    person* personPointer;
    unsigned int reply_address; // Used for efficient linking of replies from different nodes.
    unsigned char flags;

    static void createTravelPlan(world* w, infectedPerson* p, int thread_no, float end_latent);
    static localPatch* getPatchForPerson(world* w, int person, unsigned char country, int thread_no);
    static void locateTravel(world* w, infectedPerson* ip, int thread_no);
    void setFlags(world* w, int thread_no);
    
    float getNextContactWhileAtHomeOrWorking(world* w, int thread_no);
    void updateStats(world* w,int thread_no, int delta_inf, int delta_imm);
    double getInfectiousness(world* w, double t,int thread_no);
    infectedPerson(world* w, int thread_no, person* p);
    ~infectedPerson();
    
  };

class travelPlan {
  public:
    unsigned char country;
    float t_start;
    float duration;
    unsigned char traveller;
    
    unsigned short travel_node;
    unsigned int travel_subperson;
    localPatch* patch;
    float x;
    float y;

    travelPlan();
    ~travelPlan();
  };

    
#endif

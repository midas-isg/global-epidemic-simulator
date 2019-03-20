/* household.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for household class.
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
#ifndef HOUSEHOLD_H
#define HOUSEHOLD_H

#include "person.h"

class household {
  public:
    float lat;                 // Latitude of household
    float lon;                 // Longitude of household
    unsigned char country;     // Country of household
    int unit;                  // index of administrative unit
    unsigned char no_people;   // No. of people in household

    int patch;                 // Index of patch that household is in
    int first_person;          // Index into patch->people of the first person in this household. (They are contiguous)
    char susc_people; // No. of susceptible people in this house. (when zero, don't do household alg).

    float pph_start;           // Start of prophylaxis (delay included)
    float pph_end;             // End of prophylax period

    float q_start;             // Start of quarantine 
    float q_end;               // End of quarantine
    
    void applyProphylaxis(world* w, int thread_no);
    void applyQuarantine(world* w, int thread_no);
    household();
    ~household();
    
};
    
#endif

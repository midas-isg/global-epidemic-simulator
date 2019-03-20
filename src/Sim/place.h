/* place.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for places (school/workplaces)
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

#ifndef ESTABLISHMENT_H
#define ESTABLISHMENT_H

#include "world.h"
#include "person.h"
#include "vector_replacement.h"
#include "messages.h"


class person;

class place {
  public:
    double lat;                 // Latitude of establishment
    double lon;                 // Longitude of establishment
    unsigned char country;
    unsigned char no_nodes;     // How many nodes are repesented
    unsigned int** no_hosts;    // No. of members in establishment for each node, for each group. (Node index Length either 1, or mpi_rank, as most ests will be on one node)
    unsigned int total_hosts;   // Total number of hosts in this establishment
    unsigned int unit;          // Admin unit containing establishment
    unsigned int no_groups;     // No. of workgroups or classes within establishment.

    unsigned int* group_member_count;       // [group] - number of members in each group. (With Extra index for group -1)
    unsigned int** group_member_node_count; // [group][node] - number of members in each node.

    person*** local_members;     // [group][person] - pointers to people in each group on *this* node.



    
    unsigned int acc_cases;     // Accumulate no. of cases
    float pph_start;           // Start of prophylaxis (delay included) (hours)
    float pph_end;             // End of prophylax period (hours)

    float closure_start;       // Start of closure period (delay included) (hours)
    float closure_end;         // End of closure period (hours)
 


    void applyProphylaxis(world* w, int thread_no, unsigned char place_type, int est_no);
    void applyProphylaxisRemote(world* w,int thread_no, float start, float end);
    void applyClosure(world* w, int thread_no, int unit, unsigned char place_type, int est_no);
    void applyClosureRemote(world* w, float start, float end);
    place();
    ~place();
    
};
    
#endif

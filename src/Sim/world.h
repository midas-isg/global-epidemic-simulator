/* world.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for the "world" object.
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

#ifndef WORLD_H
#define WORLD_H

#include "sim.h"
#include "omp.h"
#include "vector_replacement.h"
#include <string.h>
#ifndef _WIN32
  #define strnicmp strncasecmp
#endif
#include "params.h"
#include "DBOpsPar.h"
#include "person.h"
#include "messages.h"
#include "unit.h"
#include "intervention.h"
#include "place.h"
#include "output.h"


class patch;
class localPatch;
class person;
class infectedPerson;
class world;
class params;
class unit;
class intervention;
class place;

class world { // The world as this node sees it.
  public:
    params *P;
    int localPatchLookup[2160][1080];    // This is a lookup table, so you can quickly get to a local patch by landscan x,y. (Which you can convert to/from lat/lon)
                                         // NB - it is not always 1-1 - remote patches are bigger, so more than one patch[x,y] points to same object transparently - which I think is useful.
    int allPatchLookup[2160][1080];      // Lookup table, so you can quickly get to a patch index in allPatches - which will equal indexes for patch_populations.

    DBOps *db;
    int*** immune_grid;              // [x][y][thread] - number of immune people (for PNG output) on this node
    int*** infected_grid;            // [x][y][thread] - number of infected people (for PNG output) on this node
    unsigned int noLocalPatches;
    unsigned int noRemotePatches;
    unsigned int totalPatches;
    int mpi_rank;
    int mpi_size;
    lwv::vector<unsigned char>** remoteRequests;      // [thread][node]
    lwv::vector<unsigned char>** remoteReplies;       // [thread][node]
    lwv::vector<unsigned char>** placeInfMsg;     // [thread][node] - buffer messages for infection events in establishments
    lwv::vector<unsigned char>** placeClosureMsg; // [thread][node] - buffer messages for closure events in establishments
    lwv::vector<unsigned char>** placeProphMsg;   // [thread][node] - buffer messages for prophylaxis events in establishments
    unsigned char* message_in;

    // Files
    string in_path;

    FILE* ff;
    bool log_db;
    char* db_table;
    char* db_server;
    bool log_flat;
    char* ff_path;
    char* ff_file;
    bool log_movie;
    char* mv_path;
    char* mv_file;
    int log_10day_slot;
    unsigned char con_toggle;    // Toggles between 1 and 0 for contact confirmations single queue
    int infectionMod;            // Current modulo of infection sliding window

    int*** delta_place_cases;    // Change in no. of cases in a unit. [unit][thread][place]
    int*** delta_place_infs;     // Change in no. of infections in a unit. [unit][thread][place]
    int** delta_comm_cases;      // Change in no. of community cases in a unit. [unit][thread]
    int** delta_comm_infs;       // Change in no. of community infections in a unit. [unit][thread]
    int** delta_hh_cases;        // Change in no. of hosuehold cases in a unit. [unit][thread]
    int** delta_hh_infs;         // Change in no. of household infections in a unit. [unit][thread]
    int** delta_total_sympt_inf; // Change in no. of symptomatic infections in a unit. [unit][thread]
    int** delta_total_nonsympt_inf; // Chg in no. of non-symptomatic infections in a unit. [unit][thread]

    unsigned int* rep_bytes_from;
    unsigned int* req_bytes_from;
    unsigned int* est_bytes_from;
    unsigned int total_req_bytes_in;
    unsigned int total_rep_bytes_in;
    unsigned int total_est_bytes_in;

    char** node_mpi_use; // A flag for each thread, for each node, that stores if a node is involved in a remote contact process.
    int** req_base;
    int** reply_base;
    int thread_count; // Max number of OpenMP Threads
    int thread_max;   // Command-line overide for max threads;
    unsigned int T;        // Actual time(step) (hours)
    float T_day;    // Actual time(step) (days) - T/Timesteps per day gets used often.
    localPatch** localPatchList; // Local patches on this node.
    patch** allPatchList;        // All the patches, local and remote
    int* patch_populations;      // Population of each patch. Only need it at initialisation, so delete afterwards.
    int continue_status;         // A flag: >=1 = at least one node wants to continue work. 0 = everyone is totally finished.
    
    // Travel matrix
    
    int no_countries;
    float* prob_travel;        // Probability of travelling from a country
    float* prob_visit;         // Probability of being a visitor in a country
    int* destinations_count;   // For each country, how many destinations for travellers?
    float** prob_dest;         // For each country, cumulative probability of destination country
    unsigned char** prob_dest_country;   // For each country, the ids of destination countries for each probability
    int* origins_count;                  // For each country, how many origins for visitors?
    float** prob_orig;                   // For each country, cumulative probability of each origin
    unsigned char** prob_orig_country;   // For each country, the ids of origin countries for each probability

    lwv::vector<int>* patches_in_country;  //   Each patch that has people belonging to each country
    lwv::vector<int>* country_patch_pop;   //   Population in associated patch in associated country!

    int no_units;                     //   Number of administrative units
    unit* a_units;                    //   List of administrative units
    unsigned int** people_per_country_per_node; // For each country, gives number of people on each node. (Even if zero)

    intervention* interventions;
    int no_interventions;

    // Initialisation
    int no_age_bands;
    float* max_age_band;
    double* init_susceptibility;     // Susceptibility by age band

    // Establishments

    unsigned int** no_places;             // No. of establishments [country][place_type] on this node.
    lwv::vector<place*>** places;         // Array of [country][type] vectors - so we can assign NULL
                                          // to establishments absent from this node, while preserving the indexes.

    // End

#define PNG_WIDTH 2160
#define PNG_HEIGHT 1080

    char* read_buffer;
    SIM_I64 buffer_content;
    SIM_I64 buffer_pointer;
#define BUFFER_SIZE 100000000L
    
    
    unsigned char image[PNG_WIDTH*PNG_HEIGHT]; // The image for output - put here just so the message class can see it more easily.
    lwv::vector<SIM_I64>** reqHostAddresses;         // A list (per thread, per step) of addresses of infected people (on other nodes), who have requested contacts from this node
    lwv::vector<lwv::vector<unsigned short> >** reqOrders;  // For each address above, a list of "contact numbers" for successful contacts.
    lwv::vector<lwv::vector<SIM_I64> >** reqContactAddresses;  // For each address above, a list of "contact numbers" for successful contacts.
    
    lwv::vector<infectedPerson*>** confirmQueue;       // List of contacts to be confirmed after MPI message. [thread][time]  - where time = index in infectionWindow
    lwv::vector<infectedPerson*>** recoveryQueue;      // List of individuals per thread who will recover in a timestep in the future.   [thread][2] - only need a one-timestep buffer.
    lwv::vector<infectedPerson*>** contactQueue;       // List of individuals who will establish contacts. [thread][time]
    lwv::vector<infectedPerson*>** symptomQueue;       // List of individuals who will exhibit symptoms. [thread][time]
    unsigned char** buffer;
    
    ~world();
    world(int argc, char* argv[]);
};
#endif

/* messages.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for functions concerning MPI message building and comms
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


#ifndef MESSAGES_H
#define MESSAGES_H

#include "sim.h"
#include "mpi.h"
#include "world.h"
#include "output.h"
#include "person.h"
#include "patch.h"
#include "vector_replacement.h"

class world;
class person;
class infectedPerson;
class patch;

#define REQUEST     0
#define REPLY       1

void initialiseMessages(world* w);
void doMessage(world* w);
void syncAdminUnitUse(world* w);
void syncPPCPN(world* w);
void addRemoteRequest(world* w, unsigned short thread_no,patch* location_susceptible,float lon, float lat, infectedPerson* infected,float new_contact_time,unsigned short contact_no, unsigned short node);
void addTravelRequest(world* w, unsigned short thread_no,infectedPerson* infected,float new_contact_time,unsigned short contact_no, unsigned char first_flag,unsigned short travel_type);
void finaliseRemoteRequest(world* w, unsigned short thread_no, infectedPerson* infected);
void addFirstRemoteReply(world* w, unsigned short thread_no, infectedPerson* pointer, unsigned short home_node, unsigned short node, unsigned short contact_no);
void addRemoteReply(world* w, unsigned short thread_no, infectedPerson* pointer, unsigned short home_node, unsigned short node, unsigned short contact_no);
void finaliseRemoteReply(world* w, unsigned short thread_no);
void addNullVisitRequest(world* w, unsigned short thread_no,infectedPerson* infected);
void addVisitRequest(world* w, unsigned short thread_no,infectedPerson* infected,float new_contact_time,unsigned short contact_no);
void addPlaceInfectionMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country, unsigned char place_type, int establishment, int host_no,double new_contact_time, double infectiousness, double t_inf);
void addPlaceProphylaxMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country, unsigned char place_type, int establishment,float start, float end);
void addPlaceClosureMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country, unsigned char place_Type, int establishment,float start, float end);


#endif

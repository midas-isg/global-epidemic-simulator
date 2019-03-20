/* place.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Class and functions for schools and workplaces
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

#include "place.h"

place::place() {
  pph_start=-1;       // Default: prophylax is off
  closure_start=-1;   //          closure is off
  acc_cases=0;        //          accumulated cases since last intervention is zero
}

place::~place() {

}

void place::applyProphylaxis(world* w, int thread_no, unsigned char place_type, int est_no) {
  int node_index=0;     // Usually, no_nodes=1, and no_hosts[0] is the number of hosts on this node.
  if (no_nodes>1) node_index=w->mpi_rank;  // However, if no_nodes>1, then we want no_hosts[our rank]
  for (unsigned int i=0; i<no_groups; i++) {
    for (unsigned int j=0; j<group_member_node_count[i][node_index]; i++) {
      person* p = this->local_members[i][j];
      if (ranf_mt(thread_no)<w->a_units[unit].pph_social) {        // pph_social is probability of host being prophylaxed due to establishment
        if ((p->status & PROPHYLAXED) ==0) p->status += PROPHYLAXED;   // and only do it if they've not been prophylaxed already
      }
    }
  }
  if ((pph_start<=0) || (w->T>pph_end)) {
    pph_start=(float) (w->T+(w->a_units[unit].pph_delay*24.0));             // Calculate start of prophylaxis (in hours)
    pph_end=(float) (pph_start+w->T+(w->a_units[unit].pph_duration*24.0));  // And end of prophylaxis (in hours)
  }
  if (no_nodes>1) {                             // If this establishment exists on more than one node, then...
    for (int i=0; i<w->mpi_size; i++) {         // Send a message to all other nodes
      if (i!=w->mpi_rank)                       //   (except this one)
        addPlaceProphylaxMsg(w,thread_no,i,country,place_type,est_no,pph_start,pph_end);    // 13 bytes per message.
    }
  }
}

void place::applyProphylaxisRemote(world* w,int thread_no, float start, float end) {
  // This is called when a message is received saying prophylaxis has happened for this establishment, on another node.
  int node_index=0;     // Usually, no_nodes=1, and no_hosts[0] is the number of hosts on this node.
  if (no_nodes>1) node_index=w->mpi_rank;  // However, if no_nodes>1, then we want no_hosts[our rank]
  for (unsigned int i=0; i<no_groups; i++) {
    for (unsigned int j=0; j<group_member_node_count[i][node_index]; j++) {
      person* p= local_members[i][j];
      if (ranf_mt(thread_no)<w->a_units[unit].pph_social) {        // pph_social is probability of host being prophylaxed due to establishment
        if ((p->status & PROPHYLAXED) ==0) p->status += PROPHYLAXED;   // and only do it if they've not been prophylaxed already
      }
    }
  }
  if ((pph_start<=0) || (w->T>pph_end)) { // If no prophylaxis has been set up yet... (assumed true)
    pph_start=start;             // This time, use the start/end points specified
    pph_end=end;                 // In the message.
  }
}

void place::applyClosure(world* w, int thread_no, int unit, unsigned char place_type, int est_no) {
  bool apply_closure=false;
  if (w->a_units[unit].c_unit==0) {   // cases
    if (acc_cases>w->a_units[unit].c_threshold) apply_closure=true;
  } else { // Percent
    if (((double)acc_cases/(double)total_hosts)>w->a_units[unit].c_threshold) apply_closure=true;
  }
  if (apply_closure) {
    closure_start=(float) (w->T+(w->a_units[unit].c_delay*24.0));
    closure_end=(float) (closure_start+(w->a_units[unit].c_period*24.0));
    if (no_nodes>1) {
      for (int i=0; i<w->mpi_size; i++) {
        if (i!=w->mpi_rank)
          addPlaceClosureMsg(w,thread_no,i,country,place_type,est_no,closure_start,closure_end);
      }
    }
  }
}

void place::applyClosureRemote(world* w, float start, float end) {
  // This is called when a message is received saying closure has happened for this establishment, on another node.
  closure_start=start;
  closure_end=end;
}

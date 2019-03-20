/* household.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Class and functions for households.
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

#include "household.h"


household::household() {
  unit=-1;
}
household::~household() {}

void household::applyProphylaxis(world* w, int thread_no) {
  for (int i=first_person; i<no_people; i++) {
    if (ranf_mt(thread_no)<w->a_units[unit].pph_household) {
      if ((w->localPatchList[patch]->people[i].status & PROPHYLAXED)==0)
        w->localPatchList[patch]->people[i].status += PROPHYLAXED;
    } 
  }
  if ((pph_start<=0) || (w->T>pph_end)) {
    pph_start=(float) (w->T+(w->a_units[unit].pph_delay*24.0));
    pph_end=(float) (pph_start+w->T+(w->a_units[unit].pph_duration*24.0));
  }
}

void household::applyQuarantine(world* w, int thread_no) {
  if (ranf_mt(thread_no)<w->a_units[unit].q_compliance) {
    q_start=(float) (w->T+(w->a_units[unit].q_delay*24.0));
    q_end=(float) (q_start+(w->a_units[unit].q_duration*24.0));
  }
}



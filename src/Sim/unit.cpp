/* unit.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Administrative Unit class and associated functions
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

#include "unit.h"

double unit::kernel_F(unit* u, double d) {
  if (d>u->k_cut) return 0;
  else return 1.0/(1.0+pow((d/u->k_a),u->k_b));
}

unit::unit() {
  level=-1;
  country=-1;
  parent_id=-1;
  live_interventions=0;
  
  bc_deny_entry=-1;
  bc_deny_exit=-1;
  trt_mul_inf_clinical=-1;
  pph_susc=-1;
  q_compliance=-1;
}

unit::~unit() {
  delete [] new_place_cases;
  delete [] new_place_infs;
  delete hist_comm_cases;
  delete [] hist_place_cases;
  delete hist_hh_cases;
  delete contact_makers;

  delete abs_place_sympt;
  delete abs_place_sev;
  delete abs_place_sympt_cc_mul;
  delete abs_place_sev_cc_mul;

     
}

void unit::add_comm_case(world* w, unsigned int unit_no, int thread_no, bool clinical_detected) {
  if (clinical_detected) w->delta_comm_cases[unit_no][thread_no]++;
  w->delta_comm_infs[unit_no][thread_no]++;
  int id=parent_id;
  while (id!=-1) {
    if (clinical_detected) {
      w->delta_comm_cases[id][thread_no]++;
    }
    w->delta_comm_infs[id][thread_no]++;
    id=w->a_units[id].parent_id;
  }
}

void unit::add_place_case(world* w, unsigned int unit_no, unsigned char place_type, int thread_no, bool clinical_detected) {
  if (clinical_detected) w->delta_place_cases[unit_no][thread_no][place_type]++;
  w->delta_place_infs[unit_no][thread_no][place_type]++;
  int id=parent_id;
  while (id!=-1) {  // Also update parent units
    if (clinical_detected) w->delta_place_cases[id][thread_no][place_type]++;
    w->delta_place_infs[id][thread_no][place_type]++;
    id=w->a_units[id].parent_id;
  }
}

void unit::add_hh_case(world* w, unsigned int unit_no, int thread_no, bool clinical_detected) {
  if (clinical_detected) w->delta_hh_cases[unit_no][thread_no]++;
  w->delta_hh_infs[unit_no][thread_no]++;
  int id=parent_id;
  while (id!=-1) {
    if (clinical_detected) w->delta_hh_cases[id][thread_no]++;
    w->delta_hh_infs[id][thread_no]++;
    id=w->a_units[id].parent_id;
  }
}

void unit::delta_non_sympt_case(world* w, unsigned int unit_no, int thread_no,int delta) {
  w->delta_total_nonsympt_inf[unit_no][thread_no]+=delta;
  int id=parent_id;
  while (id!=-1) {
    w->delta_total_nonsympt_inf[id][thread_no]+=delta;
    id=w->a_units[id].parent_id;
  }
}

void unit::delta_sympt_case(world* w, unsigned int unit_no, int thread_no,int delta) {
  w->delta_total_sympt_inf[unit_no][thread_no]+=delta;
  int id=parent_id;
  while (id!=-1) {
    w->delta_total_sympt_inf[id][thread_no]+=delta;
    id=w->a_units[id].parent_id;
  }
}




void unit::vaccinate(world* w,unsigned int unit_no) {
  // Not totally trivial to find app people in a unit...
  if (country!=UNIVERSE) {
    for (int i=0; i<w->patches_in_country[country].size(); i++) {
      localPatch* lp = w->localPatchList[w->patches_in_country[country][i]];
      for (int j=0; j<lp->no_people; j++) {
        if (lp->people[j].house->unit==(int)unit_no) {
          if (ranf_mt(0)<v_coverage) {
            lp->people[j].status = lp->people[j].status | VACCINATED;
          } 
        }
      }
    }
  } else {
    for (unsigned int i=0; i<w->noLocalPatches; i++) {
      localPatch* lp = w->localPatchList[i];
      for (int j=0; j<lp->no_people; j++) {
        if (lp->people[j].house->unit==(int)unit_no) {
          if (ranf_mt(0)<v_coverage) {
            lp->people[j].status = lp->people[j].status | VACCINATED;
          } 
        }
      }
    }
  }
}

#define OMEGA 0.0172142063210399629504802377166
#define PIby180 0.01745329251994329576923690768489

double unit::getSeasonality(world* w, double latitude) {
  
  double temp = (seasonal_max+seasonal_min)/2.0;
  temp-=((seasonal_max-seasonal_min)/2.0)*cos(2.0*(latitude*PIby180));
  return 1.0+(temp*(cos(seasonal_temporal_offset+((double)OMEGA*(double)w->T_day))));
  
}



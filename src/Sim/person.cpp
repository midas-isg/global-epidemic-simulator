/* person.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Class and functions for susceptible and infectious people
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

#include "person.h"

person::person() {}

bool person::isSusceptible(int delta_status) {
  bool result=false;
  #pragma omp critical 
  {
    if ((status & STATUS_SUSCEPTIBLE)>0) {
      status+=delta_status;
      result=true;
    } 
  }
  return result;
}

float person::getSusceptibility(world* w,int thread_no) {
  double susc = susceptibility;
  if ((status & PROPHYLAXED)>0) {
    if (house->pph_start>=0) {
      if ((w->T>=house->pph_start) && (w->T<=house->pph_end)) {
        susc*=w->a_units[house->unit].pph_susc;
      }
    } else {
      if (w->places[house->country][place_type].at(place)->pph_start>=0) {
        if ((w->T>=w->places[house->country][place_type].at(place)->pph_start) &&
            (w->T<=w->places[house->country][place_type].at(place)->pph_end)) {
          susc*=w->a_units[house->unit].pph_susc;
        }
      }
    }
  }

  if ((status & VACCINATED)>0) {
    susc*=w->a_units[house->unit].v_m_susc;
  }
  return (float) susc;
}

person::~person() {}

void infectedPerson::updateStats(world* w,int thread_no, int delta_inf,int delta_imm) {
  
  if ((flags & SYMPTOMATIC)>0)
    w->a_units[personPointer->house->unit].delta_sympt_case(w,personPointer->house->unit,thread_no,delta_inf);
  else
    w->a_units[personPointer->house->unit].delta_non_sympt_case(w,personPointer->house->unit,thread_no,delta_inf);
  
  setInfected(personPointer->house->lon,personPointer->house->lat,delta_inf,thread_no,w);
  setImmune(personPointer->house->lon,personPointer->house->lat,delta_imm,thread_no,w);
}

infectedPerson::infectedPerson(world* w, int thread_no,person* personPtr) {
  personPointer=personPtr;
  reply_address=1; // NON-SET value - "1" can never be valid start of msg.
  travel_plan=NULL;
  contacts=NULL;
  contact_order=NULL;
  n_contacts=0;
  flags=0;
  double p_sympt_adjust = w->a_units[personPtr->house->unit].p_symptomatic;
  
  if ((personPointer->status & VACCINATED)>0) {
    p_sympt_adjust*=w->a_units[personPointer->house->unit].v_m_clin;
  }

  if ((personPointer->status & PROPHYLAXED)>0) {
    p_sympt_adjust*=w->a_units[personPointer->house->unit].pph_clin;
  }

  if (ranf_mt(thread_no)<p_sympt_adjust) {
    flags+=SYMPTOMATIC;
    if (ranf_mt(thread_no)<w->a_units[personPtr->house->unit].p_severe) {
      flags+=SEVERE;
      if (ranf_mt(thread_no)<w->a_units[personPtr->house->unit].p_detect_severe) flags+=DETECTED;
    } else {
      if (ranf_mt(thread_no)<w->a_units[personPtr->house->unit].p_detect_sympt) flags+=DETECTED;
    }
  }
}

infectedPerson::~infectedPerson() {
  if (travel_plan!=NULL) delete travel_plan;
  if (contacts!=NULL) delete [] contacts;
  if (contact_order!=NULL) delete [] contact_order;
}

double infectedPerson::getInfectiousness(world* w, double t,int thread_no) { 
  double inf_ness = w->P->infectiousness;
  if (!w->P->infectiousness_fixed) 
    inf_ness = w->P->infectiousness_profile[(int)((t-w->T)/w->P->timestep_hours)];   //NB w->T is now end of incubation period
  
  unit* u = &w->a_units[personPointer->house->unit];

  if ((flags & SEVERE)>0) inf_ness*= u->mul_severe_inf;
  else if ((flags & SYMPTOMATIC)>0) inf_ness*= u->mul_sympt_inf;

  // Deal with "instantaneous" treatment effects.
  
  if (u->trt_mul_inf_clinical>=0) {
    if (((flags & SYMPTOMATIC)>0) && ((flags & DETECTED)>0)) {
      if (((t-w->T)>=u->trt_delay) && ((t-(w->T+u->trt_delay))<=u->trt_duration)) {
        inf_ness*=u->trt_mul_inf_clinical;
      }
    }
  }
  
  if ((personPointer->status & PROPHYLAXED)>0) {
    if (personPointer->house->pph_start>=0) {
      if ((w->T>=personPointer->house->pph_start) && (w->T<=personPointer->house->pph_end)) {
        if (ranf_mt(thread_no)<u->pph_household) inf_ness*=u->pph_inf;
      }
    } else if (w->places[personPointer->house->country][personPointer->place_type].at(personPointer->place)->pph_start>=0) {
      if ((w->T>=personPointer->house->pph_start) && (w->T<=personPointer->house->pph_end)) {
        if (ranf_mt(thread_no)<u->pph_social) inf_ness*=u->pph_inf;
      }
    }
  }

  if ((personPointer->status & VACCINATED)>0) {
    if (w->T>=u->v_start) inf_ness*=u->v_m_inf;
  }
  return inf_ness;
}


#ifdef ENABLE_LD_TRAVEL
void infectedPerson::createTravelPlan(world* w, infectedPerson* p, int thread_no, float latent_end) {
  double p_travel = ranf_mt(thread_no);
  double p_visit = ranf_mt(thread_no);
  
  
  if ((p_travel<(p->t_inf/24.0)*w->prob_travel[p->personPointer->house->country]) && (p_visit<(p->t_inf/24.0)*w->prob_visit[p->personPointer->house->country])) {
    if (ranf_mt(thread_no)<0.5) {
      p_travel=1*(p->t_inf/24.0);     // Ensure that p_travel fails
      p_visit=0;
    }
    else {
      p_visit=1*(p->t_inf/24.0);       // Ensure that p_visit fails
      p_travel=0;         
    }
  }

  if (p_travel<(1-exp(-(p->t_inf/24.0)*w->prob_travel[p->personPointer->house->country]))) {
    double destination = ranf_mt(thread_no);
    int index=0;
    while ((index<w->no_countries) && (w->prob_dest[p->personPointer->house->country][index]<destination)) index++;
    if (index>=w->no_countries) {
      printf("%d,%d Time=%d, Had to correct index\n",w->mpi_rank,thread_no,w->T);
      fflush(stdout);
      index=w->no_countries-1;
    }
    p->travel_plan=new travelPlan();
    p->travel_plan->traveller=TRAVELLER;
    p->travel_plan->country=w->prob_dest_country[p->personPointer->house->country][index];
    p->travel_plan->duration=(float) (ranf_mt(thread_no)*p->t_inf);
    p->travel_plan->t_start=(float) (latent_end+(ranf_mt(thread_no)*p->t_inf));
    if ((p->travel_plan->t_start+p->travel_plan->duration)>latent_end+p->t_inf) {
      p->travel_plan->duration=(latent_end+p->t_inf)-p->travel_plan->t_start;
    }
    
    if ((p->travel_plan->country<0) || (p->travel_plan->country>w->no_countries)) {
      printf("%d,%d: Time=%d, Country error: index=%d, p->tp->country=%d, p->country=%d\n",w->mpi_rank,thread_no,w->T,index,p->travel_plan->country,p->personPointer->house->country);
      fflush(stdout);
    }
    unsigned int total_people = 0;
    int target_node=0;
    for (target_node=0; target_node<w->mpi_size; target_node++) {
      total_people+=w->people_per_country_per_node[p->travel_plan->country][target_node];   // How many people in that country?
    }
    unsigned int person_no = (unsigned int) (ranf_mt(thread_no)*total_people);
  
    total_people = 0;
    target_node=-1;
    unsigned int previous_total=0;
    while (total_people<=person_no) {
      target_node++;
      previous_total=total_people;
      total_people+=w->people_per_country_per_node[p->travel_plan->country][target_node];
    } // Now target_node is the node on which we should find a "patch" to temporarily put this traveller in.
    p->travel_plan->travel_node=target_node;
    p->travel_plan->travel_subperson=person_no-previous_total;   // So sub_person is the person no. in that country, on that node. (ie, remove any inter-node offset)
    p->travel_plan->x=-999;
    p->travel_plan->y=-999;
    if (target_node>=w->mpi_size) {
      printf("%d,%d,%d: TARGET NODE ERROR: country=%d,\n",w->mpi_rank,thread_no,w->T,p->travel_plan->country);
      fflush(stdout);
    }

  } else if (p_visit<(1-exp(-(p->t_inf/24.0)*w->prob_visit[p->personPointer->house->country]))) {
    double origin = ranf_mt(thread_no);
    int index=0;
    while ((index<w->no_countries) && (w->prob_orig[p->personPointer->house->country][index]<origin)) index++;
  
    if (index>=w->no_countries) index--;
    p->travel_plan=new travelPlan();
    p->travel_plan->traveller=VISITOR;
    p->travel_plan->country=w->prob_orig_country[p->personPointer->house->country][index];
    p->travel_plan->duration=(float) (ranf_mt(thread_no)*p->t_inf);
    p->travel_plan->t_start=(float) latent_end;


    unsigned int total_people = 0;
    int target_node=0;

    for (target_node=0; target_node<w->mpi_size; target_node++) {
      total_people+=w->people_per_country_per_node[p->travel_plan->country][target_node];   // How many people in that country?
    }

    unsigned int person_no = (unsigned int) (ranf_mt(thread_no)*total_people);
  
    total_people = 0;
    target_node=-1;
    unsigned int previous_total=0;
    while (total_people<=person_no) {
      target_node++;
      previous_total=total_people;
      total_people+=w->people_per_country_per_node[p->travel_plan->country][target_node];
    } // Now target_node is the node on which we should find a "patch" to temporarily put this traveller in.
  
    p->travel_plan->travel_node=target_node;
    p->travel_plan->travel_subperson=person_no-previous_total;   // So sub_person is the person no. in that country, on that node. (ie, remove any inter-node offset)
    p->travel_plan->x=-999;
    p->travel_plan->y=-999;
    
    if (target_node>=w->mpi_size) {
      printf("%d,%d,%d: TARGET NODE ERROR: country=%d\n",w->mpi_rank,thread_no,w->T,p->travel_plan->country);
      fflush(stdout);
      p->travel_plan=NULL;
    }
  }
}
#else
void infectedPerson::createTravelPlan(world* w, infectedPerson* p, int thread_no, float t_incub) {
  p->travel_plan=NULL;
}
#endif

void infectedPerson::locateTravel(world* w, infectedPerson* ip, int thread_no) {
  // Find the patch
  ip->travel_plan->patch = getPatchForPerson (w,ip->travel_plan->travel_subperson,ip->travel_plan->country,thread_no);
  double min = lsIndexToLon(ip->travel_plan->patch->x);
  double max = lsIndexToLon(ip->travel_plan->patch->x+ip->travel_plan->patch->size);
  ip->travel_plan->x=(float) (min+(ranf_mt(thread_no)*(max-min)));
  min = lsIndexToLat(ip->travel_plan->patch->y);
  max = lsIndexToLat(ip->travel_plan->patch->y+ip->travel_plan->patch->size);
  ip->travel_plan->y=(float) (min+(ranf_mt(thread_no)*(max-min)));
}

localPatch* infectedPerson::getPatchForPerson(world* w, int person, unsigned char country, int thread_no) {
  
  int patch_no = 0;
  int people_count = person;
  while ((patch_no<w->country_patch_pop[country].size()) && (people_count>w->country_patch_pop[country].at(patch_no))) {
    people_count-=w->country_patch_pop[country].at(patch_no);
    patch_no++;
  }
  if (patch_no>=w->country_patch_pop[country].size()) patch_no--;
  return w->localPatchList[w->patches_in_country[country].at(patch_no)];

}

void infectedPerson::setFlags(world* w, int thread_no) {
  flags=0;
  if (ranf_mt(thread_no)<w->a_units[personPointer->house->unit].p_symptomatic) {
    flags=flags + SYMPTOMATIC;
    if (ranf_mt(thread_no)<w->a_units[personPointer->house->unit].p_severe) {
      flags = flags + SEVERE;
      if (ranf_mt(thread_no)<w->a_units[personPointer->house->unit].p_detect_severe) flags=flags + DETECTED;
    } else {
      if (ranf_mt(thread_no)<w->a_units[personPointer->house->unit].p_detect_sympt) flags=flags + DETECTED;
    }
  }
}

float infectedPerson::getNextContactWhileAtHomeOrWorking(world* w, int thread_no) {
  float new_contact_time = (float) (w->T+w->P->timestep_hours);
  if (travel_plan==NULL) {
    new_contact_time += (float) (ranf_mt(thread_no)*t_inf);       // Pick random (uniform) time (hours) for a contact to be scheduled
    if (new_contact_time<w->T) {
      printf("%d,%d : GNCWAHOW_NTP : w->T=%d, t_inf=%lf, nct=%lf\n", w->mpi_rank,thread_no,w->T,t_inf,new_contact_time);
      fflush(stdout);
    }
  } else {
    new_contact_time += (float) (ranf_mt(thread_no)*(t_inf-travel_plan->duration));  // Pick random point in time spent at home (ie, not travelling).
    if (new_contact_time>travel_plan->t_start) new_contact_time+=travel_plan->duration; // If at that time the person is away, shift contact time until they get back.
    if (new_contact_time<w->T) {
      printf("%d,%d : GNCWAHOW_TP : w->T=%d, t_inf=%lf, duration=%lf, nct=%lf, start=%lf\n",
        w->mpi_rank,thread_no,w->T,t_inf,travel_plan->duration,new_contact_time,travel_plan->t_start);
      fflush(stdout);
    }
  }
  return new_contact_time;
} 

travelPlan::travelPlan() {}
travelPlan::~travelPlan() {}

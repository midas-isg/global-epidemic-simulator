/* unit.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for the administrative unit class
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
#ifndef _DEFADMIN
#define _DEFADMIN

#include "gps_math.h"
#include "intervention.h"

#define UNIVERSE  255
class LiveIntervention;

class unit {
  public:
    unsigned char level;   // Level of this admin unit
    unsigned char country; // "Compressed" country code - 0-230
    int parent_id;         // Index into w->a_units for the parent admin unit (parent.level=this.level-1)
    double k_a,k_b,k_cut;  // Local Movement kernel
    double B_spat,B_hh;    // Transmission coefficient for spatial and households
    double* B_place;       // Transmission coefficients for each place
    double* P_group;       // Probability that school/workgroup contact is within "sub-group"
    int no_hosts;          // Number of people with houses in this unit
    unsigned char no_nodes; // Number of nodes that contain part of this admin unit.
    int no_interventions;  // Number of intervention polocies in this unit
    LiveIntervention* interventions; // And the interventions.

    int live_interventions;          // Number of intervention policies currently active


    // Specific variables for interventions when active in this unit.

    double bc_deny_entry,bc_deny_exit;                   // border controls
    
    double trt_mul_inf_clinical,trt_delay,trt_duration;  // treatment 
    
    double pph_susc,pph_inf;             // prophylaxis: mult susceptibility by, mult infectiousness if they still get infected...
    double pph_clin,pph_delay;           //              mult chance of clinical in they still get infected, delay before prophylaxis happens
    double pph_duration,pph_household;   //              duration of prophylaxis, proportion of household members prophylaxed,
    double pph_social;                   //              proportion of school/workplace prophylaxed.
    double pph_coverage;                 //              coverage for social prophylaxis (proportion of schoolS/workplaceS prophylaxed)

    double q_duration, q_hh_rate;     // quarantine:   duration, multiply household contact rate,
    double q_delay, q_compliance;     //               delay to setup, probability of household compliance,
    double q_s_wp_rate;               //               multiply school/workplace contact rate if hh is quarantined.
    double q_community;               //               multiply community contact rate

    double v_m_susc,v_m_inf,v_m_clin; // vaccination:  multipliers for susceptibility, infectiousness and clinical probability
    double v_coverage, v_start;       //               coverage proportion and time (including delay) when vaccination becomes active.

    double c_period,c_threshold,c_delay; // place closure:   period of closure, threshold for closure, and delay
    double c_hh_mul,c_comm_mul;          //                  multiply household and community contact rates
    int c_unit;                          //                  unit (0=cases, 1=% ?)

    double p_symptomatic, p_severe;        // probabilities of an infection being symptomatic/severe
    double p_detect_sympt,p_detect_severe;  // probability of a case being detected!
    double* abs_place_sympt;                // for each place type, probability of place absenteeism given a clinical infection
    double* abs_place_sev;                 // for each place type, probability of place absenteeism given a severe infection
    double* abs_place_sympt_cc_mul;         // If clinical, and absent, multiply community contact rate by... (for each place type)
    double* abs_place_sev_cc_mul;          // If severe, and absent, multiply community contact rate by... (for each place type)
    double mul_sympt_inf;               // Multiply infectiousness if case is clinical
    double mul_severe_inf;                 // Multiply infectiousness if case is severe
    double seasonal_max;
    double seasonal_min;
    double seasonal_temporal_offset;
      
    // Logging
    
    bool log;                        // Should output be logged for this admin unit?
    int new_comm_infs;     // New community infections (clincal + non-clinical)
    int* new_place_infs;   // New place infections - per place-type
    int new_hh_infs;       // New household infections
    
    int current_symptomatic_inf;     // Number of currently infectious (symptomatic) people
    int current_nonsymptomatic_inf;  // Number of currently infected (non-symptomatic) people

    int new_comm_cases;     // New community clinical cases
    int* new_place_cases;   // New establishment clincal cases - per place-type
    int new_hh_cases;       // New household clinical cases

	  int* hist_comm_cases;    // Track community cases over last 10 days - per timestes
	  int** hist_place_cases;  // Track place-cases over last 10 days - per place-type, per timestep
	  int* hist_hh_cases;      // Track household cases over last 10 days - per timestep

	  int comm_10day_accumulator;  // 10-day value for community contacts
	  int* place_10day_accumulator;   // 10-day value for each place-type
	  int hh_10day_accumulator;    // 10-day value for households

	  short next_slot;             // Next slot to be over-written in the 10-day history arrays.
	                             // (i.e., subtract this entry from the 10-day values, and put the new value in the same place)

    int* contact_makers;       // Number of individuals scheduling contacts (per thread)

    unit();
    
    ~unit();
    
    static double kernel_F(unit* u, double d);
    void add_comm_case(world* w, unsigned int unit_no, int thread_no, bool clinical);
    void add_place_case(world* w, unsigned int unit_no, unsigned char place_type, int thread_no, bool clinical);
    void add_hh_case(world* w, unsigned int unit_no, int thread_no, bool clinical);
    void delta_non_sympt_case(world* w, unsigned int unit_no, int thread_no,int delta);
    void delta_sympt_case(world* w, unsigned int unit_no, int thread_no,int delta);

    void vaccinate(world* w,unsigned int unit_no);
    double getSeasonality(world* w, double latitude);
  };


#endif

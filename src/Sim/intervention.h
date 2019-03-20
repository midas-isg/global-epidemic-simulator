/* intervention.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for interventions class.
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


#ifndef INTERVENTION_H
#define INTERVENTION_H

#define BORDER_CONTROL_ID 0
#define TREATMENT_ID 1
#define PROPHYLAXIS_ID 2
#define VACC_ID 3
#define QUARANTINE_ID 4
#define PLACE_CLOSE_ID 5
#define BLANKET_ID 6
#define AREA_QUARANTINE_ID 7

#include "world.h"
#include "gps_math.h"

class world;

class intervention {
  public:
    int type;           // Intervention type
    int trig_on_type;   // 1 = Fixed day, 0 = threshold start criteria
    double start_day;      // Start day of intervention (if t.on.type==1)
    int on_case_period; // 0 = instant, 1 = last 10 days...??
    int on_case_op;     // 0 = >, 1 = < for trigger on
    double on_threshold;   // threshold for switching on
    int on_thr_unit;    // unit for threshold (%, or cases)
    double on_thr_delay;   // delay before policy becomes active after switching
    
    
    int trig_off_type;   // 1 = Fixed duration, 0 = threshold stop criteria
    double duration_days;   // End day of intervention (if t.off.type==1)
    int off_case_period; // 0 = instant, 1 = last 10 days...??
    int off_case_op;     // 0 = >, 1 = < for trigger off
    double off_threshold;   // threshold for switching off
    int off_thr_unit;    // unit for threshold (%, or cases)
    double off_thr_delay;   // delay before policy becomes inactive after switching

    void* sub_type;      // pointer to other info.
    
    ~intervention();
    intervention();
    
};

class LiveIntervention {
  public:
    int int_no;         // Index of intervention in w->interventions.
    int switch_time;    // If active, switch off time, else switch on time. (step).
    bool active;        // Intervention currently working
    int unit;           // ID of parent unit
    void checkStatus(world* w);
    void turnOn(world* w);
    void turnOff(world* w);
    ~LiveIntervention();
    LiveIntervention();
};

class BorderControlInt {
  public:
    double p_deny_entry;
    double p_deny_exit;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    
    ~BorderControlInt();
    BorderControlInt();
};

class TreatmentInt {
  public:
    double m_inf_of_clinical;
    double treat_delay;
    double treat_duration;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    ~TreatmentInt();
    TreatmentInt();
};

class ProphylaxisInt {
  public:
    double m_susc_of_susceptible;
    double m_inf_of_proph;
    double m_clin_of_proph;
    double proph_delay;
    double proph_duration;
    double proph_household;
    double proph_coverage;
    double proph_social;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    
    ~ProphylaxisInt();
    ProphylaxisInt();
};

class VaccinationInt {
  public:
    double m_vacc_susc;
    double m_vacc_inf;
    double m_vacc_clin;
    double vacc_delay;
    double vacc_coverage;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    ~VaccinationInt();
    VaccinationInt();
};

class QuarantineInt {
  public:
    double m_s_wp_rate;
    double m_hh_rate;
    double q_period;
    double q_delay;
    double q_compliance;
    double q_community;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    ~QuarantineInt();
    QuarantineInt();
  
};

class PlaceClosureInt {
  public:
    double threshold;
    int thr_unit; 
    double delay;
    double period;
    double m_hh_rate;
    double m_comm_rate;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
   
    ~PlaceClosureInt();
    PlaceClosureInt();
};

class AreaQuarantineInt {
  public:
    double ring_radius;
    double p_deny_travel;
    double period;
    double delay;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
        
    ~AreaQuarantineInt();
    AreaQuarantineInt();
};

class BlanketTravelInt {
  public:
    double distance;
    double p_deny_travel;
    void on(world* w,LiveIntervention* intv);
    void off(world* w,LiveIntervention* intv);
    
    ~BlanketTravelInt();
    BlanketTravelInt();
};



#endif


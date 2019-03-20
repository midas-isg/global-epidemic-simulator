/* intervention.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Intervention class and functions for de/activating.
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


#include "intervention.h"

intervention::~intervention() {}
intervention::intervention() {}
  
BorderControlInt::~BorderControlInt() {}
BorderControlInt::BorderControlInt() {}
TreatmentInt::~TreatmentInt() {}
TreatmentInt::TreatmentInt() {}
ProphylaxisInt::~ProphylaxisInt() {}
ProphylaxisInt::ProphylaxisInt() {}
VaccinationInt::~VaccinationInt() {}
VaccinationInt::VaccinationInt() {}
QuarantineInt::~QuarantineInt() {}
QuarantineInt::QuarantineInt() {}
PlaceClosureInt::~PlaceClosureInt() {}
PlaceClosureInt::PlaceClosureInt() {}
AreaQuarantineInt::~AreaQuarantineInt() {}
AreaQuarantineInt::AreaQuarantineInt() {}
BlanketTravelInt::~BlanketTravelInt() {}
BlanketTravelInt::BlanketTravelInt() {}
LiveIntervention::~LiveIntervention() {}
LiveIntervention::LiveIntervention() {}

void LiveIntervention::checkStatus(world* w) {
  if (!active) {                                                     // Intervention is not active
    if (switch_time==-1) {                                           // Switch-on time has not been decided
      if (w->interventions[int_no].trig_on_type==1) {                // If start of intervention is at fixed time
        if (eq(w->T_day,w->interventions[int_no].start_day)) {       //    Then has the timestep been reached?
          switch_time=w->T;
          if ((int)w->T>=switch_time) {                                   //    If the switching time is already here
            turnOn(w);                                               //    Turn on intervention
            switch_time=-1;                                          //    And set switch-off time to undefined.
          }
        }
      } else {                                    // Dynamic start of intervention
        bool turn_on=false;
        int total=0;
        if (w->interventions[int_no].on_case_period==0) {                      // Consider instantaneous no. of cases
          total=w->a_units[unit].new_comm_cases+w->a_units[unit].new_hh_cases;
          for (unsigned int j=0; j<w->P->no_place_types; j++) total+=w->a_units[unit].new_place_cases[j];
        }

        if (w->interventions[int_no].on_thr_unit==0) {                      // Unit is no. of cases (not percent)
          if (w->interventions[int_no].on_case_op==0) turn_on=(total>w->interventions[int_no].on_threshold);
          else turn_on=(total<w->interventions[int_no].on_threshold);
        } else {                                                            // Unit is percent
          if (w->interventions[int_no].on_case_op==0)
            turn_on=(100.0*total/(double)w->a_units[unit].no_hosts>w->interventions[int_no].on_threshold);
          else
            turn_on=(100.0*total/(double)w->a_units[unit].no_hosts<w->interventions[int_no].on_threshold);
        }

        if (turn_on) {
          switch_time=(int) (w->T+(w->interventions[int_no].on_thr_delay*24.0));
          if ((int)w->T>=switch_time) {
            turnOn(w);
            switch_time=-1;
          }
        }
      }
    } else {                                      // Intervention is active, but switch on delay has not been reached
      if ((int)w->T>=switch_time) {                    // Re-check switch on time.
        turnOn(w);                                // Turn on if reached
        switch_time=-1;                           // And set switch-off time to undefined.
      }
    }
  }

  ////////// Check for deactivation

  if (active) {                        // Intervention is active.
    if (switch_time==-1) {             // Switch-off time is undefined
      if (w->interventions[int_no].trig_off_type==1) {   // Intervention has a fixed duration
        switch_time=(int) (w->T+(w->interventions[int_no].duration_days*24.0));
        if ((int)w->T>=switch_time) {       // If fixed-time is already reached
          turnOff(w);                  // Turn off intervention now
          switch_time=-1;              // Set switching time as undefined (assuming intervention can be reactivated)
        }
      } else {                         // Dynamic duration of intervention
        bool turn_off=false;
        int total=0;
        if (w->interventions[int_no].off_case_period==0) {                      // Consider instantaneous no. of cases
          total=w->a_units[unit].new_comm_cases+w->a_units[unit].new_hh_cases;
          for (unsigned int j=0; j<w->P->no_place_types; j++) total+=w->a_units[unit].new_place_cases[j];
        }

        if (w->interventions[int_no].off_thr_unit==0) {                      // Unit is no. of cases (not percent)
          if (w->interventions[int_no].off_case_op==0) turn_off=(total>w->interventions[int_no].off_threshold);
          else turn_off=(total<w->interventions[int_no].off_threshold);
        } else {                                                            // Unit is percent
          if (w->interventions[int_no].off_case_op==0)
            turn_off=(100.0*total/(double)w->a_units[unit].no_hosts>w->interventions[int_no].off_threshold);
          else
            turn_off=(100.0*total/(double)w->a_units[unit].no_hosts<w->interventions[int_no].off_threshold);
        }

        if (turn_off) {
          switch_time=(int) (w->T+(w->interventions[int_no].off_thr_delay*24.0));
          if ((int)w->T>=switch_time) {
            turnOff(w);
            switch_time=-1;
          }
        }
      }
    } else {
      if ((int)w->T>=switch_time) {         // Fixed-time has now been reached
        turnOff(w);                    // Turn off intervention
        switch_time=-1;                // Set switching time as undefined.
      }
    }
  }


}

void LiveIntervention::turnOn(world* w) {
  active=true;
  w->a_units[unit].live_interventions++;
  if (w->interventions[int_no].type==BORDER_CONTROL_ID) ((BorderControlInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==TREATMENT_ID) ((TreatmentInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==PROPHYLAXIS_ID) ((ProphylaxisInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==VACC_ID) ((VaccinationInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==QUARANTINE_ID) ((QuarantineInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==PLACE_CLOSE_ID) ((PlaceClosureInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==AREA_QUARANTINE_ID) ((AreaQuarantineInt*) w->interventions[int_no].sub_type)->on(w,this);
  else if (w->interventions[int_no].type==BLANKET_ID) ((BlanketTravelInt*) w->interventions[int_no].sub_type)->on(w,this);
}

void LiveIntervention::turnOff(world* w) {
  active=false;
  w->a_units[unit].live_interventions--;
  if (w->interventions[int_no].type==BORDER_CONTROL_ID) ((BorderControlInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==TREATMENT_ID) ((TreatmentInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==PROPHYLAXIS_ID) ((ProphylaxisInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==VACC_ID) ((VaccinationInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==QUARANTINE_ID) ((QuarantineInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==PLACE_CLOSE_ID) ((PlaceClosureInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==AREA_QUARANTINE_ID) ((AreaQuarantineInt*) w->interventions[int_no].sub_type)->off(w,this);
  else if (w->interventions[int_no].type==BLANKET_ID) ((BlanketTravelInt*) w->interventions[int_no].sub_type)->off(w,this);

}

void BorderControlInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].bc_deny_entry=p_deny_entry;
  w->a_units[intv->unit].bc_deny_exit=p_deny_exit;
}

void BorderControlInt::off(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].bc_deny_entry=-1;
  w->a_units[intv->unit].bc_deny_exit=-1;
}


void TreatmentInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].trt_mul_inf_clinical=m_inf_of_clinical;
  w->a_units[intv->unit].trt_delay=treat_delay;
  w->a_units[intv->unit].trt_duration=treat_duration;
}

void TreatmentInt::off(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].trt_mul_inf_clinical=-1;
}


void ProphylaxisInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].pph_susc=m_susc_of_susceptible;
  w->a_units[intv->unit].pph_inf=m_inf_of_proph;
  w->a_units[intv->unit].pph_clin=m_clin_of_proph;
  w->a_units[intv->unit].pph_delay=proph_delay;
  w->a_units[intv->unit].pph_duration=proph_duration;
  w->a_units[intv->unit].pph_household=proph_household;
  w->a_units[intv->unit].pph_coverage=proph_coverage;
  w->a_units[intv->unit].pph_social=proph_social;
}

void ProphylaxisInt::off(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].pph_susc=-1;
}


void VaccinationInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].v_coverage=vacc_coverage;
  w->a_units[intv->unit].v_start=w->T+(vacc_delay*24.0);
  w->a_units[intv->unit].v_m_susc=m_vacc_susc;
  w->a_units[intv->unit].v_m_inf=m_vacc_inf;
  w->a_units[intv->unit].v_m_clin=m_vacc_clin;
  w->a_units[intv->unit].vaccinate(w,intv->unit);

}

void VaccinationInt::off(world* w,LiveIntervention* intv) {
}

    
void QuarantineInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].q_duration=q_period;
  w->a_units[intv->unit].q_delay=q_delay;
  w->a_units[intv->unit].q_compliance=q_compliance;
  w->a_units[intv->unit].q_hh_rate=m_hh_rate;
  w->a_units[intv->unit].q_s_wp_rate=m_s_wp_rate;
  w->a_units[intv->unit].q_community=q_community;
}

void QuarantineInt::off(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].q_compliance=-1;
}

    
void PlaceClosureInt::on(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].c_threshold=threshold;
  w->a_units[intv->unit].c_unit=thr_unit;
  w->a_units[intv->unit].c_delay=delay;
  w->a_units[intv->unit].c_period=period;
  w->a_units[intv->unit].c_hh_mul=m_hh_rate;
  w->a_units[intv->unit].c_comm_mul=m_comm_rate;
}

void PlaceClosureInt::off(world* w,LiveIntervention* intv) {
  w->a_units[intv->unit].c_threshold=-1;
}

    
void AreaQuarantineInt::on(world* w,LiveIntervention* intv) {}
void AreaQuarantineInt::off(world* w,LiveIntervention* intv) {}


void BlanketTravelInt::on(world* w,LiveIntervention* intv) {}
void BlanketTravelInt::off(world* w,LiveIntervention* intv) {}


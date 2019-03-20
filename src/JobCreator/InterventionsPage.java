/* Interventions.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for designing intervention policies 
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

import java.awt.Color;
import java.io.DataOutputStream;
import java.util.ArrayList;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GButton;
import GKit.GLabel;
import GKit.GLine;
import GKit.GList;
import GKit.GListHeader;
import GKit.GPage;
import GKit.GPanel;
import GKit.GTextEntry;
import GKit.GTickBox;
import GKit.GWindow;



public class InterventionsPage implements GPage {
  GWindow gw;
  GSim ga;
  PolicyLinkage pl;

  GLabel _name,_type;
  GTextEntry t_name;
  GListHeader lh_type;
  GList l_type,l_intervent;
  GButton b_removeInt,b_editInt,b_createI,b_overwriteI;
  GPanel p_triggers;
  
  // Trigger bits
  
  GLabel _tr_fixed_on,_tr_fixed_duration,_tr_trig_on,_tr_trig_off,_fixed_on_day,_fixed_duration,_on_cases,_off_cases;
  GLabel _delay_on,_delay_off;
  GButton b_on_case_op,b_off_case_op;
  GListHeader lh_trig_on_period,lh_trig_off_period,lh_case_unit_on,lh_case_unit_off;
  GList l_trig_on_period,l_trig_off_period,l_case_unit_on,l_case_unit_off;
  GTickBox tb_fixed_on,tb_fixed_duration,tb_trig_on,tb_trig_off;
  GTextEntry t_fixed_on_day,t_fixed_duration,t_cases_on,t_cases_off,t_delay_on,t_delay_off;
  
  static double DEFAULT_FIX_ON_DAY = 30;
  static double DEFAULT_CASES_ON=10;
  static double DEFAULT_CASES_OFF=90;
  
  // Border crossing panel
  
  GPanel p_border;
  GLabel _denyEntry,_denyExit;
  GTextEntry t_denyEntry,t_denyExit;
  
  static double DEFAULT_DENY_ENTRY = 0.99;
  static double DEFAULT_DENY_EXIT = 0.99;
  
  // Treatment panel
  
  GPanel p_treatment;
  GLabel _rel_mod_of_symptomatic,_treatment_delay,_treat_duration;
  GTextEntry t_mod_inf_of_symptomatic,t_treatment_delay,t_treat_duration;
  
  static double DEFAULT_MOD_INF_SYMPT = 0.4;
  static double DEFAULT_TREATMENT_DELAY = 1;
  static double DEFAULT_TREATMENT_DURATION = 5;
    
   
  // Prophylaxis panel
  
  GPanel p_proph;
  GLabel _mod_susc_of_susceptible,_mod_inf_post_proph,_mod_proph_sympt,_proph_delay,_proph_duration,_proph_house,_proph_social,_proph_cov;
  GTextEntry t_mod_susc_of_susceptible,t_mod_inf_post_proph,t_mod_proph_sympt,t_proph_delay,t_proph_duration,t_proph_house,t_proph_social,t_proph_cov;
  
  static double DEFAULT_PROPH_SUSC_UNINF = 0.7;
  static double DEFAULT_POST_PROPH_INF = 0.4;
  static double DEFAULT_PROPH_REDUCE_SYMPT = 0.35;
  static double DEFAULT_PROPH_DELAY = 1;
  static double DEFAULT_PROPH_DURATION = 10;
  static double DEFAULT_PROPH_SOCIAL = 0.9;
  static double DEFAULT_PROPH_HOUSE = 1.0;
  static double DEFAULT_PROPH_SOCIAL_COVERAGE = 0.9;

  
  // Vaccination panel
  
  GPanel p_vacc;
  GLabel _mod_vacc_susc,_mod_vacc_inf,_mod_vacc_clin,_vacc_pro_delay,_vacc_coverage;
  GTextEntry t_mod_vacc_susc,t_mod_vacc_inf,t_mod_vacc_clin,t_vacc_pro_delay,t_vacc_coverage;
  
  static double DEFAULT_VACC_MOD_SUSC = 0.3;
  static double DEFAULT_VACC_MOD_INF = 0.7;
  static double DEFAULT_VACC_MOD_CLIN = 0.5;
  static double DEFAULT_VACC_PROT_DELAY = 14;
  static double DEFAULT_VACC_COVERAGE = 0.9;
  
  
  // Household quarantine
  
  GPanel p_hq;
  GLabel _hq_mod_s_wp_rate,_hq_mod_hh_rate,_hq_period,_hq_delay,_hq_compliance,_hq_community;
  GTextEntry t_mod_s_wp_rate,t_hq_mod_hh_rate,t_hq_period,t_hq_delay,t_hq_compliance,t_hq_community;
  
  static double DEFAULT_HQ_MOD_S_WP_RATE = 0.25;
  static double DEFAULT_HQ_MOD_HH_RATE = 2.0;
  static double DEFAULT_HQ_MOD_COMM_RATE = 0.75;
  static double DEFAULT_HQ_PERIOD = 14;
  static double DEFAULT_HQ_DELAY = 1;
  static double DEFAULT_HQ_COMPLIANCE = 0.5;
  
  // Place closure
  
  GPanel p_pc;
  GLabel _pc_threshold,_pc_delay,_pc_period,_pc_mod_hh,_pc_mod_comm;
  GList l_pc_threshold_unit;
  GListHeader lh_pc_threshold_unit;
  GTextEntry t_pc_threshold,t_pc_delay,t_pc_period,t_pc_mod_hh,t_pc_mod_comm;
  
  static double DEFAULT_PC_THRESHOLD = 1;
  static double DEFAULT_PC_DELAY = 1;
  static double DEFAULT_PC_PERIOD = 21;
  static double DEFAULT_PC_MOD_HH = 1.5;
  static double DEFAULT_PC_MOD_COMM = 1.25;
  
  // Area quarantine - implemented next release
/*  
  GPanel p_aq;
  GLabel _aq_ring_radius,_aq_deny_travel_prob,_aq_period,_aq_delay;
  GTextEntry t_aq_ring_radius,t_aq_deny_travel_prob,t_aq_period,t_aq_delay;
  
  static double DEFAULT_AQ_RING_SIZE = 20;
  static double DEFAULT_AQ_DENY_TRAVEL = 0.9;
  static double DEFAULT_AQ_PERIOD = 7;
  static double DEFAULT_AQ_DELAY = 2;
*/  
  // Blanket travel restriction
  
  GPanel p_bt;
  GLabel _bt_distance,_bt_deny_travel_prob;
  GTextEntry t_bt_distance,t_bt_deny_travel_prob;
  
  static double DEFAULT_BT_DISTANCE = 20;
  static double DEFAULT_BT_DENY_TRAVEL = 0.9;
  
  
  static String PROPORTION = new String("%");
  static String COUNT = new String("Cases");
  static String[] case_units = new String[] {PROPORTION,COUNT};
  
  static String DAILY = new String("Daily");
  static String TEN_DAY_PERIOD = new String("Last 10 days");
  static String CUMULATIVE = new String("Cumulative");
  static String[] periods = new String[] {DAILY,CUMULATIVE,TEN_DAY_PERIOD};
  
  static String BORDER_CONTROL = new String("Border Controls");
  static String TREATMENT = new String("Treatment of Symptomatic");
  static String PROPH = new String("Prophylaxis of Uninfected");
  static String VACCINATION = new String("Vaccination");
  static String QUARANTINE = new String("Case Quarantine");
  static String CLOSURE = new String("Place Closure");
  static String BLANKET_T = new String("Blanket Travel Restriction");
  //static String AREA_Q = new String("Area Quarantine");  
  
  static int BORDER_CONTROL_ID = 0;
  static int TREATMENT_ID = 1;
  static int PROPHYLAXIS_ID = 2;
  static int VACC_ID = 3;
  static int QUARANTINE_ID = 4;
  static int PLACE_CLOSE_ID = 5;
  static int BLANKET_ID = 6;
  //static int AREA_QUARANTINE_ID = 7;    
  // Blanket travel
  // What about limits of vaccines.
  
  
  GPanel[] interventions;
  static String[] types = new String[] {BORDER_CONTROL,TREATMENT,PROPH,VACCINATION,QUARANTINE,CLOSURE,BLANKET_T/*,AREA_Q*/};
  
  // The actual data
  
  ArrayList<ArrayList<Integer>> data;
  
  public InterventionsPage(GWindow _gw, GSim _ga, PolicyLinkage _pl) {
    gw=_gw;
    ga=_ga;
    pl=_pl;
    data = new ArrayList<ArrayList<Integer>>();
  }
  
  public final int REMOVE_INTERVENTION = 300;
  public final int EDIT_INTERVENTION = 301;
  public final int ADD_INTERVENTION = 302;
  public final int FIX_TRIG_STOP = 303;
  public final int FIX_TRIG_START = 304;
  public final int TRIG_STOP = 305;
  public final int TRIG_START = 306;
  public final int INTERVENT_TYPE = 307;
  public final int CASE_OP_ON = 308;
  public final int CASE_OP_OFF = 309;
  public final int OVER_INTERVENTION = 310;
  public final int SELECT_INTERVENTION = 311;
  public final int CHANGE_NAME = 312;
  
  public int getMinEvent() { return 300; }
  public int getMaxEvent() { return 399; }
  
  public void doFunction(int func,Object component) {
    if (func==FIX_TRIG_START) fixTrigStart();
    else if (func==FIX_TRIG_STOP) fixTrigStop();
    else if (func==TRIG_START) trigStart();
    else if (func==TRIG_STOP) trigStop();
    else if (func==INTERVENT_TYPE) changeInterventionType();
    else if (func==CASE_OP_ON) clickCaseOpOn();
    else if (func==CASE_OP_OFF) clickCaseOpOff();
    else if (func==REMOVE_INTERVENTION) removeIntervention();
    else if (func==EDIT_INTERVENTION) editIntervention();
    else if (func==ADD_INTERVENTION) addIntervention();
    else if (func==OVER_INTERVENTION) replaceIntervention(l_intervent.getSelected());
    else if (func==SELECT_INTERVENTION) selectIntervention();
    else if (func==CHANGE_NAME) changeNameText();
  }
  
  public void selectIntervention() {
    boolean orig = b_overwriteI.isEnabled();
    if (l_intervent.countSelection()==1) {
      int item = checkDupName(t_name.getText());
      if ((item==-1) || (item==l_intervent.getSelected())) b_overwriteI.setEnabled(true);
      else b_overwriteI.setEnabled(false);
    } else b_overwriteI.setEnabled(false);
    if (orig!=b_overwriteI.isEnabled()) {
      b_overwriteI.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
  }
  
  public void changeNameText() {
    selectIntervention();
    int item = checkDupName(t_name.getText());
    boolean orig = b_createI.isEnabled();
    b_createI.setEnabled(item==-1);
    if (orig!=b_createI.isEnabled()) {
      b_createI.paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
  }
  
  public void clear() {
    data.clear();
    l_intervent.clearEntries();
    pl.removeAllInterventions();
  }
  
  public void createDefaultXML(Element root) {
    saveXML(root);
  }
  
  public void saveXML(Element root) {
    Node intv_node=root.selectSingleNode("interventions");
    if (intv_node!=null) {
      root.remove(intv_node);
    }
    Element intv_el = root.addElement("interventions");
    intv_el.addAttribute("no",String.valueOf(l_intervent.countEntries()));
    for (int i=0; i<l_intervent.countEntries(); i++) {
      Element intv = intv_el.addElement("intervention");
      
      intv.addAttribute("no",String.valueOf(i));
      intv.addAttribute("name",l_intervent.getEntry(i));
      ArrayList<Integer> rec = data.get(i);
      int rec_ptr=0;
      int type = rec.get(rec_ptr++).intValue();
      intv.addAttribute("type",String.valueOf(type));
      
      // Trigger information
      
      int trig_on_type = rec.get(rec_ptr++).intValue();
      intv.addAttribute("trig_on_fix",String.valueOf(trig_on_type));
      if (trig_on_type==1) rec_ptr=addToXML(intv,"trig_on_day",rec,rec_ptr);
      else {
        intv.addAttribute("trig_on_period",String.valueOf(rec.get(rec_ptr++).intValue()));
        intv.addAttribute("trig_on_case_op",String.valueOf(rec.get(rec_ptr++).intValue()));
        rec_ptr=addToXML(intv,"trig_on_number",rec,rec_ptr);
        intv.addAttribute("trig_on_case_unit",String.valueOf(rec.get(rec_ptr++).intValue()));
        rec_ptr=addToXML(intv,"trig_on_delay",rec,rec_ptr);
      }
      
      
      int trig_off_type = rec.get(rec_ptr++).intValue();
      intv.addAttribute("trig_off_fix",String.valueOf(trig_off_type));
      if (trig_off_type==1) rec_ptr=addToXML(intv,"trig_duration",rec,rec_ptr);
      else {
        intv.addAttribute("trig_off_period",String.valueOf(rec.get(rec_ptr++).intValue()));
        intv.addAttribute("trig_off_case_op",String.valueOf(rec.get(rec_ptr++).intValue()));
        rec_ptr=addToXML(intv,"trig_off_number",rec,rec_ptr);
        intv.addAttribute("trig_off_case_unit",String.valueOf(rec.get(rec_ptr++).intValue()));
        rec_ptr=addToXML(intv,"trig_off_delay",rec,rec_ptr);
      }
      
      
      if (type==BORDER_CONTROL_ID) {
        rec_ptr=addToXML(intv,"deny_exit_p",rec,rec_ptr);
        rec_ptr=addToXML(intv,"deny_entry_p",rec,rec_ptr);
        
      } else if (type==TREATMENT_ID) {
        rec_ptr=addToXML(intv,"mult_inf_of_sympt_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"treatment_delay",rec,rec_ptr);
        rec_ptr=addToXML(intv,"treatment_duration",rec,rec_ptr);
        
        
      } else if (type==PROPHYLAXIS_ID) {
        rec_ptr=addToXML(intv,"mult_suscpeptibility_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"mult_inf_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"mult_prob_clin_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"proph_delay",rec,rec_ptr);
        rec_ptr=addToXML(intv,"proph_duration",rec,rec_ptr);
        rec_ptr=addToXML(intv,"proph_coverage",rec,rec_ptr);
        rec_ptr=addToXML(intv,"proph_hh",rec,rec_ptr);
        rec_ptr=addToXML(intv,"proph_wp",rec,rec_ptr);
        
      } else if (type==VACC_ID) {
        rec_ptr=addToXML(intv,"mult_susceptibility_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"mult_infectiousness_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"mult_clinical_p_by",rec,rec_ptr);
        rec_ptr=addToXML(intv,"vacc_delay",rec,rec_ptr);
        rec_ptr=addToXML(intv,"vacc_coverage",rec,rec_ptr);
        
      } else if (type==QUARANTINE_ID) {
        rec_ptr=addToXML(intv,"mult_sch_work_rate",rec,rec_ptr);
        rec_ptr=addToXML(intv,"mult_house_rate",rec,rec_ptr);
        rec_ptr=addToXML(intv,"hq_period",rec,rec_ptr);
        rec_ptr=addToXML(intv,"hq_delay",rec,rec_ptr);          
        rec_ptr=addToXML(intv,"hq_compliance",rec,rec_ptr);
        rec_ptr=addToXML(intv,"hq_community",rec,rec_ptr);
        
      } else if (type==PLACE_CLOSE_ID) {
        rec_ptr=addToXML(intv,"closure_threshold",rec,rec_ptr);
        intv.addAttribute("closure_threshold_unit",String.valueOf(rec.get(rec_ptr++)));
        rec_ptr=addToXML(intv,"closure_delay",rec,rec_ptr);
        rec_ptr=addToXML(intv,"closure_period",rec,rec_ptr);
        rec_ptr=addToXML(intv,"closure_mult_hh_rate",rec,rec_ptr);
        rec_ptr=addToXML(intv,"closure_mult_com_rate",rec,rec_ptr);          
        
      } else if (type==BLANKET_ID) {
        rec_ptr=addToXML(intv,"bk_distance",rec,rec_ptr);
        rec_ptr=addToXML(intv,"bk_deny_travel_prob",rec,rec_ptr);
        /*
      } else if (type==AREA_QUARANTINE_ID) {
        rec_ptr=addToXML(intv,"aq_radius",rec,rec_ptr);
        rec_ptr=addToXML(intv,"aq_deny_travel_prob",rec,rec_ptr);
        rec_ptr=addToXML(intv,"aq_period",rec,rec_ptr);
        rec_ptr=addToXML(intv,"aq_delay",rec,rec_ptr);
      */
        
      }
    } 
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(l_intervent.countEntries()));
      for (int i=0; i<l_intervent.countEntries(); i++) {
        ArrayList<Integer> rec = data.get(i);
        int rec_ptr=0;
        int type = rec.get(rec_ptr++).intValue();
        dos.writeInt(Integer.reverseBytes(type));
        int trig_on_type = rec.get(rec_ptr++).intValue();
        dos.writeInt(Integer.reverseBytes(trig_on_type));
        if (trig_on_type==1) { 
          rec_ptr=writeDouble(dos,rec,rec_ptr); 
        } else {
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          rec_ptr=writeDouble(dos,rec,rec_ptr);
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          rec_ptr=writeDouble(dos,rec,rec_ptr);
        }
        
        int trig_off_type = rec.get(rec_ptr++).intValue();
        dos.writeInt(Integer.reverseBytes(trig_off_type));
        if (trig_off_type==1) { rec_ptr=writeDouble(dos,rec,rec_ptr); }
        else {
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          rec_ptr=writeDouble(dos,rec,rec_ptr);
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          rec_ptr=writeDouble(dos,rec,rec_ptr);
        }
        int length=0;
        if (type==BORDER_CONTROL_ID) length=2;
        else if (type==TREATMENT_ID) length=3;
        else if (type==PROPHYLAXIS_ID) length=8;
        else if (type==VACC_ID) length=5;
        else if (type==QUARANTINE_ID) length=6;
        //else if (type==AREA_QUARANTINE_ID) length=4;
        else if (type==BLANKET_ID) length=2;
        else if (type==PLACE_CLOSE_ID) {
          rec_ptr=writeDouble(dos,rec,rec_ptr);
          dos.writeInt(Integer.reverseBytes(rec.get(rec_ptr++).intValue()));
          length=4;
        }
        for (int j=0; j<length; j++) rec_ptr=writeDouble(dos,rec,rec_ptr);
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  
  
  public void loadXML(Element root) {
    Node intv_el = root.selectSingleNode("interventions");
    if (intv_el!=null) {
      int no_intvs = Integer.parseInt(intv_el.valueOf("@no"));
      for (int i=0; i<no_intvs; i++) {
        Element intv = (Element) (intv_el.selectSingleNode("intervention["+(i+1)+"]"));
        String name = intv.valueOf("@name");
        l_intervent.addEntry(name);
        pl.addIntervention(name,true);
        ArrayList<Integer> rec = new ArrayList<Integer>();
        int type = Integer.parseInt(intv.valueOf("@type"));
        
        rec.add(type);

        // Trigger information
        
        int trig_on_type = Integer.parseInt(intv.valueOf("@trig_on_fix"));
        rec.add(trig_on_type);
        if (trig_on_type==1) getFromXML(intv,"@trig_on_day",rec);
        else {
          rec.add(Integer.parseInt(intv.valueOf("@trig_on_period")));
          rec.add(Integer.parseInt(intv.valueOf("@trig_on_case_op")));
          getFromXML(intv,"@trig_on_number",rec);
          rec.add(Integer.parseInt(intv.valueOf("@trig_on_case_unit")));
          getFromXML(intv,"@trig_on_delay",rec);
        }
        
        int trig_off_type = Integer.parseInt(intv.valueOf("@trig_off_fix"));
        rec.add(trig_off_type);
        if (trig_off_type==1) getFromXML(intv,"@trig_duration",rec);
        else {
          rec.add(Integer.parseInt(intv.valueOf("@trig_off_period")));
          rec.add(Integer.parseInt(intv.valueOf("@trig_off_case_op")));
          getFromXML(intv,"@trig_off_number",rec);
          rec.add(Integer.parseInt(intv.valueOf("@trig_off_case_unit")));
          getFromXML(intv,"@trig_off_delay",rec);
        }

        
        if (type==BORDER_CONTROL_ID) {
          getFromXML(intv,"@deny_exit_p",rec);
          getFromXML(intv,"@deny_exit_p",rec);
          getFromXML(intv,"@deny_entry_p",rec);
          
        } else if (type==TREATMENT_ID) {
          getFromXML(intv,"@mult_inf_of_sympt_by",rec);
          getFromXML(intv,"@treatment_delay",rec);
          getFromXML(intv,"@treatment_duration",rec);
          
        } else if (type==PROPHYLAXIS_ID) {
          getFromXML(intv,"@mult_suscpeptibility_by",rec);
          getFromXML(intv,"@mult_inf_by",rec);
          getFromXML(intv,"@mult_prob_clin_by",rec);
          getFromXML(intv,"@proph_delay",rec);
          getFromXML(intv,"@proph_duration",rec);
          getFromXML(intv,"@proph_coverage",rec);
          getFromXML(intv,"@proph_hh",rec);
          getFromXML(intv,"@proph_wp",rec);
          
        } else if (type==VACC_ID) {
          getFromXML(intv,"@mult_susceptibility_by",rec);
          getFromXML(intv,"@mult_infectiousness_by",rec);
          getFromXML(intv,"@mult_clinical_p_by",rec);
          getFromXML(intv,"@vacc_delay",rec);
          getFromXML(intv,"@vacc_coverage",rec);
          
        } else if (type==QUARANTINE_ID) {
          getFromXML(intv,"@mult_sch_work_rate",rec);
          getFromXML(intv,"@mult_house_rate",rec);
          getFromXML(intv,"@hq_period",rec);
          getFromXML(intv,"@hq_delay",rec);          
          getFromXML(intv,"@hq_compliance",rec);
          getFromXML(intv,"@hq_community",rec);
          
        } else if (type==PLACE_CLOSE_ID) {
          getFromXML(intv,"@closure_threshold",rec);
          rec.add(Integer.parseInt(intv.valueOf("@closure_threshold_unit")));
          getFromXML(intv,"@closure_delay",rec);
          getFromXML(intv,"@closure_period",rec);
          getFromXML(intv,"@closure_mult_hh_rate",rec);
          getFromXML(intv,"@closure_mult_com_rate",rec);          
        /*  
        } else if (type==AREA_QUARANTINE_ID) {
          getFromXML(intv,"@aq_radius",rec);
          getFromXML(intv,"@aq_deny_travel_prob",rec);
          getFromXML(intv,"@aq_period",rec);
          getFromXML(intv,"@aq_delay",rec);
          */
        } else if (type==BLANKET_ID) {
          getFromXML(intv,"@bk_distance",rec);
          getFromXML(intv,"@bk_deny_travel_prob",rec);
        }
        data.add(rec);
        
      }
    }
  }

  
  public void editIntervention() {
    int no = l_intervent.getSelected();
    int p = 0;
    t_name.setText(l_intervent.getEntry(no));
    ArrayList<Integer> rec = data.get(no);
    int type = rec.get(p++).intValue();
    if (rec.get(p++).intValue()==1) {
      tb_fixed_on.setSelected(true);
      tb_trig_on.setSelected(false);
      p=unpack(rec,t_fixed_on_day,p);
            
    } else {
      tb_fixed_on.setSelected(false);
      tb_trig_on.setSelected(true);
      int op = rec.get(p++).intValue();
      b_on_case_op.setText((op==0)?"<":">");
      p=unpack(rec,t_cases_on,p);
      l_case_unit_on.setSelected(rec.get(p++).intValue());
      p=unpack(rec,t_delay_on,p);
    }
    
    if (rec.get(p++).intValue()==1) {
      tb_fixed_duration.setSelected(true);
      tb_trig_off.setSelected(false);
      p=unpack(rec,t_fixed_duration,p);
            
    } else {
      tb_fixed_duration.setSelected(false);
      tb_trig_off.setSelected(true);
      l_trig_off_period.setSelected(rec.get(p++).intValue());
      int op = rec.get(p++).intValue();
      b_off_case_op.setText((op==0)?"<":">");
      p=unpack(rec,t_cases_off,p);
      l_case_unit_off.setSelected(rec.get(p++).intValue());
      p=unpack(rec,t_delay_off,p);
    }

    
    if (type==BORDER_CONTROL_ID) {
      p=unpack(rec,t_denyExit,p);
      p=unpack(rec,t_denyEntry,p);
      
    } else if (type==TREATMENT_ID) {
      p=unpack(rec,t_mod_inf_of_symptomatic,p);
      p=unpack(rec,t_treatment_delay,p);
      p=unpack(rec,t_treat_duration,p);
      
    } else if (type==PROPHYLAXIS_ID) {
      p=unpack(rec,t_mod_susc_of_susceptible,p);
      p=unpack(rec,t_mod_inf_post_proph,p);
      p=unpack(rec,t_mod_proph_sympt,p);
      p=unpack(rec,t_proph_delay,p);
      p=unpack(rec,t_proph_duration,p);
      p=unpack(rec,t_proph_cov,p);
      p=unpack(rec,t_proph_social,p);
      p=unpack(rec,t_proph_house,p);
      
    } else if (type==VACC_ID) {
      p=unpack(rec,t_mod_vacc_susc,p);
      p=unpack(rec,t_mod_vacc_inf,p);
      p=unpack(rec,t_mod_vacc_clin,p);
      p=unpack(rec,t_vacc_pro_delay,p);
      p=unpack(rec,t_vacc_coverage,p);
      
    } else if (type==QUARANTINE_ID) {
      p=unpack(rec,t_mod_s_wp_rate,p);
      p=unpack(rec,t_hq_mod_hh_rate,p);
      p=unpack(rec,t_hq_period,p);
      p=unpack(rec,t_hq_delay,p);
      p=unpack(rec,t_hq_compliance,p);
      p=unpack(rec,t_hq_community,p);
    
    } else if (type==PLACE_CLOSE_ID) {
      p=unpack(rec,t_pc_threshold,p);
      l_pc_threshold_unit.setSelected(rec.get(p++).intValue());
      p=unpack(rec,t_pc_delay,p);
      p=unpack(rec,t_pc_period,p);
      p=unpack(rec,t_pc_mod_hh,p);
      p=unpack(rec,t_pc_mod_comm,p);
      /*
    } else if (type==AREA_QUARANTINE_ID) {
      p=unpack(rec,t_aq_ring_radius,p);
      p=unpack(rec,t_aq_deny_travel_prob,p);
      p=unpack(rec,t_aq_period,p);
      p=unpack(rec,t_aq_delay,p);
      */
    } else if (type==BLANKET_ID) {
      p=unpack(rec,t_bt_distance,p);
      p=unpack(rec,t_bt_deny_travel_prob,p);
    }
    l_type.setSelected(type);
    changeInterventionType();
    p_triggers.paintOn(gw.bi(),gw.g2d());
    lh_type.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void removeIntervention() {
    int[] indexes = l_intervent.getSelectedIndexes();
    for (int j=indexes.length-1; j>=0; j--) {
      l_intervent.removeEntry(indexes[j]);
      data.remove(indexes[j]);
      pl.removeIntervention(indexes[j]);
    }
    ga.setUnSaved(true);
    
  }
  
 
  public int addToXML(Element i, String att, ArrayList<Integer> a, int p) {
    int len = a.get(p++);
    StringBuffer sb = new StringBuffer();
    for (int _i=0; _i<len; _i++) {
      sb.append((char)(a.get(p++).intValue()));
    }
    i.addAttribute(att,sb.toString());
    sb.setLength(0);
    sb=null;
    return p;
  }
  
  public int writeInt(DataOutputStream dos, ArrayList<Integer> a, int p) {
    int len = a.get(p++);
    StringBuffer sb = new StringBuffer();
    for (int _i=0; _i<len; _i++) {
      sb.append((char)(a.get(p++).intValue()));
    }
    try {
      dos.writeInt(Integer.reverseBytes(Integer.parseInt(sb.toString())));
    } catch (Exception e) { e.printStackTrace(); }
    sb.setLength(0);
    sb=null;
    return p;
  }
  
  public int writeDouble(DataOutputStream dos, ArrayList<Integer> a, int p) {
    int len = a.get(p++);
    StringBuffer sb = new StringBuffer();
    for (int _i=0; _i<len; _i++) {
      sb.append((char)(a.get(p++).intValue()));
    }
    try {
      dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(sb.toString())))));
    } catch (Exception e) { e.printStackTrace(); }
    sb.setLength(0);
    sb=null;
    return p;
  }
  
  
  public void getFromXML(Element i, String att, ArrayList<Integer> a) {
    String s = i.valueOf(att);
    a.add(new Integer(s.length()));
    for (int _i=0; _i<s.length(); _i++) {
     a.add(new Integer((int)(s.charAt(_i))));
    }
  }
  
  public int unpack(ArrayList<Integer> a, GTextEntry t, int p) {
    int len = a.get(p++);
    StringBuffer sb = new StringBuffer();
    for (int i=0; i<len; i++) {
      sb.append((char)(a.get(p++).intValue()));
    }
    t.setText(new String(sb.toString()));
    sb.setLength(0);
    sb=null;
    return p;
  }
  
  public void pack(ArrayList<Integer> a, GTextEntry t) {
    String s = t.getText();
    a.add(new Integer(s.length()));
    for (int i=0; i<s.length(); i++) {
     a.add(new Integer((int)(s.charAt(i))));
    }
  }
  
  public ArrayList<Integer> makeRecord() {
    ArrayList<Integer> this_record = new ArrayList<Integer>();
    int type = l_type.getSelected();
    this_record.add(new Integer(type));
    
    if (tb_fixed_on.isSelected()) {
      this_record.add(new Integer(1));
      pack(this_record,t_fixed_on_day);
    } else {
      this_record.add(new Integer(2));
      this_record.add(new Integer(l_trig_on_period.getSelected()));
      int op = (b_off_case_op.getText().charAt(0)=='<')?0:1;
      this_record.add(new Integer(op));
      pack(this_record,t_cases_on);
      this_record.add(new Integer(l_case_unit_on.getSelected()));
      pack(this_record,t_delay_on);
    }
    
    // Integer.   ==1   =>   String: fixed on day
    //            ==2   =>   Integer (trig_on_period), Integer (op), String (cases_on), Integer (unit), String (t_delay)
    
    if (tb_fixed_duration.isSelected()) {
      this_record.add(new Integer(1));
      pack(this_record,t_fixed_duration);
      
    } else {
      this_record.add(new Integer(2));
      this_record.add(new Integer(l_trig_off_period.getSelected()));
      int op = (b_off_case_op.getText().charAt(0)=='<')?0:1;
      this_record.add(new Integer(op));
      pack(this_record,t_cases_off);
      this_record.add(new Integer(l_case_unit_off.getSelected()));
      pack(this_record,t_delay_off);
    }
    
    // Integer.   ==1   =>   String: fixed on day
    //            ==2   =>   Integer (trig_on_period), Integer (op), String (cases_on), Integer (unit), String (t_delay)

    
    
   if (type==BORDER_CONTROL_ID) {
      pack(this_record,t_denyExit);
      pack(this_record,t_denyEntry);
      
    } else if (type==TREATMENT_ID) {
      pack(this_record,t_mod_inf_of_symptomatic);
      pack(this_record,t_treatment_delay);
      pack(this_record,t_treat_duration);
      
    } else if (type==PROPHYLAXIS_ID) {
      pack(this_record,t_mod_susc_of_susceptible);
      pack(this_record,t_mod_inf_post_proph);
      pack(this_record,t_mod_proph_sympt);
      pack(this_record,t_proph_delay);
      pack(this_record,t_proph_duration);
      pack(this_record,t_proph_cov);
      pack(this_record,t_proph_social);
      pack(this_record,t_proph_house);
      
    } else if (type==VACC_ID) {
      pack(this_record,t_mod_vacc_susc);
      pack(this_record,t_mod_vacc_inf);
      pack(this_record,t_mod_vacc_clin);
      pack(this_record,t_vacc_pro_delay);
      pack(this_record,t_vacc_coverage);
      
    } else if (type==QUARANTINE_ID) {
      pack(this_record,t_mod_s_wp_rate);
      pack(this_record,t_hq_mod_hh_rate);
      pack(this_record,t_hq_period);
      pack(this_record,t_hq_delay);
      pack(this_record,t_hq_compliance);
      pack(this_record,t_hq_community);
      
    } else if (type==PLACE_CLOSE_ID) {
      pack(this_record,t_pc_threshold);
      this_record.add(new Integer((int)l_pc_threshold_unit.getSelected()));
      pack(this_record,t_pc_delay);
      pack(this_record,t_pc_period);
      pack(this_record,t_pc_mod_hh);
      pack(this_record,t_pc_mod_comm);
      /*
    } else if (type==AREA_QUARANTINE_ID) {
      pack(this_record,t_aq_ring_radius);
      pack(this_record,t_aq_deny_travel_prob);
      pack(this_record,t_aq_period);
      pack(this_record,t_aq_delay);
      */
    } else if (type==BLANKET_ID) {
      pack(this_record,t_bt_distance);
      pack(this_record,t_bt_deny_travel_prob);
    }
    return this_record; 
  }
   
  public void addIntervention() {
    l_intervent.addEntry(t_name.getText());
    data.add(makeRecord());
    changeNameText();
    selectIntervention();
    l_intervent.paintOn(gw.bi(),gw.g2d());
    ga.setUnSaved(true);
    gw.requestRepaint();
    pl.addIntervention(t_name.getText(),false);
    
  }
  
  public void replaceIntervention(int index) {
    ArrayList<Integer> old_record = data.get(index);
    old_record.clear();
    old_record=null;
    data.set(index,makeRecord());
    l_intervent.setEntry(index,t_name.getText());
    l_intervent.paintOn(gw.bi(),gw.g2d());
    ga.setUnSaved(true);
    gw.requestRepaint();
    pl.renameIntervention(index,t_name.getText());
  }
  
  public int checkDupName(String name) {
    int item=-1;
    for (int i=0; i<l_intervent.countEntries(); i++) {
      if (l_intervent.getEntry(i).toUpperCase().equals(name.toUpperCase())) {
        item=i;
        i=l_intervent.countEntries();
      }
    }
    return item;
  }
  
  
  public void clickCaseOpOn() {
    if (b_on_case_op.getText().equals("<")) b_on_case_op.setText(">");
    else b_on_case_op.setText("<");
    b_on_case_op.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void clickCaseOpOff() {
    if (b_off_case_op.getText().equals("<")) b_off_case_op.setText(">");
    else b_off_case_op.setText("<");
    b_off_case_op.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  
  public void changeInterventionType() {
    GPanel target=null;
    if (l_type.getSelected()==BORDER_CONTROL_ID) target=p_border;
    else if (l_type.getSelected()==TREATMENT_ID) target=p_treatment;
    else if (l_type.getSelected()==PROPHYLAXIS_ID) target=p_proph;
    else if (l_type.getSelected()==VACC_ID) target=p_vacc;
    else if (l_type.getSelected()==QUARANTINE_ID) target=p_hq;
    else if (l_type.getSelected()==PLACE_CLOSE_ID) target=p_pc;
 //   else if (l_type.getSelected()==AREA_QUARANTINE_ID) target=p_aq;
    else if (l_type.getSelected()==BLANKET_ID) target=p_bt;
    else target=null;
    for (int i=0; i<interventions.length; i++) { interventions[i].setVisible(false); }
    target.setVisible(true);
    if (gw.g2d()!=null) target.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  // Trigger functions
  
  public void setFixedOn(boolean b) {
    _fixed_on_day.setVisible(b);
    t_fixed_on_day.setVisible(b);
    tb_trig_on.setSelected(!b);
    tb_fixed_on.setSelected(b);
    lh_trig_on_period.setVisible(!b);
    _on_cases.setVisible(!b);
    t_cases_on.setVisible(!b);
    lh_case_unit_on.setVisible(!b);
    b_on_case_op.setVisible(!b);
    t_delay_on.setVisible(!b);
    _delay_on.setVisible(!b);
    
  }
  
  public void setFixedDuration(boolean b) {
    _fixed_duration.setVisible(b);
    t_fixed_duration.setVisible(b);
    tb_trig_off.setSelected(!b);
    tb_fixed_duration.setSelected(b);
    lh_trig_off_period.setVisible(!b);
    _off_cases.setVisible(!b);
    t_cases_off.setVisible(!b);
    lh_case_unit_off.setVisible(!b);
    b_off_case_op.setVisible(!b);
    t_delay_off.setVisible(!b);
    _delay_off.setVisible(!b);
  }
  
  public void fixTrigStart() {
    setFixedOn(true);
    p_triggers.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void fixTrigStop() {
    setFixedDuration(true);
    p_triggers.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void trigStart() {
    setFixedOn(false);
    p_triggers.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void trigStop() {
    setFixedDuration(false);
    p_triggers.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void initGUI(GPanel i) {
    _name = (GLabel) i.addChild(new GLabel(20,20,"Name: ",GLabel.LEFT_ALIGN,ga.NONE,i,gw));
    _type = (GLabel) i.addChild(new GLabel(420,20,"Type: ",GLabel.LEFT_ALIGN,ga.NONE,i,gw));
    t_name = (GTextEntry) i.addChild(new GTextEntry(80,14,250,i,gw,"Intervention Name",CHANGE_NAME));
    l_type= (GList) i.addChild(new GList(480,14,250,150,i,types,gw,GList.SINGLE_SELECTION,INTERVENT_TYPE));
    lh_type = (GListHeader) i.addChild(new GListHeader(480,14,250,i,gw,l_type,ga.NONE));
    l_intervent=(GList) i.addChild(new GList(20,394,820,200,i,null,gw,GList.MULTI_SELECTION,SELECT_INTERVENTION));
    b_removeInt = (GButton) i.addChild(new GButton(20,614,120,22,i,gw,REMOVE_INTERVENTION,"Remove"));
    b_editInt = (GButton) i.addChild(new GButton(160,614,120,22,i,gw,EDIT_INTERVENTION,"Edit"));
    b_createI = (GButton) i.addChild(new GButton(20,354,120,22,i,gw,ADD_INTERVENTION,"Create"));
    b_overwriteI = (GButton) i.addChild(new GButton(160,354,120,22,i,gw,OVER_INTERVENTION,"Overwrite"));
    b_overwriteI.setEnabled(false);
    l_type.setVisible(false);
    
    // One panel per intervention type.
    
    // Isolation
    
    //p_isolation = (GPanel) i.addChild(new GPanel(20,140,820,200,false,"",(byte)0,false,i,gw));
    //p_isolation.setVisible(false);
    
    p_triggers = (GPanel) i.addChild(new GPanel(20,240,820,100,false,"",(byte)0,false,i,gw));
    p_triggers.addChild(new GLine(410,0,410,100,new Color(gw.getGCS().getEdge()), p_triggers, gw));
    tb_fixed_on = (GTickBox) p_triggers.addChild(new GTickBox(20,8,p_triggers,gw,FIX_TRIG_START));
    tb_fixed_duration = (GTickBox) p_triggers.addChild(new GTickBox(430,8,p_triggers,gw,FIX_TRIG_STOP));
    tb_trig_on = (GTickBox) p_triggers.addChild(new GTickBox(220,8,p_triggers,gw,TRIG_START));
    tb_trig_off = (GTickBox) p_triggers.addChild(new GTickBox(630,8,p_triggers,gw,TRIG_STOP));
    _tr_fixed_on = (GLabel) p_triggers.addChild(new GLabel(52,14,"Fixed Start Time:",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    _tr_fixed_duration = (GLabel) p_triggers.addChild(new GLabel(462,14,"Fixed Duration:",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    _tr_trig_on = (GLabel) p_triggers.addChild(new GLabel(252,14,"Switch on when:",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    _tr_trig_off = (GLabel) p_triggers.addChild(new GLabel(662,14,"Switch off when:",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    
    // Starting trigger
    
    _fixed_on_day = (GLabel) p_triggers.addChild(new GLabel(80,44,"Days into epidemic",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    t_fixed_on_day = (GTextEntry) p_triggers.addChild(new GTextEntry(20,38,50,p_triggers,gw,String.valueOf(DEFAULT_FIX_ON_DAY),ga.NONE));
    l_trig_on_period = (GList) p_triggers.addChild(new GList(20,38,120,80,p_triggers,periods,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_trig_on_period = (GListHeader) p_triggers.addChild(new GListHeader(20,38,120,p_triggers,gw,l_trig_on_period,ga.NONE));
    _on_cases = (GLabel) p_triggers.addChild(new GLabel(150,44,"clinical cases",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    b_on_case_op = (GButton) p_triggers.addChild(new GButton(244,38,22,22,p_triggers,gw,CASE_OP_ON,">"));
    t_cases_on = (GTextEntry) p_triggers.addChild(new GTextEntry(273,38,50,p_triggers,gw,String.valueOf(DEFAULT_CASES_ON),ga.NONE));
    l_case_unit_on = (GList) p_triggers.addChild(new GList(330,38,63,60,p_triggers,case_units,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_case_unit_on = (GListHeader) p_triggers.addChild(new GListHeader(330,38,63,p_triggers,gw,l_case_unit_on,ga.NONE));
    _delay_on = (GLabel) p_triggers.addChild(new GLabel(80,74,"Delay before activation (days)",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    t_delay_on = (GTextEntry) p_triggers.addChild(new GTextEntry(20,68,50,p_triggers,gw,"0",ga.NONE));
    l_case_unit_on.setVisible(false);
    l_trig_on_period.setVisible(false);
    
    
    // Ending trigger
    
    _fixed_duration = (GLabel) p_triggers.addChild(new GLabel(490,44,"Days",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    t_fixed_duration = (GTextEntry) p_triggers.addChild(new GTextEntry(430,38,50,p_triggers,gw,"1000",ga.NONE));
    l_trig_off_period = (GList) p_triggers.addChild(new GList(430,38,120,80,p_triggers,periods,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_trig_off_period = (GListHeader) p_triggers.addChild(new GListHeader(430,38,120,p_triggers,gw,l_trig_off_period,ga.NONE));
    _off_cases = (GLabel) p_triggers.addChild(new GLabel(560,44,"clinical cases",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    b_off_case_op = (GButton) p_triggers.addChild(new GButton(654,38,22,22,p_triggers,gw,CASE_OP_OFF,"<"));
    t_cases_off = (GTextEntry) p_triggers.addChild(new GTextEntry(683,38,50,p_triggers,gw,String.valueOf(DEFAULT_CASES_OFF),ga.NONE));
    l_case_unit_off = (GList) p_triggers.addChild(new GList(740,38,63,60,p_triggers,case_units,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_case_unit_off = (GListHeader) p_triggers.addChild(new GListHeader(740,38,63,p_triggers,gw,l_case_unit_off,ga.NONE));
    _delay_off = (GLabel) p_triggers.addChild(new GLabel(490,74,"Delay before deactivation (days)",GLabel.LEFT_ALIGN,ga.NONE,p_triggers,gw));
    t_delay_off = (GTextEntry) p_triggers.addChild(new GTextEntry(430,68,50,p_triggers,gw,"0",ga.NONE));

    l_case_unit_off.setVisible(false);
    l_trig_off_period.setVisible(false);
    
    
    //GLabel _tr_fixed_on,_tr_fixed_duration,_tr_trig_on,_tr_trig_off,_fixed_on_day;
    //GTickBox tb_fixed_on,tb_fixed_duration,tb_trig_on,tb_trig_off;
    //GTextEntry t_fixed_on_day;
    
    setFixedOn(true);
    setFixedDuration(true);
    
    // Border quarantine
    
    p_border = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    p_border.setVisible(true);
    _denyEntry = (GLabel) p_border.addChild(new GLabel(400,20,"Deny Entry Probability",GLabel.RIGHT_ALIGN,ga.NONE,p_border,gw));
    _denyExit = (GLabel) p_border.addChild(new GLabel(400,50,"Deny Exit Probability",GLabel.RIGHT_ALIGN,ga.NONE,p_border,gw));    
    t_denyEntry = (GTextEntry) p_border.addChild(new GTextEntry(410,14,50,p_border,gw,String.valueOf(DEFAULT_DENY_ENTRY),ga.NONE));
    t_denyExit = (GTextEntry) p_border.addChild(new GTextEntry(410,44,50,p_border,gw,String.valueOf(DEFAULT_DENY_EXIT),ga.NONE));
    
    // Treatment
    
    p_treatment = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _rel_mod_of_symptomatic = (GLabel) p_treatment.addChild(new GLabel(400,20,"Multiply infectiousness of symptomatic by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_treatment,gw));
    t_mod_inf_of_symptomatic = (GTextEntry) p_treatment.addChild(new GTextEntry(410,14,50,p_treatment,gw,String.valueOf(DEFAULT_MOD_INF_SYMPT),ga.NONE));
    _treatment_delay = (GLabel) p_treatment.addChild(new GLabel(400,50,"Delay from symptoms to treatment (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_treatment,gw));
    t_treatment_delay = (GTextEntry) p_treatment.addChild(new GTextEntry(410,44,50,p_treatment,gw,String.valueOf(DEFAULT_TREATMENT_DELAY),ga.NONE));
    _treat_duration = (GLabel) p_treatment.addChild(new GLabel(400,80,"Treatment duration (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_treatment,gw));
    t_treat_duration = (GTextEntry) p_treatment.addChild(new GTextEntry(410,74,50,p_treatment,gw,String.valueOf(DEFAULT_TREATMENT_DURATION),ga.NONE));

    // Prophylaxis
    
    p_proph = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _mod_susc_of_susceptible = (GLabel) p_proph.addChild(new GLabel(400,20,"Multiply susceptibility of uninfected by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_mod_susc_of_susceptible = (GTextEntry) p_proph.addChild(new GTextEntry(410,14,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_SUSC_UNINF),ga.NONE));
    _mod_inf_post_proph = (GLabel) p_proph.addChild(new GLabel(400,50,"Multiply infectiousness of post-prophylaxis infected by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_mod_inf_post_proph = (GTextEntry) p_proph.addChild(new GTextEntry(410,44,50,p_proph,gw,String.valueOf(DEFAULT_POST_PROPH_INF),ga.NONE));
    _mod_proph_sympt = (GLabel) p_proph.addChild(new GLabel(400,80,"Multiply probability of symptomatic infection by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_mod_proph_sympt = (GTextEntry) p_proph.addChild(new GTextEntry(410,74,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_REDUCE_SYMPT),ga.NONE));
    _proph_delay = (GLabel) p_proph.addChild(new GLabel(400,110,"Delay from symptoms to prophylaxis (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_proph_delay = (GTextEntry) p_proph.addChild(new GTextEntry(410,104,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_DELAY),ga.NONE));
    _proph_duration = (GLabel) p_proph.addChild(new GLabel(400,140,"Prophylaxis duration (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_proph_duration = (GTextEntry) p_proph.addChild(new GTextEntry(410,134,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_DURATION),ga.NONE));

    _proph_cov = (GLabel) p_proph.addChild(new GLabel(680,20,"Case Coverage: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_proph_cov = (GTextEntry) p_proph.addChild(new GTextEntry(690,44,20,p_proph,gw,String.valueOf(DEFAULT_PROPH_SOCIAL_COVERAGE),ga.NONE));
    
    _proph_house = (GLabel) p_proph.addChild(new GLabel(680,50,"Proportion of household: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_proph_house = (GTextEntry) p_proph.addChild(new GTextEntry(690,44,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_HOUSE),ga.NONE));
    _proph_social = (GLabel) p_proph.addChild(new GLabel(680,80,"Proportion of class/workgroup: ",GLabel.RIGHT_ALIGN,ga.NONE,p_proph,gw));
    t_proph_social = (GTextEntry) p_proph.addChild(new GTextEntry(690,74,50,p_proph,gw,String.valueOf(DEFAULT_PROPH_SOCIAL),ga.NONE));

    
    // Vaccination
    
    p_vacc = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _mod_vacc_susc = (GLabel) p_vacc.addChild(new GLabel(400,20,"Multiply susceptibility of uninfected by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_vacc,gw));
    t_mod_vacc_susc = (GTextEntry) p_vacc.addChild(new GTextEntry(410,14,50,p_vacc,gw,String.valueOf(DEFAULT_VACC_MOD_SUSC),ga.NONE));
    _mod_vacc_inf = (GLabel) p_vacc.addChild(new GLabel(400,50,"If infected, multiply infectiousness by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_vacc,gw));
    t_mod_vacc_inf = (GTextEntry) p_vacc.addChild(new GTextEntry(410,44,50,p_vacc,gw,String.valueOf(DEFAULT_VACC_MOD_INF),ga.NONE));
    _mod_vacc_clin = (GLabel) p_vacc.addChild(new GLabel(400,80,"If infected, multiply probability of symptomatic by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_vacc,gw));
    t_mod_vacc_clin = (GTextEntry) p_vacc.addChild(new GTextEntry(410,74,50,p_vacc,gw,String.valueOf(DEFAULT_VACC_MOD_CLIN),ga.NONE));
    _vacc_pro_delay = (GLabel) p_vacc.addChild(new GLabel(400,110,"Delay between vaccination and protection (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_vacc,gw));
    t_vacc_pro_delay = (GTextEntry) p_vacc.addChild(new GTextEntry(410,104,50,p_vacc,gw,String.valueOf(DEFAULT_VACC_PROT_DELAY),ga.NONE));
    _vacc_coverage = (GLabel) p_vacc.addChild(new GLabel(400,140,"Vaccination Coverage Proportion: ",GLabel.RIGHT_ALIGN,ga.NONE,p_vacc,gw));
    t_vacc_coverage = (GTextEntry) p_vacc.addChild(new GTextEntry(410,134,50,p_vacc,gw,String.valueOf(DEFAULT_VACC_COVERAGE),ga.NONE));
    
    // Household quarantine
    
    p_hq = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _hq_mod_s_wp_rate = (GLabel) p_hq.addChild(new GLabel(300,20,"Multiply Work/school contact rate by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_mod_s_wp_rate = (GTextEntry) p_hq.addChild(new GTextEntry(310,14,50,p_hq,gw,String.valueOf(DEFAULT_HQ_MOD_S_WP_RATE),ga.NONE));
    _hq_mod_hh_rate = (GLabel) p_hq.addChild(new GLabel(300,50,"Multiply household contact rate by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_hq_mod_hh_rate = (GTextEntry) p_hq.addChild(new GTextEntry(310,44,50,p_hq,gw,String.valueOf(DEFAULT_HQ_MOD_HH_RATE),ga.NONE));
    _hq_period = (GLabel) p_hq.addChild(new GLabel(300,80,"Quarantine period (days) : ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_hq_period = (GTextEntry) p_hq.addChild(new GTextEntry(310,74,50,p_hq,gw,String.valueOf(DEFAULT_HQ_PERIOD),ga.NONE));
    _hq_delay = (GLabel) p_hq.addChild(new GLabel(300,110,"Start-up delay (days) : ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_hq_delay = (GTextEntry) p_hq.addChild(new GTextEntry(310,104,50,p_hq,gw,String.valueOf(DEFAULT_HQ_DELAY),ga.NONE));
    _hq_compliance = (GLabel) p_hq.addChild(new GLabel(300,140,"Compliance probabilty : ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_hq_compliance = (GTextEntry) p_hq.addChild(new GTextEntry(310,134,50,p_hq,gw,String.valueOf(DEFAULT_HQ_COMPLIANCE),ga.NONE));
    _hq_community = (GLabel) p_hq.addChild(new GLabel(680,20,"Multiply community contact rate by : ",GLabel.RIGHT_ALIGN,ga.NONE,p_hq,gw));
    t_hq_community = (GTextEntry) p_hq.addChild(new GTextEntry(690,14,50,p_hq,gw,String.valueOf(DEFAULT_HQ_MOD_COMM_RATE),ga.NONE));
    
    // Place closure
    
    p_pc = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _pc_threshold = (GLabel) p_pc.addChild(new GLabel(400,20,"Threshold for closure: ",GLabel.RIGHT_ALIGN,ga.NONE,p_pc,gw));
    t_pc_threshold = (GTextEntry) p_pc.addChild(new GTextEntry(410,14,50,p_pc,gw,String.valueOf(DEFAULT_PC_THRESHOLD),ga.NONE));
    _pc_delay = (GLabel) p_pc.addChild(new GLabel(400,50,"Closure delay (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_pc,gw));
    t_pc_delay = (GTextEntry) p_pc.addChild(new GTextEntry(410,44,50,p_pc,gw,String.valueOf(DEFAULT_PC_DELAY),ga.NONE));
    _pc_period = (GLabel) p_pc.addChild(new GLabel(400,80,"Closure period (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_pc,gw));
    t_pc_period = (GTextEntry) p_pc.addChild(new GTextEntry(410,74,50,p_pc,gw,String.valueOf(DEFAULT_PC_PERIOD),ga.NONE));
    _pc_mod_hh = (GLabel) p_pc.addChild(new GLabel(400,110,"Multiply household contact rate by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_pc,gw));
    t_pc_mod_hh = (GTextEntry) p_pc.addChild(new GTextEntry(410,104,50,p_pc,gw,String.valueOf(DEFAULT_PC_MOD_HH),ga.NONE));
    _pc_mod_comm = (GLabel) p_pc.addChild(new GLabel(400,140,"Multiply community contact by: ",GLabel.RIGHT_ALIGN,ga.NONE,p_pc,gw));
    t_pc_mod_comm = (GTextEntry) p_pc.addChild(new GTextEntry(410,134,50,p_pc,gw,String.valueOf(DEFAULT_PC_MOD_COMM),ga.NONE));
    l_pc_threshold_unit = (GList) p_pc.addChild(new GList(480,14,63,60,p_pc,case_units,gw,GList.SINGLE_SELECTION,ga.NONE));    
    lh_pc_threshold_unit = (GListHeader) p_pc.addChild(new GListHeader(480,14,63,p_pc,gw,l_pc_threshold_unit,ga.NONE));
    l_pc_threshold_unit.setVisible(false);
    
   
    // Blanket travel restriction
    
    p_bt = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _bt_distance = (GLabel) p_bt.addChild(new GLabel(400,20,"Travel limit (km): ",GLabel.RIGHT_ALIGN,ga.NONE,p_bt,gw));
    t_bt_distance = (GTextEntry) p_bt.addChild(new GTextEntry(410,14,50,p_bt,gw,String.valueOf(DEFAULT_BT_DISTANCE),ga.NONE));
    _bt_deny_travel_prob = (GLabel) p_bt.addChild(new GLabel(400,50,"Probability of denying travel beyond limit: ",GLabel.RIGHT_ALIGN,ga.NONE,p_bt,gw));
    t_bt_deny_travel_prob = (GTextEntry) p_bt.addChild(new GTextEntry(410,44,50,p_bt,gw,String.valueOf(DEFAULT_BT_DENY_TRAVEL),ga.NONE));
    
    // Area quarantine; To be added next release
  /*  
    p_aq = (GPanel) i.addChild(new GPanel(20,50,820,170,false,"",(byte)0,false,i,gw));
    _aq_ring_radius = (GLabel) p_aq.addChild(new GLabel(400,20,"Ring radius (km): ",GLabel.RIGHT_ALIGN,ga.NONE,p_aq,gw));
    t_aq_ring_radius = (GTextEntry) p_aq.addChild(new GTextEntry(410,14,50,p_aq,gw,String.valueOf(DEFAULT_AQ_RING_SIZE),ga.NONE));
    _aq_deny_travel_prob = (GLabel) p_aq.addChild(new GLabel(400,50,"Probability of denying crossing border: ",GLabel.RIGHT_ALIGN,ga.NONE,p_aq,gw));
    t_aq_deny_travel_prob = (GTextEntry) p_aq.addChild(new GTextEntry(410,44,50,p_aq,gw,String.valueOf(DEFAULT_AQ_DENY_TRAVEL),ga.NONE));
    _aq_period = (GLabel) p_aq.addChild(new GLabel(400,80,"Period of restriction (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_aq,gw));
    t_aq_period = (GTextEntry) p_aq.addChild(new GTextEntry(410,74,50,p_aq,gw,String.valueOf(DEFAULT_AQ_PERIOD),ga.NONE));
    _aq_delay = (GLabel) p_aq.addChild(new GLabel(400,110,"Delay to create/extend zone (days): ",GLabel.RIGHT_ALIGN,ga.NONE,p_aq,gw));
    t_aq_delay = (GTextEntry) p_aq.addChild(new GTextEntry(410,104,50,p_aq,gw,String.valueOf(DEFAULT_AQ_DELAY),ga.NONE));
*/ 
    
    // Final initialisation
    
    interventions = new GPanel[] {p_border,p_treatment,p_proph,p_vacc,p_hq,p_pc,p_bt/*,p_aq*/};
    l_type.setSelected(0);
    changeInterventionType();
    
    
    
  }
  
  
}

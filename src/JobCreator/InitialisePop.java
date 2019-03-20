/* InitialisePop.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: set age bands and initial susceptibility page  
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

import java.io.DataOutputStream;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GButton;
import GKit.GLabel;
import GKit.GPage;
import GKit.GPanel;
import GKit.GSlider;
import GKit.GTable;
import GKit.GTextCell;
import GKit.GTextEntry;
import GKit.GTickBox;
import GKit.GWindow;


public class InitialisePop implements GPage {
  GWindow gw;
  GSim ga;
  GSlider gs_age;
  GLabel _susc_age,_age_label,_all_ages;
  GTickBox tb_all_ages;
  GLabel _le_x_lt;
  GButton b_ag_merge,b_ag_add;
  GTextEntry t_age_from,t_age_to;
  
  
  
  GTable gt_agesusc;
  
  
  static final int SUSC_ENTRY = 701;
  static final int TICK_AGES = 702;
  public final int MERGE_AGES = 703;
  public final int SELECT_AGE = 704;
  public final int AGE_FROM = 705;
  public final int AGE_TO = 706;
  public final int ADD_AGEGROUP = 707;
  
  
  static final String DEFAULT_SUSC = "0.5";
  
  String[] DEFAULT_AGES = new String[] {"0..5","5..10","10..15","15..20","20..25","25..30","30..35","35..40","40..45","45..50",
      "50..55","55..60","60..65","65..70","70..75","75..80","80..85","85..90","90..95","95..120"};
  
  
  
  
  public int getMinEvent() { return 700; }
  public int getMaxEvent() { return 799; }
  
  public InitialisePop(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
  }
  
  public void createDefaultXML(Element root) {
    Node ages_node=root.selectSingleNode("agegroups");
    if (ages_node!=null) root.remove(ages_node);
    Element units_el = root.addElement("agegroups");
    units_el.addAttribute("no",String.valueOf(DEFAULT_AGES.length));
    
    for (int i=0; i<DEFAULT_AGES.length; i++) {
      Element group = units_el.addElement("band");
      group.addAttribute("min",DEFAULT_AGES[i].substring(0,DEFAULT_AGES[i].indexOf(".")));
      group.addAttribute("max",DEFAULT_AGES[i].substring(DEFAULT_AGES[i].indexOf(".")+2));
    }

    Node ip_node=root.selectSingleNode("initpop");
    if (ip_node!=null) {
      root.remove(ip_node);
    }
    int countAgeBands = Integer.parseInt(root.selectSingleNode("agegroups").valueOf("@no"));
    Element ip_el=root.addElement("initpop");
    Element susc_age = ip_el.addElement("susc_age");
    for (int i=0; i<countAgeBands; i++) {
      susc_age.addElement("susc").addAttribute("band",String.valueOf(i)).addAttribute("value","0.5");
    }
  }
  
  public void saveXML(Element root) {
    Node units_node=root.selectSingleNode("agegroups");
    if (units_node!=null) {
      root.remove(units_node);
    }
    Element units_el = root.addElement("agegroups");
    units_el.addAttribute("no",String.valueOf(gt_agesusc.countRows()));
    for (int i=0; i<gt_agesusc.countRows(); i++) {
      Element group = units_el.addElement("band");
      String s = gt_agesusc.getValue(0, i);
      group.addAttribute("min",s.substring(0,s.indexOf(".")));
      group.addAttribute("max",s.substring(s.indexOf(".")+2));
    }
  
    Node ip_node=root.selectSingleNode("initpop");
    if (ip_node!=null) {
      root.remove(ip_node);
    }
    Element ip_el=root.addElement("initpop");
    Element susc_age = ip_el.addElement("susc_age");
    for (int i=0; i<gt_agesusc.countRows(); i++) {
      susc_age.addElement("susc").addAttribute("band",String.valueOf(i)).addAttribute("value",gt_agesusc.getValue(1,i));
    }
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(gt_agesusc.countRows()));
      for (int b=0; b<gt_agesusc.countRows(); b++) {
        String s = gt_agesusc.getValue(0,b);
        dos.writeInt(Integer.reverseBytes(Integer.parseInt(s.substring(0,s.indexOf(".")))));
        dos.writeInt(Integer.reverseBytes(Integer.parseInt(s.substring(s.indexOf(".")+2))));
        dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(gt_agesusc.getValue(1,b))))));
      }
      
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {}
  
  public void loadXML(Element root) {
    while (gt_agesusc.countRows()>0) gt_agesusc.removeRow(0);
    Node ages_el = root.selectSingleNode("agegroups");
    int no_groups = Integer.parseInt(ages_el.valueOf("@no"));
    for (int i=0; i<no_groups; i++) {
      Node group = ages_el.selectSingleNode("band["+(i+1)+"]");
      String s = group.valueOf("@min")+".."+group.valueOf("@max");
      gt_agesusc.addRow(new String[] {s,""});
    }

    Node ip_node = root.selectSingleNode("initpop");
    if (ip_node!=null) {
      Node susc_age = ip_node.selectSingleNode("susc_age");
      for (int i=0; i<no_groups; i++) {
        gt_agesusc.setValue(1,i,susc_age.selectSingleNode("susc["+(i+1)+"]").valueOf("@value"));
      }
    }
  }
      
  public void doFunction(int func,Object component) {
    if (func==SELECT_AGE) selectAge();
    else if (func==MERGE_AGES) mergeAges();
    else if (func==AGE_FROM) ageFrom();
    else if (func==AGE_TO) ageTo();
    else if (func==ADD_AGEGROUP) addAgeGroup();
    else if (func==SUSC_ENTRY) { suscEntry((GTextCell)component); }
    else if (func==TICK_AGES) tick_ages(); 
  }
  
  public void selectAge() {
    int first=gt_agesusc.getSelectionTop();
    int bottom=gt_agesusc.getSelectionBottom();
    boolean mergeOk=((first>=0) && (bottom>=0) && (first!=bottom));
    if (mergeOk!=b_ag_merge.isEnabled()) { 
      b_ag_merge.setEnabled(mergeOk);
    }
  }
  
  public void ageFrom() {
    ageTo();
  }
  
  public void ageTo() {
    boolean proceed=true;
    int from=-1;
    int to=-1;
    try {
      from = Integer.parseInt(t_age_from.getText());
      to = Integer.parseInt(t_age_to.getText());
    } catch (Exception e) {
      proceed=false;
    }
    if (proceed) {
      boolean okToAdd=((from<to) && (from>0) && (to>0) && (from<=120) && (to<=120));
      if (okToAdd!=b_ag_add.isEnabled()) {
        b_ag_add.setEnabled(okToAdd);
      }
    }
  }
  
  public void addAgeGroup() {
    int from = Integer.parseInt(t_age_from.getText());
    int to = Integer.parseInt(t_age_to.getText());
    int i=0;
    boolean done=false;
    while (i<gt_agesusc.countRows()) {
      String entry = gt_agesusc.getValue(0,i);
      int _from=Integer.parseInt(entry.substring(0,entry.indexOf(".")));
      int _to=Integer.parseInt(entry.substring(entry.indexOf(".")+2));
      if ((from==_from) && (to==_to)) { // Age group already exists..
        done=true;
        i=gt_agesusc.countRows();
      } 
      
      else if ((from<=_from) && (to>=_to)) {  // Trivial case:   ADD (10..20)  when (13..15) already exists.
        gt_agesusc.removeRow(i);         //                 Remove (13..15)
      }                                  // Job not finished yet.
                                                             
      // Now three cases: EG1. (ADD 10..20) when (5..25) exists (total inclusion)
      //                  EG2. (ADD 5..20)  when (5..25) exists (ie, same beginning)
      //                  EG3. (ADD 10..25) when (5..25) exists (ie, same end)
      
      else if ((from>=_from) && (from<=_to) && (to>=_from) && (to<=_to)) {      
        gt_agesusc.removeRow(i);                     // Remove original (5..25)
        int shift=0;
        if (_from!=from) {                                                  //     IF NOT Eg2:
          gt_agesusc.addRow(new String[] {_from+".."+from,String.valueOf(DEFAULT_SUSC)},i);  // Add early part (5..10)
          shift++;
        }
        gt_agesusc.addRow(new String[] {from+".."+to,String.valueOf(DEFAULT_SUSC)},i+shift);  // ADD NEW middle section
        shift++;
        if (to!=_to) {                                                      //     IF NOT EG3
          gt_agesusc.addRow(new String[] {to+".."+_to,String.valueOf(DEFAULT_SUSC)},i+shift);  // Add late part (20..25)
        }
        i=gt_agesusc.countRows();
        done=true;   // We know job is done now.
        

        //  Next case:   ADD (20..30) when (25..35) exists
        
      } else if ((from<=_from) && (to>_from) && (to<_to)) {
        gt_agesusc.removeRow(i);
        gt_agesusc.addRow(new String[] {to+".."+_to,String.valueOf(DEFAULT_SUSC)},i);
        i++;
        
        // Next case:   ADD (30..40) when (25..35) exists
      } else if ((to>=_to) && (from>_from) && (from<_to)) {
        gt_agesusc.removeRow(i);
        gt_agesusc.addRow(new String[] {_from+".."+from,String.valueOf(DEFAULT_SUSC)},i);
        i++;
      } else i++;
    }
 
    
    // Final check - fill in any gaps.
    
    if (!done) {
      i=0;
      while (i<gt_agesusc.countRows()) {
        String entry = gt_agesusc.getValue(0,i);
        int _from = Integer.parseInt(entry.substring(0,entry.indexOf(".")));
        if (_from>from) {
          gt_agesusc.addRow(new String[] {from+".."+to,String.valueOf(DEFAULT_SUSC)});
          i=gt_agesusc.countRows();
        }
        i++;
      }
    }
    gt_agesusc.paintOn(gw.bi(),gw.g2d());
    ga.setUnSaved(true);
    gw.requestRepaint();
    
  }
  
  public void mergeAges() {
    int top = gt_agesusc.getSelectionTop();
    int bottom = gt_agesusc.getSelectionBottom();
    String first = gt_agesusc.getValue(0,top);
    String last = gt_agesusc.getValue(0,bottom);
    String oldSusc = gt_agesusc.getValue(1,top);
    int i_first = Integer.parseInt(first.substring(0,first.indexOf(".")));
    int j_last = Integer.parseInt(last.substring(last.indexOf(".")+2));
    for (int i=bottom; i>=top; i--) gt_agesusc.removeRow(i);
    gt_agesusc.addRow(new String[] {String.valueOf(i_first+".."+j_last),oldSusc},top);
    gt_agesusc.clearSelection();
    b_ag_merge.setEnabled(false);
    gt_agesusc.paintOn(gw.bi(),gw.g2d());
    ga.setUnSaved(true);
    gw.requestRepaint();
    
  }
  
  
  public void tick_ages() {
    tb_all_ages.setSelected(!tb_all_ages.isSelected());
    tb_all_ages.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }

  public void suscEntry(GTextCell gt) {
    if (tb_all_ages.isSelected()) {
      for (int i=0; i<gt_agesusc.countRows(); i++) {
        gt_agesusc.setValue(1,i,gt.getValue());
      }
    }
    gt_agesusc.paintOn(gw.bi(),gw.g2d());
    changeVal();
  }
  
  public void changeVal() {
    ga.setUnSaved(true);
    gw.requestRepaint();
  }
  
 
  
        
  public void initGUI(GPanel p) {
    
    gt_agesusc=(GTable) p.addChild(new GTable(20,20,200,450,p,gw,SUSC_ENTRY,SELECT_AGE));
    gt_agesusc.addColumn("Age","");
    gt_agesusc.addColumn("Susceptibility","0.5");
    gt_agesusc.setColEditable(0, false);
    gt_agesusc.setFixedColHeader(true,true);
    gt_agesusc.setColWidth(0, 60);
    gt_agesusc.setColWidth(1,118);
    gt_agesusc.setColHeaderSelectable(false);
    gt_agesusc.setColSelectable(1,false);
    gt_agesusc.setSelectionModel(GTable.LINEAR_SELECTION);
   
    b_ag_add = (GButton) p.addChild(new GButton(170,490,50,22,p,gw,ADD_AGEGROUP,"Add"));
    b_ag_merge = (GButton) p.addChild(new GButton(20,533,125,22,p,gw,MERGE_AGES,"Merge Groups"));
    t_age_from = (GTextEntry) p.addChild(new GTextEntry(20,490,40,p,gw,"1",AGE_FROM));
    _le_x_lt = (GLabel) p.addChild(new GLabel(82,496,"\u2264 a <",GLabel.CENTRE_ALIGN,ga.NONE,p,gw));
    t_age_to = (GTextEntry) p.addChild(new GTextEntry(105,490,40,p,gw,"20",AGE_TO));
    b_ag_merge.setEnabled(false);
    
    _all_ages = (GLabel) p.addChild(new GLabel(50,584,"Link all ages",GLabel.LEFT_ALIGN,ga.NONE,p,gw));
    tb_all_ages = (GTickBox) p.addChild(new GTickBox(20,580,p,gw,TICK_AGES));
    tb_all_ages.setSelected(false);

    
  }
}

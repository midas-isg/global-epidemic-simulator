/* ParamGrid.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for setting per-admin-unit parameters 
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
import java.util.ArrayList;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GButton;
import GKit.GList;
import GKit.GPage;
import GKit.GPanel;
import GKit.GTable;
import GKit.GWindow;

public class ParamGrid implements GPage {
  GWindow gw;
  GSim ga;
  GPanel panel;
  GButton b_apply, b_load, b_selectall, b_selectsibs;
  GList l_adminunits;                    // List of units 
  GTable gt_params;

  ArrayList<ArrayList<StringBuffer>> states;  // For each unit, for each param, string representation of value
  ArrayList<String> default_vals;    // The list of default values

   
  public ParamGrid(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
    states = new ArrayList<ArrayList<StringBuffer>>();
    default_vals = new ArrayList<String>();
  }
  
  
  public final static int SELECT_ALL = 503;
  //public final static int LINK_KIDS = 504;
  public final static int SELECT_SIBLINGS = 505;
  public final static int SELECT_LIST = 506;
  public final static int APPLY_SET = 507;
  public final static int VIEW_SET = 508;
  
  
  
  public int getMinEvent() { return 500; }
  public int getMaxEvent() { return 509; }
  
  public void saveXML(Element root) {
    Node param_node=root.selectSingleNode("spatialparams");
    if (param_node!=null) {
      root.remove(param_node);
    }
    int no_admin_units = root.selectSingleNode("adminunits").selectNodes("unit").size();
    Element param_el=root.addElement("spatialparams");

    for (int i=0; i<no_admin_units; i++) {
      Element au = param_el.addElement("au");
      au.addAttribute("no",String.valueOf(i));
      for (int j=0; j<gt_params.countRows(); j++) {
        au.addAttribute(gt_params.getValue(0,j),states.get(i).get(j).toString());
      }
    }
  }
  
  public void saveBinary(DataOutputStream dos, int unit) {
    try {
      ArrayList<StringBuffer> _params = states.get(unit);
      for (int j=0; j<_params.size(); j++) {                          //   Dump all parameter values for this unit
        dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(_params.get(j).toString())))));
      }

    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {
    for (int i=0; i<states.size(); i++) {
      for (int j=0; j<states.get(i).size(); j++) {
        states.get(i).get(j).setLength(0);
      }
      states.get(i).clear();
    }
    states.clear();
  }
  
  public void createDefaultXML(Element root) {
    saveXML(root);
  }
  
  public void loadXML(Element root) {
    Node param_node = root.selectSingleNode("spatialparams");
    if (param_node!=null) {
      int no_admin_units = root.selectSingleNode("adminunits").selectNodes("unit").size();
      for (int i=0; i<no_admin_units; i++) {
        Element au = (Element) param_node.selectSingleNode("au["+(i+1)+"]");
        ArrayList<StringBuffer> a_sb = new ArrayList<StringBuffer>();
        for (int j=0; j<gt_params.countRows(); j++) {
          a_sb.add(new StringBuffer(au.valueOf("@"+gt_params.getValue(0,j))));
        }
        states.add(a_sb);
      }
    }
  }
  
  public void doFunction(int func, Object component) {
    if (func==SELECT_ALL) selectAll();
    else if (func==SELECT_SIBLINGS) selectSiblings();
    else if (func==SELECT_LIST) updateButtons();
    else if (func==VIEW_SET) viewSet();
    else if (func==APPLY_SET) applySet();
  }
  
  public void selectAll() {
    for (int i=0; i<l_adminunits.countEntries(); i++) {
      l_adminunits.setSelected(i);
    }
    updateButtons();
    l_adminunits.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void selectSiblings() {
    int sib=l_adminunits.getSelected();
    String[] s = l_adminunits.getEntry(sib).split(",");
    for (int i=0; i<l_adminunits.countEntries(); i++) {
      String[] s_comp = l_adminunits.getEntry(i).split(",");
      if (s.length==s_comp.length) {
        boolean sibling=true;
        for (int j=s.length-2; j>=0; j--) {
          if (!s[j].equals(s_comp[j])) {
            sibling=false;
            j=-1;
          }
        }
        if (sibling) {
          l_adminunits.setSelected(i);
        }
      }
    }
    updateButtons();
  }
  
  
  public void viewSet() {
    int i = l_adminunits.getSelectedIndexes()[0];
    ArrayList<StringBuffer> uvalues = states.get(i);
    for (int j=0; j<uvalues.size(); j++) {
      gt_params.setValue(1,j,uvalues.get(j).toString());
    }
  }
  
  public void applySet() {
    boolean changes=false;
    int[] sel = l_adminunits.getSelectedIndexes();
    for (int i=0; i<sel.length; i++) {
      for (int j=0; j<gt_params.countRows(); j++) {
        StringBuffer buf = states.get(sel[i]).get(j);
        if (!buf.toString().equals(gt_params.getValue(1,j))) {
          buf.setLength(0);
          buf.append(gt_params.getValue(1,j));
          changes=true;
        }
      }
    }
    ga.setUnSaved(changes);
  }
  
  public void updateButtons() {
    b_selectsibs.setEnabled(l_adminunits.countSelection()==1);
    b_load.setEnabled(l_adminunits.countSelection()==1);
    b_apply.setEnabled(l_adminunits.countSelection()>0);
  }
  
  public void addParameter(String s,String _default) {
    gt_params.addRow(new String[] {s,_default});
    for (int i=0; i<states.size(); i++) {
      states.get(i).add(new StringBuffer(_default));
    }
    default_vals.add(_default);
    gw.requestRepaint();
  }
  
  public void removeParameter(int i) {
    gt_params.removeRow(i);
    for (int k=0; k<states.size(); k++) {
      StringBuffer b= states.get(k).get(i);
      b.setLength(0);
      b=null;
      states.get(k).remove(i);
    }
    default_vals.remove(i);
    gw.requestRepaint();

  }
  
   
  public void addAdminUnit(String s) {
    l_adminunits.addEntry(s);
    ArrayList<StringBuffer> a_sb = new ArrayList<StringBuffer>();
    for (int j=0; j<gt_params.countRows(); j++) {
      a_sb.add(new StringBuffer(default_vals.get(j)));
    }
    states.add(a_sb);
    
    
  }
  
  public void addAdminUnit(int index,String s) {
    l_adminunits.addEntry(index,s);
    ArrayList<StringBuffer> a_sb = new ArrayList<StringBuffer>();
    for (int j=0; j<gt_params.countRows(); j++) {
      a_sb.add(new StringBuffer(default_vals.get(j)));
    }
    states.add(index,a_sb);
   }
  
  public void removeAdminUnit(int index) {
    l_adminunits.removeEntry(index);
    states.remove(index);
  }
  
  public void removeAllAdminUnits() {
    for (int i=states.size()-1; i>=0; i--) removeAdminUnit(i);
  }
  
  public void initParams() {
    addParameter("B_spatial","0.075");
    addParameter("Spatial_Krn_a","4.0");
    addParameter("Spatial_Krn_b","3.0");
    addParameter("Spatial_Krn_cut","200.0");
    addParameter("B_household","0.94");
    
    addParameter("B_P1","0.47");
    addParameter("P_P1_group","0.9");    
    addParameter("P1_sympt_absent","0.8");
    addParameter("P1_sympt_absent_com","1.2");    
    addParameter("P1_severe_absent","1.0");
    addParameter("P1_severe_absent_com","1.0");
    
    addParameter("B_P2","0.47");
    addParameter("P_P2_group","0.9");    
    addParameter("P2_sympt_absent","0.8");
    addParameter("P2_sympt_absent_com","1.2");    
    addParameter("P2_severe_absent","1.0");
    addParameter("P2_severe_absent_com","1.0");
    
    addParameter("B_P3","0.47");
    addParameter("P_P3_group","0.9");    
    addParameter("P3_sympt_absent","0.8");
    addParameter("P3_sympt_absent_com","1.2");    
    addParameter("P3_severe_absent","1.0");
    addParameter("P3_severe_absent_com","1.0");
    
    addParameter("B_P4","0.47");
    addParameter("P_P4_group","0.9");
    addParameter("P4_sympt_absent","0.5");
    addParameter("P4_sympt_absent_com","1.2");    
    addParameter("P4_severe_absent","0.8");
    addParameter("P4_severe_absent_com","1.0");
    
    
    addParameter("p_symptomatic","0.5");
    addParameter("p_sympt_detect","0.9");
    addParameter("mul_clinical_inf","1.2");
    addParameter("p_severe","0.5");
    addParameter("p_severe_detect","0.95");    
    addParameter("mul_severe_inf","2.0");
    
    
    addParameter("seasonal_lat_max","1.0");
    addParameter("seasonal_lat_min","0.0");
    addParameter("seasonal_temporal_offset","0.0");
  }
  
  public void initGUI(GPanel p) {
    panel=p;
    gt_params = (GTable) p.addChild(new GTable(20,20,300,500,p,gw,ga.NONE,ga.NONE));
    gt_params.addColumn("Param","");
    gt_params.addColumn("Value","0.5");
    gt_params.setColEditable(0, false);
    gt_params.setFixedColHeader(true,true);
    gt_params.setColWidth(0, 200);
    gt_params.setColWidth(1,80);
    gt_params.setColHeaderSelectable(false);
    gt_params.setColSelectable(1,false);
    gt_params.setColSelectable(0,false);
    gt_params.setSelectionModel(GTable.LINEAR_SELECTION);
    l_adminunits = (GList) p.addChild(new GList(360,20,350,500,p,new String[] {},gw,GList.MULTI_SELECTION,SELECT_LIST));
    b_apply = (GButton) p.addChild(new GButton(30,600,80,25,p,gw,APPLY_SET,"Apply >>"));
    b_load = (GButton) p.addChild(new GButton(160,600,80,25,p,gw,VIEW_SET,"<< View"));
    b_selectall = (GButton) p.addChild(new GButton(540,600,120,25,p,gw,SELECT_ALL,"Select All"));
    b_selectsibs = (GButton) p.addChild(new GButton(400,600,120,25,p,gw,SELECT_SIBLINGS,"Select Siblings"));    
    updateButtons();
    
  }
}

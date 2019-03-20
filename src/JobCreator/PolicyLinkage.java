/* PolicyLinkage.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for applying policies to admin units 
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

import java.awt.FontMetrics;
import java.io.DataOutputStream;
import java.util.ArrayList;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GLabel;
import GKit.GPage;
import GKit.GPanel;
import GKit.GSlider;
import GKit.GTickBox;
import GKit.GVerticalLabel;
import GKit.GWindow;

public class PolicyLinkage implements GPage {
  GWindow gw;
  GSim ga;
  GPanel panel;
  GSlider gs_horiz,gs_vert;
  GLabel _no_policies,_no_admins;
  GTickBox tb_linkall,tb_linksibs;//,tb_linkkids;
  GLabel _tb_linkall,_tb_linksibs;//,_tb_linkkids;
  int first_admin;
  int first_intv;
  int no_admin;
  int no_intv;
  static int max_intv_screen = 16;
  static int max_admin_screen = 12;
  
  int t_startx=200;
  int t_spacingx=30;
  int t_starty=200;
  int t_spacingy=30;
  
  ArrayList<GLabel> ll_adminunits;
  ArrayList<GVerticalLabel> ll_interventions;
  ArrayList<ArrayList<GTickBox>> tickboxes;
  ArrayList<StringBuffer> states;                 // For each admin unit, list of 0 and 1 for policies on or off
  ArrayList<String> intvs;
  ArrayList<String> units;
  ArrayList<String> short_units;
   
  public PolicyLinkage(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
    ll_adminunits = new ArrayList<GLabel>();
    ll_interventions = new ArrayList<GVerticalLabel>();
    tickboxes = new ArrayList<ArrayList<GTickBox>>();
    
    states = new ArrayList<StringBuffer>();
    intvs = new ArrayList<String>();
    units = new ArrayList<String>();
    short_units = new ArrayList<String>();
  }
  
  
  public final static int TICKBOX = 400;
  public final static int SLIDE_ADM = 401;
  public final static int SLIDE_INT = 402;
  public final static int LINK_ALL = 403;
  public final static int LINK_KIDS = 404;
  public final static int LINK_SIBLINGS = 405;
  
  public int getMinEvent() { return 400; }
  public int getMaxEvent() { return 499; }
  
  public void doFunction(int func, Object component) {
    if (func==TICKBOX) tick((GTickBox)component);
    if (func==SLIDE_ADM) slide_adm();
    if (func==SLIDE_INT) slide_int();
    if (func==LINK_ALL) link_all();
  //  if (func==LINK_KIDS) link_kids();
    if (func==LINK_SIBLINGS) link_sibs();
  }
  
  
  public void saveXML(Element root) {
    Node intv_node=root.selectSingleNode("policylinkage");
    if (intv_node!=null) {
      root.remove(intv_node);
    }
    int no_interventions = root.selectSingleNode("interventions").selectNodes("intervention").size();
    Element intv_el=root.addElement("policylinkage");

    for (int i=0; i<no_interventions; i++) {
      Element linkage = intv_el.addElement("intv");
      linkage.addAttribute("no",String.valueOf(i));
      StringBuffer sb = new StringBuffer();
      boolean found_yes=false;
      boolean found_no=false;
      for (int j=0; j<states.size(); j++) {
        if (states.get(j).charAt(i)=='1') {
          found_yes=true;
          sb.append("1");
        } else {
          found_no=true;
          sb.append("0");
        }
      }
      if ((found_yes) && (found_no)) linkage.addAttribute("admins",sb.toString());
      else if (found_yes) linkage.addAttribute("admins","ALL");
      else if (found_no) linkage.addAttribute("admins","NONE");
    }
  }
  
  public void saveBinary(DataOutputStream dos, int unit) {
    try {
      StringBuffer _links = states.get(unit);
      int count=0;
      for (int i=0; i<_links.length(); i++) count+=_links.charAt(i)=='1'?1:0;
      dos.writeInt(Integer.reverseBytes(count));
      for (int i=0; i<_links.length(); i++) {
        if (_links.charAt(i)=='1') dos.writeInt(Integer.reverseBytes(i));
      }
      
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {
    removeAllAdminUnits();
    removeAllInterventions();
  }
  
  public void loadXML(Element root) {
    int no_admin_units = root.selectSingleNode("adminunits").selectNodes("unit").size();
    Node intv_el = root.selectSingleNode("policylinkage");
    if (intv_el!=null) {
    
      int no_interventions = root.selectSingleNode("interventions").selectNodes("intervention").size();
      for (int i=0; i<no_interventions; i++) {
        Element linkage = (Element) (intv_el.selectSingleNode("intv["+(i+1)+"]"));
        String sb = linkage.valueOf("@admins");
        if (sb.equals("NONE")) {
          for (int j=0; j<no_admin_units; j++) {
            states.get(j).setCharAt(i,'0');
          }
        } else if (sb.equals("ALL")) {
          for (int j=0; j<no_admin_units; j++) {
            states.get(j).setCharAt(i,'1');
          }
        } else {
          for (int j=0; j<no_admin_units; j++) {
            states.get(j).setCharAt(i,sb.charAt(j));
          }
        }
      }
    }
  }
  
  public void createDefaultXML(Element root) {
    saveXML(root);
  }


  
  public void link_all() {
    tb_linkall.setSelected(!tb_linkall.isSelected());
    if (tb_linkall.isSelected()) {
      tb_linksibs.setSelected(false);
//      tb_linkkids.setSelected(false);
      tb_linksibs.paintOn(gw.bi(),gw.g2d());
//      tb_linkkids.paintOn(gw.bi(),gw.g2d());      
    }
    tb_linkall.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  /*
  public void link_kids() {
    tb_linkkids.setSelected(!tb_linkkids.isSelected());
    if (tb_linkkids.isSelected()) {
      tb_linksibs.setSelected(false);
      tb_linkall.setSelected(false);
      tb_linksibs.paintOn(gw.bi(),gw.g2d());
      tb_linkall.paintOn(gw.bi(),gw.g2d());      
    }
    tb_linkkids.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  */
  public void link_sibs() {
    tb_linksibs.setSelected(!tb_linksibs.isSelected());
    if (tb_linksibs.isSelected()) {
//      tb_linkkids.setSelected(false);
      tb_linkall.setSelected(false);
//      tb_linkkids.paintOn(gw.bi(),gw.g2d());
      tb_linkall.paintOn(gw.bi(),gw.g2d());      
    }
    tb_linksibs.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void slide_adm() {
    first_admin=gs_vert.getValue();
    updateGrid();
  }
  
  public void slide_int() {
    first_intv=gs_horiz.getValue();
    updateGrid();
  }
  
  public void tick(GTickBox gt) {
    int x_index = (gt.getX()-t_startx)/t_spacingx;
    int y_index = (gt.getY()-t_starty)/t_spacingy;
    char c = states.get(y_index+first_admin).charAt(x_index+first_intv);
    c=(c=='0')?'1':'0';
    if (tb_linkall.isSelected()) {
      for (int i=0; i<units.size(); i++) {
        states.get(i).setCharAt(x_index+first_intv,c);
      }
      updateGrid();
    } else if (tb_linksibs.isSelected()) {
      String[] s = units.get(first_admin+y_index).split(",");
      for (int i=0; i<units.size(); i++) {
        String[] s_comp = units.get(i).split(",");
        if (s.length==s_comp.length) {
          boolean sibling=true;
          for (int j=s.length-2; j>=0; j--) {
            if (!s[j].equals(s_comp[j])) {
              sibling=false;
              j=-1;
            }
          }
          if (sibling) {
            states.get(i).setCharAt(x_index+first_intv,c);
          }
        }
      }
      updateGrid();
    } else {
      states.get(y_index+first_admin).setCharAt(x_index+first_intv,c);
      tickboxes.get(y_index).get(x_index).setSelected(c=='1');
      tickboxes.get(y_index).get(x_index).paintOn(gw.bi(),gw.g2d());  
    }
    ga.setUnSaved(true);
    gw.requestRepaint();
    
  }
  
  public void updateGrid() {
    for (int i=0; i<ll_interventions.size(); i++) {
      if (i<no_intv) ll_interventions.get(i).unPaint(gw.bi(),gw.g2d());
    }
    for (int i=0; i<ll_adminunits.size(); i++) {
      if (i<no_admin) ll_adminunits.get(i).unPaint(gw.bi(),gw.g2d());
    }
    for (int j=0; j<max_intv_screen; j++) {
      if (j<no_intv) {
        ll_interventions.get(j).setText(intvs.get(j+first_intv));
        ll_interventions.get(j).setHeight(0);
        ll_interventions.get(j).setVisible(true);
      } else ll_interventions.get(j).setVisible(false);
      //ll_interventions.get(j).paintOn(gw.bi(),gw.g2d());
    }
    
    for (int i=0; i<max_admin_screen; i++) {
      if (i<no_admin) {
        ll_adminunits.get(i).setText(short_units.get(i+first_admin));
        ll_adminunits.get(i).setHeight(0);
        ll_adminunits.get(i).setVisible(true);
      } else ll_adminunits.get(i).setVisible(false);
//      ll_adminunits.get(i).paintOn(gw.bi(),gw.g2d());
      
      for (int j=0; j<max_intv_screen; j++) {
        if ((i<no_admin) && (j<no_intv)) {
          tickboxes.get(i).get(j).setSelected(states.get(i+first_admin).charAt(j+first_intv)=='1');
          tickboxes.get(i).get(j).setVisible(true);
        } else tickboxes.get(i).get(j).setVisible(false);
        //tickboxes.get(i).get(j).paintOn(gw.bi(),gw.g2d());
      }
    }
    //panel.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void addIntervention(String s,boolean init) {
    FontMetrics fm = gw.g2d().getFontMetrics(gw.getGCS().getLabelFont());
    if (fm.stringWidth(s)>(t_starty-20)) {
      s=s.substring(0,s.length()-1);
      while (fm.stringWidth(s)>(t_starty-20)) s=s.substring(0,s.length()-1);
      s+="..";
    }
    intvs.add(s);
    no_intv++;
    if (no_intv>max_intv_screen) {
      no_intv=max_intv_screen;
      first_intv++;
    }
    for (int i=0; i<states.size(); i++) {
      states.get(i).append('0');
    }
    if (!init) ga.setUnSaved(true);
    gs_horiz.updateSliderSize(intvs.size(),max_intv_screen);
    
    
  }
  
  public void removeIntervention(int i) {
    intvs.remove(i);
    for (int k=0; k<states.size(); k++) {
      states.get(k).deleteCharAt(i);
    }
    if (no_intv+first_intv>intvs.size()) {
      if (first_intv>0) first_intv--;
      else no_intv--;
    }
    ga.setUnSaved(true);
    gs_horiz.updateSliderSize(intvs.size(),max_intv_screen);
  }
  
  public void removeAllInterventions() {
    for (int i=intvs.size()-1; i>=0; i--) removeIntervention(i);
  }
  
  public void renameIntervention(int i, String s) {
    FontMetrics fm = gw.g2d().getFontMetrics(gw.getGCS().getLabelFont());
    if (fm.stringWidth(s)>(t_starty-20)) {
      s=s.substring(0,s.length()-1);
      while (fm.stringWidth(s)>t_starty-20) s=s.substring(0,s.length()-1);
      s+="..";
    }
    ll_interventions.get(i).setText(s);
    ga.setUnSaved(true);
  } 
   
  public void addAdminUnit(String s) {
    addAdminUnit(units.size(),s);
    gs_vert.updateSliderSize(units.size(),max_admin_screen);

  }
  
  public void addAdminUnit(int index,String s) {
    String orig=new String(s);
    FontMetrics fm = gw.g2d().getFontMetrics(gw.getGCS().getLabelFont());
    if (fm.stringWidth(s)>t_startx-20) {
      s=s.substring(1,s.length());
      while (fm.stringWidth(s)>(t_startx-20)) s=s.substring(1,s.length());
      s=".."+s;
    }
    StringBuffer sb = new StringBuffer();
    for (int i=0; i<intvs.size(); i++) sb.append('0');
    states.add(index,sb);
    units.add(index,orig);
    short_units.add(index,s);
    no_admin++;
    if (no_admin>max_admin_screen) no_admin=max_admin_screen;
    gs_vert.updateSliderSize(units.size(),max_admin_screen);

  }
  
  public void removeAdminUnit(int index) {
    states.remove(index);
    units.remove(index);
    short_units.remove(index);
    if (no_admin+first_admin>states.size()) {
      if (first_admin>0) first_admin--;
      else no_admin--;
    }
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
  }
  
  public void removeAllAdminUnits() {
    for (int i=states.size()-1; i>=0; i--) removeAdminUnit(i);
  }
  
  public void preparePanel() {
    _no_admins.setVisible(units.size()==0);
    _no_policies.setVisible(intvs.size()==0);
    _no_admins.paintOn(gw.bi(),gw.g2d());
    _no_policies.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
    updateGrid();
  }
  
  
  public void initGUI(GPanel p) {
    panel=p;
    _no_policies = (GLabel) p.addChild(new GLabel(t_startx+((t_spacingx*max_intv_screen)/2),t_starty+((t_spacingy*max_admin_screen)/3),"No interventions have been set up",GLabel.CENTRE_ALIGN,0,p,gw));
    _no_admins = (GLabel) p.addChild(new GLabel(t_startx+((t_spacingx*max_intv_screen)/2),(int) (t_starty+((t_spacingy*max_admin_screen)/1.5)),"No administrative units have been set up",GLabel.CENTRE_ALIGN,0,p,gw));
    no_admin=0;
    no_intv=0;
    first_admin=0;
    first_intv=0;
    for (int i=0; i<max_admin_screen; i++) {
      ll_adminunits.add((GLabel) p.addChild(new GLabel(t_startx-10,4+t_starty+(i*t_spacingy),"Admin "+i,GLabel.RIGHT_ALIGN,0,p,gw)));
    }
    for (int i=0; i<max_intv_screen; i++) {
      ll_interventions.add((GVerticalLabel) p.addChild(new GVerticalLabel(5+t_startx+(i*t_spacingx),t_starty-10,"Intv "+i,GVerticalLabel.LEFT_ALIGN,GVerticalLabel.UP,0,p,gw)));
    }
    for (int j=0; j<max_admin_screen; j++) {
      ArrayList<GTickBox> row = new ArrayList<GTickBox>();
      for (int i=0; i<max_intv_screen; i++) {
        row.add((GTickBox) p.addChild(new GTickBox(t_startx+(i*t_spacingx),t_starty+(j*t_spacingy),p,gw,TICKBOX)));
      }
      tickboxes.add(row);
    }
    gs_horiz = (GSlider) p.addChild(new GSlider(t_startx,t_starty+(max_admin_screen*t_spacingy)+20,(max_intv_screen*t_spacingx),GSlider.SLIDE_HORIZ,p,0,0,0,SLIDE_INT,gw));
    gs_vert = (GSlider) p.addChild(new GSlider(t_startx+(max_intv_screen*t_spacingx)+20,t_starty,(max_admin_screen*t_spacingy),GSlider.SLIDE_VERT,p,0,0,0,SLIDE_ADM,gw));
    
    tb_linkall = (GTickBox) p.addChild(new GTickBox(t_startx+20,gs_horiz.getY()+20,p,gw,LINK_ALL));
    _tb_linkall = (GLabel) p.addChild(new GLabel(tb_linkall.getX()+32,tb_linkall.getY()+5,"Link all units",GLabel.LEFT_ALIGN,-1,p,gw));
    //tb_linkkids = (GTickBox) p.addChild(new GTickBox(t_startx+160,gs_horiz.getY()+20,p,gw,LINK_KIDS));
    //_tb_linkkids = (GLabel) p.addChild(new GLabel(tb_linkkids.getX()+32,tb_linkkids.getY()+5,"Link children",GLabel.LEFT_ALIGN,-1,p,gw));
    tb_linksibs = (GTickBox) p.addChild(new GTickBox(t_startx+300,gs_horiz.getY()+20,p,gw,LINK_SIBLINGS));
    _tb_linksibs = (GLabel) p.addChild(new GLabel(tb_linksibs.getX()+32,tb_linksibs.getY()+5,"Link siblings",GLabel.LEFT_ALIGN,-1,p,gw));
    tb_linkall.setSelected(false);
    //tb_linkkids.setSelected(false);
    tb_linksibs.setSelected(false);
    gs_horiz.updateSliderSize(intvs.size(),max_intv_screen);
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
  }
}

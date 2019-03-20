/* OutputSelect.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for choosing output facilities and locations 
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
import GKit.GTextEntry;
import GKit.GTickBox;
import GKit.GVerticalLabel;
import GKit.GWindow;

public class OutputSelect implements GPage {
  GWindow gw;
  GSim ga;
  GPanel panel,p_db,p_flat,p_movie;
  GSlider gs_vert;
  GLabel _no_admins;
  GTickBox tb_linkall,tb_linksibs;
  GLabel _tb_linkall,_tb_linksibs;
  
  GTickBox tb_dbtable,tb_flatfile,tb_pngs;
  GLabel _dbtable,_flatfile,_pngs,_flatfilepath,_flatfilename,_dbserver,_pngpath,_pngfilename;
  GTextEntry t_dbtable,t_flatfilepath,t_flatfile,t_pngs,t_dbserver,t_pngpath,t_pngfile;
  
  int first_admin;
  int no_admin;
  static int max_admin_screen = 14;
  static int max_params_screen = 6;
  
  int t_startx=200;
  int t_spacingx=60;
  int t_starty=140;
  int t_spacingy=30;
  
  ArrayList<GLabel> ll_adminunits;
  ArrayList<GTickBox> entries;
  
  ArrayList<Integer> selection;                 // For each admin unit, list of 0 and 1 for policies on or off
  ArrayList<String> units;
  ArrayList<String> short_units;
   
  public OutputSelect(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
    ll_adminunits = new ArrayList<GLabel>();
    entries = new ArrayList<GTickBox>();
    selection = new ArrayList<Integer>();
    units = new ArrayList<String>();
    short_units = new ArrayList<String>();
  }
  
  
  public final static int TICKBOX = 1000;
  public final static int SLIDE_ADM = 1001;
  public final static int LINK_ALL = 1003;
  public final static int LINK_SIBLINGS = 1005;
  public final static int CLICK_DB = 1006;
  public final static int CHANGE_VAL = 1007;
  public final static int CLICK_FLAT = 1008;
  public final static int CLICK_PNGS = 1009;
  
  public int getMinEvent() { return 1000; }
  public int getMaxEvent() { return 1099; }
  
  public boolean useFlat() {
    return tb_flatfile.isSelected();
  }
  
  public String getFlatDir() {
    return t_flatfilepath.getText();
    
  }
  
  public void saveXML(Element root) {
    Element adm_el=(Element)root.selectSingleNode("adminunits");
    int no_admin_units = adm_el.selectNodes("unit").size();

    for (int i=0; i<no_admin_units; i++) {
      Element au = (Element) adm_el.selectSingleNode("unit["+(i+1)+"]");
      au.addAttribute("log",String.valueOf(selection.get(i)));
    }
    Node out_node=root.selectSingleNode("output");
    if (out_node!=null) {
      root.remove(out_node);
    }
    Element out_el = root.addElement("output");
    out_el.addElement("db").addAttribute("on",(tb_dbtable.isSelected()?"1":"0"))
                           .addAttribute("server",t_dbserver.getText()).addAttribute("table",t_dbtable.getText());
    out_el.addElement("flat").addAttribute("on",(tb_flatfile.isSelected()?"1":"0"))
                             .addAttribute("path",t_flatfilepath.getText()).addAttribute("file",t_flatfile.getText());
    out_el.addElement("pngs").addAttribute("on",(tb_pngs.isSelected()?"1":"0"))
                             .addAttribute("path",t_pngpath.getText()).addAttribute("file",t_pngfile.getText());
  }
  
  public void saveBinary(DataOutputStream dos, int unit) {
    try {
      dos.writeInt(Integer.reverseBytes(selection.get(unit)));
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      int do_db = tb_dbtable.isSelected()?1:0;
      dos.writeInt(Integer.reverseBytes(do_db));
      if (do_db==1) {
        dos.writeInt(Integer.reverseBytes(t_dbserver.getText().length()));
        dos.writeBytes(t_dbserver.getText());
        dos.writeInt(Integer.reverseBytes(t_dbtable.getText().length()));
        dos.writeBytes(t_dbtable.getText());
      }
      int do_flat = tb_flatfile.isSelected()?1:0;
      dos.writeInt(Integer.reverseBytes(do_flat));
      if (do_flat==1) {
        dos.writeInt(Integer.reverseBytes(t_flatfilepath.getText().length()));
        dos.writeBytes(t_flatfilepath.getText());
        dos.writeInt(Integer.reverseBytes(t_flatfile.getText().length()));
        dos.writeBytes(t_flatfile.getText());
      }
    
      int do_pngs = tb_pngs.isSelected()?1:0;
      dos.writeInt(Integer.reverseBytes(do_pngs));
      if (do_pngs==1) {
        dos.writeInt(Integer.reverseBytes(t_pngpath.getText().length()));
        dos.writeBytes(t_pngpath.getText());
        dos.writeInt(Integer.reverseBytes(t_pngfile.getText().length()));
        dos.writeBytes(t_pngfile.getText());      
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {
    no_admin=0;
    first_admin=0;
    selection.clear();
  }
  
  public void createDefaultXML(Element root) {
    saveXML(root);
  }
  
  public void loadXML(Element root) {
    clear();
    Node adm_node = root.selectSingleNode("adminunits");
    int no_admin_units = adm_node.selectNodes("unit").size();
    for (int i=0; i<no_admin_units; i++) {
      Element unit = (Element) adm_node.selectSingleNode("unit["+(i+1)+"]");
      selection.add(Integer.parseInt(unit.valueOf("@log"))); 
      no_admin++;
    }
    Node out_node = root.selectSingleNode("output");
    
    Element db_el = (Element) out_node.selectSingleNode("db");
    tb_dbtable.setSelected(db_el.valueOf("@on").equals("1"));
    t_dbserver.setText(db_el.valueOf("@server"));
    t_dbtable.setText(db_el.valueOf("@table"));
    
    Element flat_el = (Element) out_node.selectSingleNode("flat");
    tb_flatfile.setSelected(flat_el.valueOf("@on").equals("1"));
    t_flatfilepath.setText(flat_el.valueOf("@path"));
    t_flatfile.setText(flat_el.valueOf("@file"));
    
    Element png_el = (Element) out_node.selectSingleNode("pngs");
    tb_pngs.setSelected(png_el.valueOf("@on").equals("1"));
    t_pngpath.setText(png_el.valueOf("@path"));
    t_pngfile.setText(png_el.valueOf("@file"));
  }

  
  
  public void doFunction(int func, Object component) {
    if (func==TICKBOX) editTickbox((GTickBox)component);
    else if (func==SLIDE_ADM) slide_adm();
    else if (func==LINK_ALL) link_all();
    else if (func==LINK_SIBLINGS) link_sibs();
    else if (func==CLICK_DB) clickDB();
    else if (func==CLICK_FLAT) clickFlat();
    else if (func==CLICK_PNGS) clickPNGs();
    else if (func==CHANGE_VAL) changeVal();
  }
  
  public void changeVal() {
    ga.setUnSaved(true);
  }
  
  public void clickDB() {
    tb_dbtable.setSelected(!tb_dbtable.isSelected());
    tb_dbtable.paintOn(gw.bi(),gw.g2d());
    changeVal();
    gw.requestRepaint();
  }
  
  public void clickFlat() {
    tb_flatfile.setSelected(!tb_flatfile.isSelected());
    tb_flatfile.paintOn(gw.bi(),gw.g2d());
    changeVal();
    gw.requestRepaint();
  }
  
  public void clickPNGs() {
    tb_pngs.setSelected(!tb_pngs.isSelected());
    tb_pngs.paintOn(gw.bi(),gw.g2d());
    changeVal();
    gw.requestRepaint();
  }
  
  
  
  public void link_all() {
    tb_linkall.setSelected(!tb_linkall.isSelected());
    if (tb_linkall.isSelected()) {
      tb_linksibs.setSelected(false);
      tb_linksibs.paintOn(gw.bi(),gw.g2d());
    }
    tb_linkall.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }

  public void link_sibs() {
    tb_linksibs.setSelected(!tb_linksibs.isSelected());
    if (tb_linksibs.isSelected()) {
      tb_linkall.setSelected(false);
      tb_linkall.paintOn(gw.bi(),gw.g2d());      
    }
    tb_linksibs.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void slide_adm() {
    first_admin=gs_vert.getValue();
    updateGrid();
  }
  
  public void editTickbox(GTickBox gt) {
    gt.setSelected(!gt.isSelected());
    gt.paintOn(gw.bi(),gw.g2d());
    int y = (int) ((gt.getY()-t_starty)/t_spacingy);
    int v = gt.isSelected()?1:0;
    selection.set(y,new Integer(v));
    if (tb_linkall.isSelected()) {
      for (int i=0; i<units.size(); i++) selection.set(i,new Integer(v));
      updateGrid();
    } else if (tb_linksibs.isSelected()) {
      String[] s = units.get(first_admin+y).split(",");
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
            selection.set(i,new Integer(v));
          }
        }
      }
      updateGrid();
    }
    ga.setUnSaved(true);
    gw.requestRepaint();
    
  }
  
  public void updateGrid() {
    for (int i=0; i<ll_adminunits.size(); i++) {
      if (i<no_admin) ll_adminunits.get(i).unPaint(gw.bi(),gw.g2d());
    }
    
    for (int i=0; i<max_admin_screen; i++) {
      if (i<no_admin) {
        ll_adminunits.get(i).setText(short_units.get(i+first_admin));
        ll_adminunits.get(i).setHeight(0);
        ll_adminunits.get(i).setVisible(true);
      } else ll_adminunits.get(i).setVisible(false);
     
      ll_adminunits.get(i).paintOn(gw.bi(),gw.g2d());
      
      if (i<no_admin) {
        entries.get(i).setVisible(true);
        entries.get(i).setSelected(selection.get(i+first_admin)==1);
      } else entries.get(i).setVisible(false);
      entries.get(i).paintOn(gw.bi(),gw.g2d());
    }
    gw.requestRepaint();
  }
  
  public void addAdminUnit(String s) {
    addAdminUnit(units.size(),s);
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
   }
  
  public void addAdminUnit(int index,String s) {
    String orig = new String(s);
    FontMetrics fm = gw.g2d().getFontMetrics(gw.getGCS().getLabelFont());
    if (fm.stringWidth(s)>t_startx-20) {
      s=s.substring(1,s.length());
      while (fm.stringWidth(s)>(t_startx-20)) s=s.substring(1,s.length());
      s=".."+s;
    }
    selection.add(new Integer(0));
    units.add(index,orig);
    short_units.add(index,s);
    no_admin++;
    if (no_admin>max_admin_screen) no_admin=max_admin_screen;
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
    gs_vert.updateBallFromVal();
    gs_vert.paintOn(gw.bi(),gw.g2d());
  }
  
  public void removeAdminUnit(int index) {
    selection.remove(index);
    units.remove(index);
    if (no_admin+first_admin>selection.size()) {
      if (first_admin>0) first_admin--;
      else no_admin--;
    }
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
    gs_vert.updateBallFromVal();
    gs_vert.paintOn(gw.bi(),gw.g2d());

  }
  
  public void removeAllAdminUnits() {
    for (int i=selection.size()-1; i>=0; i--) removeAdminUnit(i);
  }
  
  public void preparePanel() {
    _no_admins.setVisible(units.size()==0);
    _no_admins.paintOn(gw.bi(),gw.g2d());
    updateGrid();
    gw.requestRepaint();
    
  }
  
  public void initGUI(GPanel p) {
    panel=p;
    _no_admins = (GLabel) p.addChild(new GLabel(t_startx+((t_spacingx*max_params_screen)/2),(int) (t_starty+((t_spacingy*max_admin_screen)/1.5)),"No administrative units have been set up",GLabel.CENTRE_ALIGN,0,p,gw));
    no_admin=0;
    first_admin=0;
    for (int i=0; i<max_admin_screen; i++) {
      ll_adminunits.add((GLabel) p.addChild(new GLabel(t_startx-10,4+t_starty+(i*t_spacingy),"Admin "+i,GLabel.RIGHT_ALIGN,0,p,gw)));
    }
    p.addChild(new GVerticalLabel(5+t_startx,t_starty-10,"Log stats",GVerticalLabel.LEFT_ALIGN,GVerticalLabel.UP,0,p,gw));
    for (int j=0; j<max_admin_screen; j++) {
      entries.add((GTickBox) p.addChild(new GTickBox(t_startx,t_starty+(j*t_spacingy),p,gw,TICKBOX)));
    }
    gs_vert = (GSlider) p.addChild(new GSlider(t_startx+(t_spacingx),t_starty,(max_admin_screen*t_spacingy),GSlider.SLIDE_VERT,p,0,0,0,SLIDE_ADM,gw));
   
    
    tb_linkall = (GTickBox) p.addChild(new GTickBox(20,gs_vert.getY()+gs_vert.getHeight()+10,p,gw,LINK_ALL));
    _tb_linkall = (GLabel) p.addChild(new GLabel(tb_linkall.getX()+32,tb_linkall.getY()+5,"Link all units",GLabel.LEFT_ALIGN,-1,p,gw));
    tb_linksibs = (GTickBox) p.addChild(new GTickBox(160,gs_vert.getY()+gs_vert.getHeight()+10,p,gw,LINK_SIBLINGS));
    _tb_linksibs = (GLabel) p.addChild(new GLabel(tb_linksibs.getX()+32,tb_linksibs.getY()+5,"Link siblings",GLabel.LEFT_ALIGN,-1,p,gw));
    tb_linkall.setSelected(false);

    tb_linksibs.setSelected(false);

    p_db = (GPanel) p.addChild(new GPanel(330,80,350,100,false,"",(byte)0,false,p,gw));
    p_flat = (GPanel) p.addChild(new GPanel(330,190,350,100,false,"",(byte)0,false,p,gw));
    p_movie = (GPanel) p.addChild(new GPanel(330,300,350,100,false,"",(byte)0,false,p,gw));
    
    
    tb_dbtable = (GTickBox) p_db.addChild(new GTickBox(165,10,p_db,gw,CLICK_DB));
    _dbtable = (GLabel) p_db.addChild(new GLabel(145,14,"Write to database",GLabel.RIGHT_ALIGN,-1,p_db,gw));
    _dbserver = (GLabel) p_db.addChild(new GLabel(145,44,"Database Server:",GLabel.RIGHT_ALIGN,-1,p_db,gw));
    _dbtable = (GLabel) p_db.addChild(new GLabel(145,74,"Table:",GLabel.RIGHT_ALIGN,-1,p_db,gw));
    t_dbserver = (GTextEntry) p_db.addChild(new GTextEntry(165,40,150,p_db,gw,"",CHANGE_VAL));
    t_dbtable = (GTextEntry) p_db.addChild(new GTextEntry(165,70,150,p_db,gw,"",CHANGE_VAL));
    
    tb_flatfile = (GTickBox) p_flat.addChild(new GTickBox(165,10,p_flat,gw,CLICK_FLAT));
    _flatfile = (GLabel) p_flat.addChild(new GLabel(145,14,"Write flat files",GLabel.RIGHT_ALIGN,-1,p_flat,gw));
    _flatfilepath = (GLabel) p_flat.addChild(new GLabel(145,44,"Output Path:",GLabel.RIGHT_ALIGN,-1,p_flat,gw));
    _flatfilename = (GLabel) p_flat.addChild(new GLabel(145,74,"Filename Stub:",GLabel.RIGHT_ALIGN,-1,p_flat,gw));
    t_flatfilepath = (GTextEntry) p_flat.addChild(new GTextEntry(165,40,150,p_flat,gw,"",CHANGE_VAL));
    t_flatfile = (GTextEntry) p_flat.addChild(new GTextEntry(165,70,150,p_flat,gw,"",CHANGE_VAL));
    
    tb_pngs = (GTickBox) p_movie.addChild(new GTickBox(165,10,p_movie,gw,CLICK_PNGS));
    _pngs = (GLabel) p_movie.addChild(new GLabel(145,14,"Write PNG images",GLabel.RIGHT_ALIGN,-1,p_movie,gw));
    _pngpath = (GLabel) p_movie.addChild(new GLabel(145,44,"Output Path:",GLabel.RIGHT_ALIGN,-1,p_movie,gw));
    _pngfilename = (GLabel) p_movie.addChild(new GLabel(145,74,"Filename Stub:",GLabel.RIGHT_ALIGN,-1,p_movie,gw));
    t_pngpath = (GTextEntry) p_movie.addChild(new GTextEntry(165,40,150,p_movie,gw,"",CHANGE_VAL));
    t_pngfile = (GTextEntry) p_movie.addChild(new GTextEntry(165,70,150,p_movie,gw,"",CHANGE_VAL));
    
    gs_vert.updateSliderSize(units.size(),max_admin_screen);
  }
}

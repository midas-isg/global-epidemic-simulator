/* GSim.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: main GUI class. 
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
import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;

import org.dom4j.Document;
import org.dom4j.DocumentHelper;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;

import GKit.GApp;
import GKit.GColourScheme;
import GKit.GItem;
import GKit.GLabel;
import GKit.GLine;
import GKit.GPage;
import GKit.GPanel;
import GKit.GWindow;

public class GSim implements GApp {
  int loc_x,loc_y,wid,hei;
  String modelFile="";
  String modelName="";
  String modelAuthor="";
  String modelInfo="";
  
  String modelPath="models"+File.separator;
  String dataPath="data"+File.separator;
  String gadmPath="data"+File.separator;
  String binW64Path="";
  String binLinuxPath="";  
  
  GWindow gw;
  GPanel p_main,p_admin,p_main_menu,p_basic,p_intervent,p_plinkage,p_param,p_startup,p_seeding,p_ipop,p_sub,p_output;
  GLabel _admin,_basic,_intervent,_plinkage,_param,_startup,_seeding,_ipop,_sub,_output;
  GLine ln_admin,ln_basic,ln_intervent,ln_plinkage,ln_param,ln_startup,ln_seeding,ln_ipop,ln_sub,ln_output;
  
  private boolean unsaved=false;
  
  public boolean isUnSaved() { return unsaved; }
  
  public void setUnSaved(boolean b) { 
    unsaved=b;
  }
  
  public void save() {
    saveXML();
    unsaved=false;
  }
  
  
  final String NOT_SELECTED = new String(" - Not selected - ");
  final String NOT_AVAILABLE = new String(" - Not available - ");
  final String SIM_XML_ID = "GLOBAL_SIM_ID";
  final String SIM_VER = "1.0";

  
  public final int NONE = 0;
  public final int CLICK_AU = 1;
  public final int CLICK_SS = 2;
  public final int CLICK_IP = 3;
  public final int CLICK_PL = 4;
  public final int CLICK_PG = 5;
  public final int CLICK_PS = 6;
  public final int CLICK_IPOP = 7;
  public final int CLICK_SD = 8;
  public final int CLICK_SUB = 9;
  public final int CLICK_OS = 10;
  
  
  DBUtils db;
  AdminUnitPage au;
  BasicSettingsPage ss;
  InterventionsPage ip;
  PolicyLinkage pl;
  ParamGrid pg;
  StartupPage ps;
  SeedingPage sd;
  InitialisePop ipop;
  SubmissionPage sub;
  OutputSelect os;
  
  GPage[] pageList;
  GPanel[] pagePanels;
  GLine[] pageLines;
  GLabel[] pageLabels;
  
  private static final int LINUX=1;
  private static final int WIN=2;
  public int OPERATING_SYSTEM;
  
  public void createINIFile() {
    Document doc = DocumentHelper.createDocument();
    Element root = doc.addElement("ini");
    root.addAttribute("left","20");
    root.addAttribute("top","20");
    root.addAttribute("width","1070");
    root.addAttribute("height","700");
    root.addElement("modelpath").setText("models/");
    root.addElement("nodeconfigpath").setText("data/");    
    root.addElement("gadmpath").setText("data/");    
   
    File f = new File("gsim_conf.xml");
    if (f.exists()) f.delete();
    try {
      FileWriter fw = new FileWriter("gsim_conf.xml");
      doc.write(fw);
      fw.flush();
      fw.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void saveINIFile() {
    Document doc = DocumentHelper.createDocument();
    Element root = doc.addElement("ini");
    root.addAttribute("left",String.valueOf(gw.getLocation().x));
    root.addAttribute("top",String.valueOf(gw.getLocation().y));
    root.addAttribute("width",String.valueOf(gw.getWidth()));
    root.addAttribute("height",String.valueOf(gw.getHeight()));
    root.addElement("modelpath").setText(modelPath);
    root.addElement("nodeconfigpath").setText(dataPath);    
    root.addElement("gadmpath").setText(gadmPath);    
    root.addElement("binw64path").setText(binW64Path);
    root.addElement("binlinuxpath").setText(binLinuxPath);    
   
    File f = new File("gsim_conf.xml");
    if (f.exists()) f.delete();
    try {
      FileWriter fw = new FileWriter("gsim_conf.xml");
      doc.write(fw);
      fw.flush();
      fw.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void loadINIFile() {
    try {
      SAXReader reader = new SAXReader();
      Document doc = reader.read("gsim_conf.xml");
      Element root = doc.getRootElement();
      loc_x=Integer.parseInt(root.valueOf("@left"));
      loc_y=Integer.parseInt(root.valueOf("@top"));
      wid=Integer.parseInt(root.valueOf("@width"));
      hei=Integer.parseInt(root.valueOf("@height"));
      modelPath=root.selectSingleNode("modelpath").getText();
      dataPath=root.selectSingleNode("nodeconfigpath").getText();      
      gadmPath=root.selectSingleNode("gadmpath").getText();
      binW64Path=root.selectSingleNode("binw64path").getText();
      binLinuxPath=root.selectSingleNode("binlinuxpath").getText();      
      if (!gadmPath.endsWith(File.separator)) gadmPath+=File.separator;     
      if (!dataPath.endsWith(File.separator)) dataPath+=File.separator;
      if (!modelPath.endsWith(File.separator)) modelPath+=File.separator;      
    } catch (Exception e) {
      loc_x=20;
      loc_y=20;
      wid=1070;
      hei=700;
      e.printStackTrace(); }
  
  }
  
  public void createNewXML() {
    Document doc = DocumentHelper.createDocument();
    Element root = doc.addElement("model");
    Element meta = root.addElement("meta");
    meta.addAttribute("id",SIM_XML_ID);
    meta.addAttribute("ver",SIM_VER);
    meta.addAttribute("name",modelName);
    meta.addAttribute("author",modelAuthor);
    meta.addAttribute("info",modelInfo);
    for (int i=0; i<pageList.length; i++) pageList[i].clear();
    for (int i=0; i<pageList.length; i++) pageList[i].createDefaultXML(root);
    
    try {
      OutputFormat format = OutputFormat.createPrettyPrint();
      XMLWriter fw = new XMLWriter(new FileWriter(modelFile),format);
      fw.write(doc);
      fw.flush();
      fw.close();
    } catch (Exception e) { e.printStackTrace(); }
    
    
  }
  
  public void loadXML() {
    try {
      SAXReader reader = new SAXReader();
      Document doc = reader.read(modelFile);
      Element root = doc.getRootElement();
      for (int i=0; i<pageList.length; i++) pageList[i].clear();
      for (int i=0; i<pageList.length; i++) pageList[i].loadXML(root);
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void saveXML() {
    Document doc = DocumentHelper.createDocument();
    Element root = doc.addElement("model");
    Element meta = root.addElement("meta");
    meta.addAttribute("id",SIM_XML_ID);
    meta.addAttribute("ver",SIM_VER);
    meta.addAttribute("name",modelName);
    meta.addAttribute("author",modelAuthor);
    meta.addAttribute("info",modelInfo);
    for (int i=0; i<pageList.length; i++) pageList[i].saveXML(root);
  try {
      OutputFormat format = OutputFormat.createPrettyPrint();
      XMLWriter fw = new XMLWriter(new FileWriter(modelFile),format);
      fw.write(doc);
      fw.flush();
      fw.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void saveBinary(String file) {
    try {
      DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(file)));
      
      ss.saveBinary(dos);
      // Interventions first - because admin units will refer to them.
      ip.saveBinary(dos);
      au.saveBinary(dos); // Admin units also saves contents of the paramgrid, policylinkage, output (for each a.u.).
      sd.saveBinary(dos);
      ipop.saveBinary(dos);
      os.saveBinary(dos);   // Note os.saveBinary(dos,unit) is called from au.saveBinary
      sub.saveBinary(dos);
      dos.flush();
      dos.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
   
  public void doFunction(int func, Object component) {
    if ((func>=0) && (func<=99)) {
      if (func==CLICK_PS) show(ln_startup,p_startup,_startup);
      else if (ps.status==StartupPage.LOADED) {
        if (func==CLICK_SS) show(ln_basic,p_basic,_basic);
        else if (func==CLICK_IP) show(ln_intervent,p_intervent,_intervent);
        else if (func==CLICK_PL) { pl. preparePanel(); show(ln_plinkage,p_plinkage,_plinkage); }
        else if (func==CLICK_AU) show(ln_admin,p_admin,_admin);
        else if (func==CLICK_PG) show(ln_param,p_param,_param);
        else if (func==CLICK_SD) show(ln_seeding,p_seeding,_seeding);
        else if (func==CLICK_IPOP) show(ln_ipop,p_ipop,_ipop);
        else if (func==CLICK_SUB) show(ln_sub,p_sub,_sub); 
        else if (func==CLICK_OS) { os.preparePanel(); show(ln_output,p_output,_output); }
      }
    } else {
      if (pageList!=null) { // Just to trap for very first creation
        for (int i=0; i<pageList.length; i++) {
          if ((func>=pageList[i].getMinEvent()) && (func<=pageList[i].getMaxEvent())) {
            pageList[i].doFunction(func,component);
            i=pageList.length;
          }
        }
      }
    }
    gw.checkRepaint();
  }
  
  public double graphFunction(GItem gc,int func, double x_val) {
    return ss.graphFunction(gc,func,x_val);
  }
  
  public void show(GLine line,GPanel panel, GLabel label) {
    for (int i=0; i<pageList.length; i++) {
      pagePanels[i].setVisible(false);
      pageLines[i].setVisible(false);
      pageLabels[i].setTextColour(new Color(gw.getGCS().getText()));
    }
    line.setVisible(true);
    panel.setVisible(true);
    label.setTextColour(new Color(gw.getGCS().getListHighText()));
    gw.update();
    gw.requestRepaint();
  }
   
  
  public void initGUI() {
    db = new DBUtils(this);
    db.initDB();
    gw = new GWindow(wid,hei,loc_x,loc_y,this,GColourScheme.getDefault());
    p_main = new GPanel(0,0,wid-2,hei-2,true,"Global Simulator",(byte)46,true,null,gw);
    gw.setMaster(p_main);
    pl = new PolicyLinkage(gw,this);
    pg = new ParamGrid(gw,this);
    os = new OutputSelect(gw,this);
    au = new AdminUnitPage(gw,this,pl,pg,os);
    ss = new BasicSettingsPage(gw,this);
    ip = new InterventionsPage(gw,this,pl);
    ps = new StartupPage(gw,this);
    sd = new SeedingPage(gw,this);
    ipop = new InitialisePop(gw,this);
    sub = new SubmissionPage(gw,this,au,ss,os);
    
    p_main_menu = (GPanel) p_main.addChild(new GPanel(10,46,180,646,true,"",(byte)0,false,null,gw));
    _startup = (GLabel) p_main_menu.addChild(new GLabel(10,20,"Startup",GLabel.LEFT_ALIGN,CLICK_PS,p_main_menu,gw));
    ln_startup = (GLine) p_main_menu.addHiddenChild(new GLine(20,40,190,40,new Color(gw.getGCS().getHedge()),p_main_menu,gw));
    
    _basic = (GLabel) p_main_menu.addChild(new GLabel(10,50,"Basic Setup",GLabel.LEFT_ALIGN,CLICK_SS,p_main_menu,gw));
    ln_basic = (GLine) p_main_menu.addHiddenChild(new GLine(20,70,190,70,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    _admin = (GLabel) p_main_menu.addChild(new GLabel(10,80,"Admin units",GLabel.LEFT_ALIGN,CLICK_AU,p_main_menu,gw));
    ln_admin = (GLine) p_main_menu.addHiddenChild(new GLine(20,100,190,100,new Color(gw.getGCS().getHedge()),p_main_menu,gw));
   
    _intervent = (GLabel) p_main_menu.addChild(new GLabel(10,110,"Create Interventions",GLabel.LEFT_ALIGN,CLICK_IP,p_main_menu,gw));
    ln_intervent = (GLine) p_main_menu.addHiddenChild(new GLine(20,130,190,130,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    _plinkage = (GLabel) p_main_menu.addChild(new GLabel(10,140,"Apply Interventions",GLabel.LEFT_ALIGN,CLICK_PL,p_main_menu,gw));
    ln_plinkage = (GLine) p_main_menu.addHiddenChild(new GLine(20,160,190,160,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    _param = (GLabel) p_main_menu.addChild(new GLabel(10,170,"Spatial Parameters",GLabel.LEFT_ALIGN,CLICK_PG,p_main_menu,gw));
    ln_param = (GLine) p_main_menu.addHiddenChild(new GLine(20,190,190,190,new Color(gw.getGCS().getHedge()),p_main_menu,gw));
    
    _ipop= (GLabel) p_main_menu.addChild(new GLabel(10,200,"Initialise Population",GLabel.LEFT_ALIGN,CLICK_IPOP,p_main_menu,gw));
    ln_ipop = (GLine) p_main_menu.addHiddenChild(new GLine(20,220,190,220,new Color(gw.getGCS().getHedge()),p_main_menu,gw));
        
    _seeding = (GLabel) p_main_menu.addChild(new GLabel(10,230,"Seeding",GLabel.LEFT_ALIGN,CLICK_SD,p_main_menu,gw));
    ln_seeding = (GLine) p_main_menu.addHiddenChild(new GLine(20,250,190,250,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    _output = (GLabel) p_main_menu.addChild(new GLabel(10,260,"Output Options",GLabel.LEFT_ALIGN,CLICK_OS,p_main_menu,gw));
    ln_output = (GLine) p_main_menu.addHiddenChild(new GLine(20,280,190,280,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    _sub = (GLabel) p_main_menu.addChild(new GLabel(10,290,"Job Submission",GLabel.LEFT_ALIGN,CLICK_SUB,p_main_menu,gw));
    ln_sub = (GLine) p_main_menu.addHiddenChild(new GLine(20,310,190,310,new Color(gw.getGCS().getHedge()),p_main_menu,gw));

    p_admin = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    au.initGUI(p_admin);
    
    p_basic = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw)); 
    ss.initGUI(p_basic);
    
    p_intervent = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw)); 
    ip.initGUI(p_intervent);
    
    p_plinkage = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    pl.initGUI(p_plinkage);

    p_param = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    pg.initGUI(p_param);
    pg.initParams();
    
    p_startup = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    ps.initGUI(p_startup);
    
    p_seeding = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    sd.initGUI(p_seeding);
    
    p_ipop = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    ipop.initGUI(p_ipop);
    
    p_output = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    os.initGUI(p_output);
    
    p_sub = (GPanel) p_main.addHiddenChild(new GPanel(200,46,864,646,true,"",(byte)0,false,null,gw));
    sub.initGUI(p_sub);
    
    pageList = new GPage[] {au,ss,ip,pl,pg,ps,sd,ipop,os,sub};
    pagePanels = new GPanel[] {p_admin,p_basic,p_intervent,p_plinkage,p_param,p_startup,p_seeding,p_ipop,p_output,p_sub};
    pageLines = new GLine[] {ln_admin,ln_basic,ln_intervent,ln_plinkage,ln_param,ln_startup,ln_seeding,ln_ipop,ln_output,ln_sub};
    pageLabels = new GLabel[] {_admin,_basic,_intervent,_plinkage,_param,_startup,_seeding,_ipop,_output,_sub};

    show(ln_startup,p_startup,_startup);
   
    gw.update();
    gw.setVisible(true);
    gw.startFade();
  }
  
  public static void main(String[] args) {
    GSim gs = new GSim();
    String osystem = System.getProperty("os.name").toLowerCase();
    if (osystem.indexOf("win")>=0) gs.OPERATING_SYSTEM=WIN;
    else if ((osystem.indexOf("nix")>=0) || (osystem.indexOf("nux")>=0)) gs.OPERATING_SYSTEM=LINUX;
    else gs.OPERATING_SYSTEM=LINUX;  // Assume Win/linux for now.
    
    if (!new File("gsim_conf.xml").exists()) {
      gs.createINIFile();
    }
    gs.loadINIFile();
    gs.initGUI();
    gs.gw.requestRepaint();
  }
}

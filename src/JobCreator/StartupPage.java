/* StartupPage.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: choose model to edit/create new model 
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

import java.io.File;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

import GKit.GButton;
import GKit.GFilePanel;
import GKit.GLabel;
import GKit.GPage;
import GKit.GPanel;
import GKit.GTextEntry;
import GKit.GWindow;


public class StartupPage implements GPage {
  GWindow gw;
  GSim ga;
  GFilePanel fp_file;
  GButton b_load,b_create,b_cancel,b_doLoad,b_doCreate,b_doSave,b_doClose;
  GLabel _name,_author,_info,_file,_dir;
  GTextEntry t_name,t_author,t_info,t_file;
  GPanel p_info;
  int status;
  
  final String SIM_XML_ID = "GLOBAL_SIM_ID";
  final String SIM_VER = "1.0";
  
  public static final int NO_MODEL = 1;
  public static final int LOADING = 2;
  public static final int CREATING = 3;
  public static final int LOADED = 4;
  public static final int CLICK_LOAD = 600;
  public static final int CLICK_CREATE = 601;
  public static final int CLICK_DO_LOAD = 602;
  public static final int CLICK_DO_CREATE = 603;
  public static final int CLICK_DO_SAVE = 604;
  public static final int CLICK_DO_CLOSE = 605;
  public static final int CLICK_CANCEL = 606;
  public static final int CLICK_FILE = 607;
  public static final int CHANGE_DETAILS = 608;
  
  public int getMinEvent() { return 600; }
  public int getMaxEvent() { return 699; }
  
  public StartupPage(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
  }
  
  public void doFunction(int func,Object component) {
    if (func==CLICK_LOAD) clickLoad();
    else if (func==CLICK_CREATE) clickCreate();
    else if (func==CLICK_CANCEL) clickCancel();
    else if (func==CLICK_DO_LOAD) clickReallyLoad();
    else if (func==CLICK_DO_SAVE) clickSave();
    else if (func==CLICK_DO_CREATE) clickReallyCreate();
    else if (func==CLICK_DO_CLOSE) clickClose();
    else if (func==CLICK_FILE) clickFile();
    else if (func==CHANGE_DETAILS) changeDetails();
  }
  
  public void clickFile() {
    String file = fp_file.getDirectory()+File.separator+fp_file.getFile();
    if ((file!=null) && (file.toUpperCase().endsWith(".XML"))) {
      try {
        SAXReader reader = new SAXReader();
        Document doc = reader.read(file);
        Element root = doc.getRootElement();
        if (root.getName().equals("model")) {
          Node meta = root.selectSingleNode("meta");
          if ((meta.valueOf("@id").equals(SIM_XML_ID)) && (meta.valueOf("@ver").equals(SIM_VER))) {
            t_name.setText(meta.valueOf("@name").toString());
            t_author.setText(meta.valueOf("@author").toString());
            t_info.setText(meta.valueOf("@info").toString());
            t_file.setText(fp_file.getFile());
          }
        }
      } catch (Exception e) {}
    } 
  } 
  
  
  public void clickLoad() {
    fp_file.setVisible(true);
    b_load.setVisible(false);
    b_create.setVisible(false);
    p_info.setVisible(true);
    fp_file.setY(20);
    p_info.setY(fp_file.getY()+fp_file.getHeight()+20);
    t_name.setText("");
    t_author.setText("");
    t_info.setText("");
    t_file.setText("");
   
    b_cancel.setVisible(true);
    b_doLoad.setVisible(true);
    gw.update();
    gw.requestRepaint();
    status=LOADING;
  }
  
  public void changeDetails() {
    ga.modelFile=fp_file.getDirectory()+File.separator+t_file.getText();
    ga.modelName=t_name.getText();
    ga.modelAuthor=t_author.getText();
    ga.modelInfo=t_info.getText();
    ga.setUnSaved(true);
  }
  
  public void clickCreate() {
    fp_file.setVisible(true);
    b_load.setVisible(false);
    b_create.setVisible(false);
    p_info.setVisible(true);
    t_name.setText("Model Name");
    t_author.setText("Author Information");
    t_info.setText("Model Information");
    t_file.setText("Model.xml");
    b_cancel.setVisible(true);
    b_doCreate.setVisible(true);
    p_info.setY(20);
    fp_file.setY(p_info.getY()+p_info.getHeight()+20);
    gw.update();
    gw.requestRepaint();
    status=CREATING;
  }
  
  public void clickCancel() {
    fp_file.setVisible(false);
    b_load.setVisible(true);
    b_create.setVisible(true);
    p_info.setVisible(false);
    b_cancel.setVisible(false);
    b_doCreate.setVisible(false);
    b_doLoad.setVisible(false);
    gw.update();
    gw.requestRepaint();
    status=NO_MODEL;
  }
  
  public void clickReallyCreate() {
    ga.modelFile=fp_file.getDirectory()+File.separator+t_file.getText();
    ga.modelName=t_name.getText();
    ga.modelAuthor=t_author.getText();
    ga.modelInfo=t_info.getText();
    ga.createNewXML();
    ga.loadXML();
    status=LOADED;
    b_doSave.setVisible(true);
    b_doClose.setVisible(true);
    b_doCreate.setVisible(false);
    b_cancel.setVisible(false);
    fp_file.setVisible(false);
    ga.doFunction(ga.CLICK_SS,null);
    gw.update();
    gw.repaint();
    
  }
  
  public void clickReallyLoad() {
    try {
      ga.modelFile=fp_file.getDirectory()+File.separator+t_file.getText();
      ga.modelName=t_name.getText();
      ga.modelAuthor=t_author.getText();
      ga.modelInfo=t_info.getText();
      ga.loadXML(); 
      status=LOADED;
      b_doSave.setVisible(true);
      b_doClose.setVisible(true);
      b_doLoad.setVisible(false);
      b_cancel.setVisible(false);
      fp_file.setVisible(false);
      p_info.setY(20);
      ga.doFunction(ga.CLICK_SS,null);
      gw.update();
      gw.repaint();
      
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clickClose() {
    status=NO_MODEL;
    clickCancel();
    b_doSave.setVisible(false);
    b_doClose.setVisible(false);
    b_doClose.paintOn(gw.bi(),gw.g2d());
    b_doSave.paintOn(gw.bi(),gw.g2d());
    gw.update();
    gw.repaint();
    
  }
  
  public void clickSave() {
    ga.save();
  }
 
  
  public void initGUI(GPanel p) {
    fp_file = (GFilePanel) p.addChild(new GFilePanel(100,170,p.getWidth()-200,400,false,"",(byte)0,false,p,gw,CLICK_FILE));
    fp_file.setDirectory(ga.modelPath);
    b_load = (GButton) p.addChild(new GButton((p.getWidth()/2)-150,70,300,22,p,gw,CLICK_LOAD,"Load Existing Model"));
    b_create = (GButton) p.addChild(new GButton((p.getWidth()/2)-150,100,300,22,p,gw,CLICK_CREATE,"Create New Model"));
    status=NO_MODEL;
    fp_file.setVisible(false);
    fp_file.setExtension("xml");
    p_info = (GPanel) p.addHiddenChild(new GPanel(100,20,(p.getWidth())-200,130,false,"",(byte)0,false,p,gw));
    _name = (GLabel) p_info.addChild(new GLabel(100,15,"Name:",GLabel.RIGHT_ALIGN,0,p_info,gw));
    _author = (GLabel) p_info.addChild(new GLabel(100,44,"Author:",GLabel.RIGHT_ALIGN,0,p_info,gw));
    _info = (GLabel) p_info.addChild(new GLabel(100,73,"Info:",GLabel.RIGHT_ALIGN,0,p_info,gw));
    _file = (GLabel) p_info.addChild(new GLabel(100,102,"File:",GLabel.RIGHT_ALIGN,0,p_info,gw));
    t_name = (GTextEntry) p_info.addChild(new GTextEntry(120,10,p_info.getWidth()-140,p_info,gw,"Model Name",CHANGE_DETAILS));
    t_author = (GTextEntry) p_info.addChild(new GTextEntry(120,39,p_info.getWidth()-140,p_info,gw,"Author Information",CHANGE_DETAILS));
    t_info = (GTextEntry) p_info.addChild(new GTextEntry(120,68,p_info.getWidth()-140,p_info,gw,"Model Information",CHANGE_DETAILS));
    t_file = (GTextEntry) p_info.addChild(new GTextEntry(120,99,p_info.getWidth()-140,p_info,gw,"Model.xml",CHANGE_DETAILS));
    b_cancel = (GButton) p.addHiddenChild(new GButton((p.getWidth()/2)-150,600,140,22,p,gw,CLICK_CANCEL,"Cancel"));
    b_doLoad = (GButton) p.addHiddenChild(new GButton((p.getWidth()/2)+10,600,140,22,p,gw,CLICK_DO_LOAD,"Load"));
    b_doCreate = (GButton) p.addHiddenChild(new GButton((p.getWidth()/2)+10,600,140,22,p,gw,CLICK_DO_CREATE,"Create"));
    b_doSave = (GButton) p.addHiddenChild(new GButton((p.getWidth()/2)+10,300,140,22,p,gw,CLICK_DO_SAVE,"Save"));
    b_doClose = (GButton) p.addHiddenChild(new GButton((p.getWidth()/2)-150,300,140,22,p,gw,CLICK_DO_CLOSE,"Close"));
    
  }
  public void createDefaultXML(Element root) {}
  public void loadXML(Element root) {}
  public void saveXML(Element root) {}
  public void clear() {}
}

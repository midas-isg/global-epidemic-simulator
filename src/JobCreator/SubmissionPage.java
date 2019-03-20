/* SumbissionPage.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: mash configuration and job submission page 
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
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

import GKit.GButton;
import GKit.GFilePanel;
import GKit.GLabel;
import GKit.GLine;
import GKit.GList;
import GKit.GListHeader;
import GKit.GPage;
import GKit.GPanel;
import GKit.GTextEntry;
import GKit.GWindow;


public class SubmissionPage implements GPage {
  GWindow gw;
  GSim ga;
  GButton b_setExeDir,b_setInSPDir,b_setModSPDir,b_okDir,b_cancelDir,b_setInDir,b_setModSPWork,b_preparemash,b_genmap,b_preparejob,b_runjob,b_runmash;
  GList l_jobtype,l_platform;
  GListHeader lh_jobtype,lh_platform;
  GLabel _platform,_jobtype,_exedir,_orig_sp_data,_mod_sp_data,_gen_mod_path;
  GLabel _indir;
  GTextEntry t_inDir,t_exeDir,t_orig_sp_data,t_mod_sp_data,t_currentDir,t_mod_sp_work;
  GTextEntry t_launch_mash,t_launch_sim;
  

  GFilePanel fp_dir;
  AdminUnitPage au;
  BasicSettingsPage bs;
  OutputSelect os;
  
  
  String[] job_type_names;
  int[] job_no_nodes;
  int[] job_uids;
  ArrayList<String> job_type_info;
  
  final static String WIN64 = new String("Win-64");
  final static String LINUX = new String("Linux");
  String[] platforms = new String[] {WIN64,LINUX};
  
  final static int CHANGE_VAL=901;
  final static int SET_EXE_DIR=902;
  
  final static int SET_IN_SP_DIR=903;
  final static int SET_MOD_SP_DIR=904;
  final static int SET_MOD_WORK_DIR=905;
  final static int GENERATE_MAP=906;
  final static int PREPARE_MASH=907;
  
  final static int CHOOSE_DIR=908;
  final static int CANCEL_DIR=909;
  final static int SET_EXE_IN_DIR=910;
  final static int CHOOSE_PLATFORM=911;
  final static int PREPARE_JOB=912;
  final static int RUN_MASH=913;
  final static int RUN_JOB=914;
  
  public SubmissionPage(GWindow _gw, GSim _ga, AdminUnitPage _au,BasicSettingsPage _bs,OutputSelect _os) {
    gw=_gw;
    ga=_ga;
    au=_au;
    bs=_bs;
    os=_os;
  }
  
  
  public int getMinEvent() { return 900; }
  public int getMaxEvent() { return 999; }
  
  
  public void saveXML(Element root) {
    Node s_node=root.selectSingleNode("submission");
    if (s_node!=null) {
      root.remove(s_node);
    }
    Element s_el=root.addElement("submission");
    s_el.addElement("jobtype").addText(String.valueOf(job_uids[l_jobtype.getSelected()])); // This should be an ID
    s_el.addElement("platform").addText(String.valueOf(l_platform.getEntry(l_platform.getSelected()))); // This should be an ID    
        
    s_el.addElement("orig_sp_data_dir").addText(String.valueOf(t_orig_sp_data.getText()));
    s_el.addElement("mod_sp_data_dir").addText(String.valueOf(t_mod_sp_data.getText()));
    s_el.addElement("mash_working_dir").addText(String.valueOf(t_mod_sp_work.getText()));
    s_el.addElement("mash_launch_cmd").addText(String.valueOf(t_launch_mash.getText()));
    
    s_el.addElement("exe_dir").addText(String.valueOf(t_exeDir.getText()));
    s_el.addElement("exe_data_dir").addText(String.valueOf(t_inDir.getText()));
    s_el.addElement("sim_launch_cmd").addText(String.valueOf(t_launch_sim.getText()));

  }
  
  public void writeBinary(DataOutputStream dos, int id, int code,int no_nodes, int[] list_nodes,String country,int paramset) {
    try {
      dos.writeInt(Integer.reverseBytes(code));
      dos.writeInt(Integer.reverseBytes(id));
      dos.writeInt(Integer.reverseBytes(no_nodes));
      for (int i=0; i<no_nodes; i++) dos.writeInt(Integer.reverseBytes(list_nodes[i]));
      String hh_file = t_inDir.getText()+File.separator+"popul__"+country+"_"+String.valueOf(paramset)+".bin";
      dos.writeInt(Integer.reverseBytes(hh_file.length()));
      dos.writeBytes(hh_file);
     
      for (int i=0; i<bs.getNoPlaceTypes(); i++) {
        String est_file = t_inDir.getText()+File.separator+"est__"+country+"_"+i+"__"+String.valueOf(paramset)+".bin";
        dos.writeInt(Integer.reverseBytes(est_file.length()));
        dos.writeBytes(est_file);
      }
      
    } catch (Exception e) { e.printStackTrace(); }
      
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      String[] parts = job_type_info.get(l_jobtype.getSelected()).split("\t");   // Format: no_parts, lsi_file, then list of {name,param,id,grump_cod,node_list}
      int index=0;
      int no_sections = Integer.parseInt(parts[index++]);
      dos.writeInt(Integer.reverseBytes(no_sections));
      // Then overlay file...
      String overlay_file = t_inDir.getText()+File.separator+"overlay.bin";
      dos.writeInt(Integer.reverseBytes(overlay_file.length()));
      dos.writeBytes(overlay_file);
      
      index++; // Skip the LSI name
      index++; // Skip the travel matrix name
      for (int i=0; i<no_sections; i++) {
        String[] nodeList = parts[index+4].split(",");
        int[] intNodeList = new int[nodeList.length];
        for (int k=0; k<nodeList.length; k++) intNodeList[k]=Integer.parseInt(nodeList[k]);
        writeBinary(dos,Integer.parseInt(parts[index+2]), Integer.parseInt(parts[index+3]),nodeList.length,intNodeList,parts[index],Integer.parseInt(parts[index+1]));
        index+=5;
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {}
  
  public void createDefaultXML(Element root) {
    Node s_node=root.selectSingleNode("submission");
    if (s_node!=null) {
      root.remove(s_node);
    }
    Element s_el=root.addElement("submission");
    s_el.addAttribute("jobtype","-1");
    s_el.addAttribute("exe_dir","");
    s_el.addAttribute("orig_sp_data_dir","");
    s_el.addAttribute("mod_sp_data_dir","");    
  }
  
  public void copyFile(String from, String to) {
    DataInputStream dis=null;
    DataOutputStream dos=null;
    try {
      dis = new DataInputStream(new BufferedInputStream(new FileInputStream(from)));
      dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(to)));
      while (true) dos.writeByte(dis.readByte());
      
    } catch (Exception e) { 
      if ((dos!=null) && (dis!=null)) {
        try {
          dos.flush(); dos.close(); dis.close();
        } catch (Exception e2) { e2.printStackTrace(); }
      } else e.printStackTrace();
    }
  }
  
  public void copyAll(String from, String to, boolean exec) {
    File[] list = new File(from).listFiles();
    for (int i=0; i<list.length; i++) {
      copyFile(list[i].getAbsolutePath(),to+File.separator+list[i].getName());
      if (exec) new File(to+File.separator+list[i].getName()).setExecutable(true);
    }
  }
  
  public void loadXML(Element root) {
    Node s_node=root.selectSingleNode("submission");
    int uid = Integer.parseInt(s_node.valueOf("jobtype"));
    boolean found=false;
    for (int i=0; i<job_uids.length; i++) {
      if (uid==job_uids[i]) {
        l_jobtype.setSelected(i);
        i=job_uids.length;
        found=true;
      }
    }
    if (!found) System.out.println("ERROR - couldn't find job type with UID="+uid+" - update to later version?");
    String platform=s_node.valueOf("platform");
    for (int i=0; i<l_platform.countEntries(); i++) {
      if (l_platform.getEntry(l_platform.getSelected()).equals(platform)) {
        l_platform.setSelected(i);
        i=l_platform.countEntries();
      }
    }
    
    t_exeDir.setText(s_node.valueOf("exe_dir"));
    t_orig_sp_data.setText(s_node.valueOf("orig_sp_data_dir"));
    t_mod_sp_data.setText(s_node.valueOf("mod_sp_data_dir"));
    t_mod_sp_work.setText(s_node.valueOf("mash_working_dir"));
    t_launch_mash.setText(s_node.valueOf("mash_launch_cmd"));
    t_inDir.setText(s_node.valueOf("exe_data_dir"));
    t_launch_sim.setText(s_node.valueOf("sim_launch_cmd"));
  }

  
  
  public void doFunction(int func,Object component) {
    if (func==CHANGE_VAL) changeVal();
    else if (func==SET_EXE_DIR) setDir(t_exeDir);
    else if (func==SET_IN_SP_DIR) setDir(t_orig_sp_data);
    else if (func==SET_MOD_SP_DIR) setDir(t_mod_sp_data);  
    else if (func==SET_EXE_IN_DIR) setDir(t_inDir);
    else if (func==CHOOSE_DIR) chooseDir();
    else if (func==CANCEL_DIR) cancelDir();
    else if (func==GENERATE_MAP) generateMap();
    else if (func==CHOOSE_PLATFORM) choosePlatform();
    else if (func==PREPARE_MASH) prepareMash();
    else if (func==PREPARE_JOB) prepareJob();
    else if (func==RUN_MASH) runMash();
    else if (func==RUN_JOB) runSim();
  }
  
 public void choosePlatform() {
   
 }
  
 public void prepareMash() {
   try {
     PrintWriter PW = new PrintWriter(new File(t_mod_sp_work.getText()+File.separator+"mash.script"));
     PW.println(t_mod_sp_work.getText()+File.separator+"map_raster.map");
     PW.println(t_mod_sp_work.getText()+File.separator+"overlay.bin");
     SAXReader reader = new SAXReader();
     Document doc = reader.read(ga.dataPath+"jobtypes.xml");
     Element root = doc.getRootElement();
     int no_types = root.selectNodes("j").size();
     
     for (int i=1; i<=no_types; i++) {
       Node n = root.selectSingleNode("j["+i+"]");
       int uid = Integer.parseInt(n.valueOf("@uid"));
       if (uid==job_uids[l_jobtype.getSelected()]) {
         int no_parts = n.selectNodes("c").size();
         for (int j=1; j<=no_parts; j++) {
           Node c = n.selectSingleNode("c["+j+"]");
           PW.println(t_orig_sp_data.getText()+File.separator+"popul__"+c.valueOf("@n")+"_"+c.valueOf("@p"));
           PW.println(t_orig_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_0__"+c.valueOf("@p"));
           PW.println(t_orig_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_1__"+c.valueOf("@p"));
           PW.println(t_orig_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_2__"+c.valueOf("@p"));
           PW.println(t_orig_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_3__"+c.valueOf("@p"));           
           PW.println(t_mod_sp_data.getText()+File.separator+"popul__"+c.valueOf("@n")+"_"+c.valueOf("@p")+".bin");
           PW.println(t_mod_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_0__"+c.valueOf("@p")+".bin");
           PW.println(t_mod_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_1__"+c.valueOf("@p")+".bin");
           PW.println(t_mod_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_2__"+c.valueOf("@p")+".bin");
           PW.println(t_mod_sp_data.getText()+File.separator+"est__"+c.valueOf("@n")+"_3__"+c.valueOf("@p")+".bin");
         }
         i=no_types;
         PW.close();
       }
     }
     if (l_platform.getEntry(l_platform.getSelected()).equals(WIN64)) {
       PW = new PrintWriter(new File(t_mod_sp_work.getText()+File.separator+"run.bat"));
       PW.println(t_mod_sp_work.getText()+File.separator+"mashadmin.exe "+t_mod_sp_work.getText()+File.separator+"mash.script");
       PW.close();
       PW = new PrintWriter(new File(t_mod_sp_work.getText()+File.separator+"launch.bat"));
       PW.println(t_launch_mash.getText()+" "+t_mod_sp_work.getText()+File.separator+"run.bat");
       PW.close();
       copyFile(ga.binW64Path+File.separator+"MashAdminUnits"+File.separator+"mashadmin.exe",t_mod_sp_work.getText()+File.separator+"mashadmin.exe");
     } else {
       File f = new File(t_mod_sp_work.getText()+File.separator+"run.sh");
       PW=new PrintWriter(f);
       PW.println(t_mod_sp_work.getText()+File.separator+"mashadmin "+t_mod_sp_work.getText()+File.separator+"mash.script");
       PW.close();
       f.setExecutable(true);
       f = new File(t_mod_sp_work.getText()+File.separator+"launch.sh");
       PW=new PrintWriter(f);
       PW.println(t_launch_mash.getText()+" "+t_mod_sp_work.getText()+File.separator+"run.sh");
       PW.close();
       f.setExecutable(true);
       copyFile(ga.binLinuxPath+File.separator+"MashAdminUnits"+File.separator+"mashadmin",t_mod_sp_work.getText()+File.separator+"mashadmin");
       new File(t_mod_sp_work.getText()+File.separator+"mashadmin").setExecutable(true);
     }
     
   } catch (Exception e) { e.printStackTrace(); }
 }
 
 public void runMash() {
   try {
     if (l_platform.getEntry(l_platform.getSelected()).equals(WIN64)) {
       Runtime.getRuntime().exec(t_mod_sp_work.getText()+File.separator+"launch.bat");       
     } else {
       Runtime.getRuntime().exec(t_mod_sp_work.getText()+File.separator+"launch.sh");
     }
   } catch (Exception e) { e.printStackTrace(); }
 }
 
 public void runSim() {
   try {
     if (l_platform.getEntry(l_platform.getSelected()).equals(WIN64)) {
       Runtime.getRuntime().exec(t_exeDir.getText()+File.separator+"launch.bat");       
     } else {
       Runtime.getRuntime().exec(t_exeDir.getText()+File.separator+"launch.sh");
     }
   } catch (Exception e) { e.printStackTrace(); }
 }
 
 
 public void setDir(GTextEntry target) {
    t_currentDir=target;
    fp_dir.setDirectory(t_currentDir.getText());
    fp_dir.setVisible(true);
    fp_dir.paintOn(gw.bi(),gw.g2d());
    b_okDir.setVisible(true);
    b_cancelDir.setVisible(true);
    b_okDir.paintOn(gw.bi(),gw.g2d());
    b_cancelDir.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
    
  }
  
  public void chooseDir() {
    fp_dir.setVisible(false);
    fp_dir.paintOn(gw.bi(),gw.g2d());
    b_okDir.setVisible(false);
    b_cancelDir.setVisible(false);
    b_okDir.paintOn(gw.bi(),gw.g2d());
    b_cancelDir.paintOn(gw.bi(),gw.g2d());
    t_currentDir.setText(fp_dir.getDirectory());
    gw.update();
    gw.requestRepaint();
  }
  
  public void cancelDir() {
    fp_dir.setVisible(false);
    b_okDir.setVisible(false);
    b_cancelDir.setVisible(false);
    b_okDir.paintOn(gw.bi(),gw.g2d());
    b_cancelDir.paintOn(gw.bi(),gw.g2d());
    fp_dir.paintOn(gw.bi(),gw.g2d());
    t_currentDir.setText(fp_dir.getDirectory());
    gw.update();
    gw.requestRepaint();
  }
  
  public void changeVal() {
    ga.setUnSaved(true);
    gw.requestRepaint();
  }
    
  public void generateMap() {
    try {
      System.out.println("Creating map");
      au.createMapFile(t_mod_sp_work.getText()+File.separator+"map_raster.map");
    
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void prepareJob() {
    String[] parts = job_type_info.get(l_jobtype.getSelected()).split("\t");
    int no_nodes = job_no_nodes[l_jobtype.getSelected()];
    String lsiStub = parts[1];
    String trav = parts[2];
    for (int i=0; i<no_nodes; i++) {
      copyFile(ga.dataPath+lsiStub+"_"+String.valueOf(i)+".lsi",t_exeDir.getText()+File.separator+"config_"+String.valueOf(i)+".lsi");
    }
    copyFile(ga.dataPath+trav,t_exeDir.getText()+File.separator+"travel_matrix.bin");
    ga.saveBinary(t_exeDir.getText()+File.separator+"params.bin");
    

    // Scripts...
    
    try {
      if (os.useFlat()) {
        PrintWriter PW = new PrintWriter(os.getFlatDir()+File.separator+"admin_lookup.txt");
        for (int i=0; i<au.unit_info.size(); i++) {
          PW.println(String.valueOf(i)+"\t"+au.l_entries.getEntry(i));
        }
        PW.close();
      }
      if (l_platform.getEntry(l_platform.getSelected()).equals(WIN64)) {
        PrintWriter PW = new PrintWriter(t_exeDir.getText()+File.separator+"run.bat");
        if (no_nodes==1) 
          PW.println(t_exeDir.getText()+File.separator+"Sim.exe /in:"+t_exeDir.getText());
        else 
          PW.println("mpiexec -n "+String.valueOf(no_nodes)+" -wdir "+t_exeDir.getText()+" "+t_exeDir.getText()+File.separator+"Sim.exe /in:"+t_exeDir.getText());
        PW.flush();
        PW.close();
        PW = new PrintWriter(t_exeDir.getText()+File.separator+"launch.bat");
        String cmd = t_launch_sim.getText();
        cmd=cmd.replaceAll("%n",String.valueOf(no_nodes));
        PW.println(cmd+" "+t_exeDir.getText()+File.separator+"run.bat");
        PW.flush();
        PW.close();
        copyAll(ga.binW64Path+File.separator+"Sim",t_exeDir.getText(),false);

      } else {
        PrintWriter PW = new PrintWriter(t_exeDir.getText()+File.separator+"run.sh");
        if (no_nodes==1) 
          PW.println(t_exeDir.getText()+File.separator+"sim /in:"+t_exeDir.getText());
        else 
          PW.println("mpiexec -n "+String.valueOf(no_nodes)+" -wdir "+t_exeDir.getText()+" "+t_exeDir.getText()+File.separator+"sim /in:"+t_exeDir.getText());
        PW.flush();
        PW.close();
        PW = new PrintWriter(t_exeDir.getText()+File.separator+"launch.sh");
        String cmd = t_launch_sim.getText();
        cmd=cmd.replaceAll("%n",String.valueOf(no_nodes));
        PW.println(cmd+" "+t_exeDir.getText()+File.separator+"run.sh");
        PW.flush();
        PW.close();
        copyAll(ga.binLinuxPath+File.separator+"Sim",t_exeDir.getText(),true);
        new File(t_exeDir.getText()+File.separator+"run.sh").setExecutable(true);
        new File(t_exeDir.getText()+File.separator+"launch.sh").setExecutable(true);
        
      }
    } catch (Exception e) { e.printStackTrace(); }
    
  }
  
  public void initJobTypes() {
    try {
      
      SAXReader reader = new SAXReader();
      Document doc = reader.read(ga.dataPath+"jobtypes.xml");
      Element root = doc.getRootElement();
      job_type_info = new ArrayList<String>();
      
      int no_types = root.selectNodes("j").size();
      job_type_names=new String[no_types];
      job_no_nodes=new int[no_types];
      job_uids=new int[no_types];
      
      for (int i=1; i<=no_types; i++) {
        Node n = root.selectSingleNode("j["+i+"]");
        job_type_names[i-1]=new String(n.valueOf("@n"));
        job_no_nodes[i-1]=Integer.parseInt(n.valueOf("@nodes"));
        
        int no_parts = n.selectNodes("c").size();
        job_uids[i-1]= Integer.parseInt(n.valueOf("@uid"));
        StringBuffer sb = new StringBuffer();
        sb.append(String.valueOf(no_parts)+"\t");
        sb.append(n.valueOf("@lsi")+"\t");
        sb.append(n.valueOf("@t")+"\t");
        for (int j=1; j<=no_parts; j++) {
          Node c = n.selectSingleNode("c["+j+"]");
          sb.append(c.valueOf("@n")+"\t");
          sb.append(c.valueOf("@p")+"\t");
          sb.append(c.valueOf("@id")+"\t");
          sb.append(c.valueOf("@cod")+"\t");          
          sb.append(c.valueOf("@nod")+"\t");
        }
        job_type_info.add(new String(sb.toString()));
      }
      doc.clearContent();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void initGUI(GPanel p) {
    
    _jobtype = (GLabel) p.addChild(new GLabel(50,20,"Job Type: ",GLabel.LEFT_ALIGN,ga.NONE,p,gw));
    initJobTypes();
    
    l_jobtype = (GList) p.addHiddenChild(new GList(120,14,250,100,p,job_type_names,gw,GList.SINGLE_SELECTION,CHANGE_VAL));
    lh_jobtype = (GListHeader) p.addChild(new GListHeader(120,14,250,p,gw,l_jobtype,ga.NONE));
    
    _platform = (GLabel) p.addChild(new GLabel(400,20,"Platform: ",GLabel.LEFT_ALIGN,ga.NONE,p,gw));
    l_platform= (GList) p.addHiddenChild(new GList(470,14,120,60,p,platforms,gw,GList.SINGLE_SELECTION,CHOOSE_PLATFORM));
    lh_platform = (GListHeader) p.addChild(new GListHeader(470,14,120,p,gw,l_platform,ga.NONE));
       
    p.addChild(new GLine(0,95,864,95,new Color(gw.getGCS().getHedge()),p,gw));
    p.addChild(new GLabel(15,125,"Generate map file and modified synthetic population: ",GLabel.LEFT_ALIGN,ga.NONE,p,gw));    
    
    p.addChild(new GLabel(200,170,"Path to Original SP Files: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_orig_sp_data = (GTextEntry) p.addChild(new GTextEntry(220,164,400,p,gw,"",CHANGE_VAL));
    b_setInSPDir = (GButton) p.addChild(new GButton(630,166,30,22,p,gw,SET_IN_SP_DIR,"..."));
    
    p.addChild(new GLabel(200,200,"Path for Modified SP Files: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_mod_sp_data = (GTextEntry) p.addChild(new GTextEntry(220,194,400,p,gw,"",CHANGE_VAL));
    b_setModSPDir = (GButton) p.addChild(new GButton(630,196,30,22,p,gw,SET_MOD_SP_DIR,"..."));

    p.addChild(new GLabel(200,230,"Working Directory: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_mod_sp_work = (GTextEntry) p.addChild(new GTextEntry(220,224,400,p,gw,"",CHANGE_VAL));
    b_setModSPWork = (GButton) p.addChild(new GButton(630,226,30,22,p,gw,SET_MOD_WORK_DIR,"..."));

    p.addChild(new GLabel(200,260,"Launch command: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_launch_mash = (GTextEntry) p.addChild(new GTextEntry(220,254,400,p,gw,"",CHANGE_VAL));
    
    b_genmap = (GButton) p.addChild(new GButton(220,310,150,22,p,gw,GENERATE_MAP,"Generate map file"));
    b_preparemash = (GButton) p.addChild(new GButton(390,310,230,22,p,gw,PREPARE_MASH,"Prepare Scripts"));
    b_runmash = (GButton) p.addChild(new GButton(640,310,150,22,p,gw,RUN_MASH,"Launch Modifier"));    
       
    // ///////////////
    
    p.addChild(new GLine(0,360,864,360,new Color(gw.getGCS().getHedge()),p,gw));
    p.addChild(new GLabel(15,385,"Run Simulation",GLabel.LEFT_ALIGN,ga.NONE,p,gw));    
    
    
        
    _indir = (GLabel) p.addChild(new GLabel(200,420,"Path to Modified SP Files: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_inDir = (GTextEntry) p.addChild(new GTextEntry(220,414,400,p,gw,"",CHANGE_VAL));
    b_setInDir = (GButton) p.addChild(new GButton(630,416,30,22,p,gw,SET_EXE_IN_DIR,"..."));

    _exedir = (GLabel) p.addChild(new GLabel(200,450,"Working Directory: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_exeDir = (GTextEntry) p.addChild(new GTextEntry(220,444,400,p,gw,"",CHANGE_VAL));
    b_setExeDir = (GButton) p.addChild(new GButton(630,446,30,22,p,gw,SET_EXE_DIR,"..."));
    
    p.addChild(new GLabel(200,480,"Launch command: ",GLabel.RIGHT_ALIGN,ga.NONE,p,gw));
    t_launch_sim = (GTextEntry) p.addChild(new GTextEntry(220,474,400,p,gw,"",CHANGE_VAL));
    b_preparejob = (GButton) p.addChild(new GButton(320,525,150,22,p,gw,PREPARE_JOB,"Prepare Scripts"));
    b_runjob = (GButton) p.addChild(new GButton(490,525,150,22,p,gw,RUN_JOB,"Launch Sim"));    
    
    fp_dir = (GFilePanel) p.addHiddenChild(new GFilePanel(20,240,600,250,false,"",(byte)0,false,p,gw,0));
    b_okDir = (GButton) p.addHiddenChild(new GButton(450,400,80,22,p,gw,CHOOSE_DIR,"Ok"));
    b_cancelDir = (GButton) p.addHiddenChild(new GButton(540,400,80,22,p,gw,CANCEL_DIR,"Cancel"));
    
    
  }
}

/* GFilePanel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A panel with a file browser.  
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

package GKit;

import java.awt.Graphics2D;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.io.File;

import javax.swing.filechooser.FileSystemView;

public class GFilePanel extends GPanel {
  
  GList l_fileList,l_roots;
  GListHeader lh_roots;
  GButton b_previous;
  GButton b_create;
  File fp;
  String directory="";
  String ext="";
  int click_event;
  
  //public static int 
  
  public GFilePanel(int _pos_x, int _pos_y, int _width, int _height, 
                boolean _title, String _titleString, byte _titleHeight, boolean _enableDrag,GContainer _parent,GWindow _gw,int _click_event) {
    
    super(_pos_x,_pos_y,_width,_height,_title,_titleString,_titleHeight,_enableDrag,_parent,_gw);
    initGUI();
    click_event=_click_event;
  }
  
  public void setExtension(String x) {
    ext=new String(x);
  }
  
  public String getFile() {
    if (l_fileList.getSelected()==-1) return null;
    else return new String(l_fileList.getEntry(l_fileList.getSelected()));
  }
  
  public String getExtension() { return new String(ext); }
  public String getDirectory() { return new String(directory); }
  public void setDirectory(String s) { 
    fp = new File(s);
    directory=fp.getAbsolutePath().toString();
    refreshFiles();
    refreshRoots();
  }
  
  public void refreshRoots() {
  
    l_roots.clearEntries();
    l_roots.addEntry(directory);
    l_roots.addEntry("<desktop>");
    File[] roots = File.listRoots();
    for (int i=0;i<roots.length;i++)
      l_roots.addEntry(roots[i].toString());
  }
  
  public void refreshFiles() {
    fp = new File(directory);
    File[] files = fp.listFiles();
    l_fileList.clearEntries();
    if (files!=null) {
      for (int i=0; i<files.length; i++) {
        if (files[i].isDirectory()) {
          l_fileList.addEntry("["+files[i].getName()+"]");
        }
      }
      for (int i=0; i<files.length; i++) {
        if (!files[i].isDirectory()) {
          if ((ext.equals("")) || (files[i].getName().toUpperCase().endsWith("."+ext.toUpperCase()))) { 
            l_fileList.addEntry(files[i].getName());
          }
        }
      }
    }
    l_fileList.paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  public void previousDirectory() {
    if (directory.indexOf(File.separator)>=0) {
      directory=directory.substring(0,directory.lastIndexOf(File.separator));
      if (directory.startsWith("\\")) directory="\\"+directory.replace("\\\\","\\");
      else directory=directory.replace("\\\\","\\");
      if ((directory.length()==2) && (directory.charAt(1)==':')) directory+=File.separator;
      fp = new File(directory);
      l_roots.setEntry(0,directory);
      l_roots.setSelected(0);
      refreshFiles();
      lh_roots.paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    }
  }
  public void clickList(MouseEvent e, GList list) {
    if (list==l_fileList) {
      if (e.getClickCount()>1) {
        String s = l_fileList.getEntry(l_fileList.getSelected());
        if ((s.startsWith("[")) && (s.endsWith("]"))) {
          directory+=File.separator+s.substring(1,s.length()-1);
          if (directory.startsWith("\\")) directory="\\"+directory.replace("\\\\","\\");
          else directory=directory.replace("\\\\","\\");
          l_roots.setEntry(0,directory);
          l_roots.setSelected(0);
          lh_roots.paintOn(gw.bi,gw.g2d);
          refreshFiles();
          gw.requestRepaint();
        }
      }
      if (click_event>0) gw.ga.doFunction(click_event,this);
    } else if (list==l_roots) {
      if (l_roots.getEntry(l_roots.getSelected()).equals("<desktop>")) {
        FileSystemView filesys = FileSystemView.getFileSystemView();
        File[] roots = filesys.getRoots();
        directory = roots[0].getAbsolutePath();
      } else {
        directory=l_roots.getEntry(l_roots.getSelected());
      }
      if (directory.startsWith("\\")) directory="\\"+directory.replace("\\\\","\\");
      else directory=directory.replace("\\\\","\\");
      l_roots.setEntry(0,directory);
      l_roots.setSelected(0);
      lh_roots.paintOn(gw.bi,gw.g2d);
      refreshFiles();
      gw.requestRepaint();
    }
    
  }
  
  public void initGUI() {
    directory=new File("").getAbsolutePath().toString();
    if (new File(directory+File.separator+"Models").exists()) directory+=File.separator+"Models";
    fp = new File(directory);
    l_fileList = (GList) addChild(new GList(20,40,width-40,height-100,this,new String[] {},gw,GList.MULTI_SELECTION,0));
    l_roots = (GList) addHiddenChild(new GList(20,32,width-150,100,this,new String[] {},gw,GList.SINGLE_SELECTION,0));
    lh_roots = (GListHeader) addChild(new GListHeader(20,10,width-150,this,gw,l_roots,0));
    b_previous = (GButton) addChild(new GButton(width-120,10,22,22,this,gw,(byte)0,""));
    b_previous.setIcon(gw.prev_lit,gw.prev_dim);
    refreshRoots();
    refreshFiles();
  }
  
 
  
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    super.paintOn(bi,g);
  }

}

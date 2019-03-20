/* AdminUnitPage.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: choose administrative unit detail 
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
import java.awt.Rectangle;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.RandomAccessFile;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import org.dom4j.Element;
import org.dom4j.Node;

import GKit.GButton;
import GKit.GLabel;
import GKit.GList;
import GKit.GPage;
import GKit.GPanel;
import GKit.GPolyPanel;
import GKit.GPolygon;
import GKit.GTickBox;
import GKit.GWindow;

public class AdminUnitPage implements GPage {
  GButton b_nextLevel,b_previousLevel,b_addEntry, b_removeEntry,b_saveData;
  GList l_selection,l_entries;
  GTickBox tb_map_parent,tb_map_allparents;
  GLabel _unit,_showparent,_showallparents;
  GWindow gw;
  GPolyPanel pp_map;
  GSim ga;
  PolicyLinkage pl;
  ParamGrid pg;
  OutputSelect os;
  
  String[] menuStrings= new String[5];  // Store the strings selected - eg, USA, Alabama, Sub-Alabama
  int[] menuIds = new int[5];           // Store the IDS for USA, Alabama, Sub-Alabama
  int[] menuIndexes = new int[5];       // Store the indexes in the lists - 231, 0, x
  
  ArrayList<String> unit_info = new ArrayList<String>();
  ArrayList<Integer> unit_ids = new ArrayList<Integer>();
    
  byte currentLevel=0;
  final Color[] au_borders = new Color[] {new Color(15,91,11),new Color(104,87,10),new Color(165,68,6), new Color(104,10,10), new Color(79,11,67), new Color(6,13,152)};
  final Color[] au_fills = new Color[] {new Color(29,255,45), new Color(252,255,29), new Color(255,162,29), new Color(255,73,17), new Color(255,17,241), new Color(77,106,251)};
  
  ArrayList<String> countries;
  ArrayList<String> iso_codes;
  ArrayList<Integer> detail_level;
  ArrayList<Integer> grump_codes;
  
  
  public AdminUnitPage(GWindow _gw, GSim _ga,PolicyLinkage _pa,ParamGrid _pg,OutputSelect _os) {
    gw=_gw;
    ga=_ga;
    pl=_pa;
    pg=_pg;
    os=_os;
    initCountries();
  }
  public final int DRAW_POLY = 100;
  public final int NEXT_LEVEL = 101;
  public final int PREVIOUS_LEVEL = 102;
  public final int ADD_ENTRY = 103;
  public final int REMOVE_ENTRY = 104;
  public final int SELECT_ENTRY = 105;
  public final int POLY_PARENT = 107;
  public final int POLY_ALLPARENTS = 108;
  
  public int getMinEvent() { return 100; }
  public int getMaxEvent() { return 199; }
  
  public void doFunction(int func,Object component) {
    if (func==DRAW_POLY) clickList();
    else if (func==NEXT_LEVEL) clickNextLevel();
    else if (func==PREVIOUS_LEVEL) clickPrevLevel();
    else if (func==ADD_ENTRY) addEntry();
    else if (func==REMOVE_ENTRY) removeEntry();
    else if (func==SELECT_ENTRY) clickEntry();
    else if (func==POLY_PARENT) polyParents(false);
    else if (func==POLY_ALLPARENTS) polyParents(true);  
  }
  
  public void createDefaultXML(Element root) {
    addGlobalSetting();
    saveXML(root);
    // Admin units can start empty by default
  }

  public String encode(String t) {
    String result = "";
    try {
      result=URLEncoder.encode(t,"UTF-8");
    } catch (Exception e) {}
    return result;
  }
  
  public String decode(String t) {
    String result = "";
    try {
      result=URLDecoder.decode(t,"UTF-8");
    } catch (Exception e) {}
    return result;
  }

  public void saveXML(Element root) {
    Node units_node=root.selectSingleNode("adminunits");
    if (units_node!=null) {
      root.remove(units_node);
    }
    Element units_el = root.addElement("adminunits");
    for (int i=0; i<l_entries.countEntries(); i++) {
      Element new_au = units_el.addElement("unit");
      new_au.addAttribute("no",String.valueOf(i));
      String[] parts = unit_info.get(i).split("\t");
      byte level=Byte.parseByte(parts[0]);
      new_au.addAttribute("level",String.valueOf(level));
      String s = new String("");
      for (int j=0; j<=level; j++) {
        s+=String.valueOf(parts[j+1]);
        if (j<level) s+=",";
      }
      new_au.addAttribute("path",s);
      String text = encode(l_entries.getEntry(i));
      new_au.addAttribute("text",text);
    }
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(l_entries.countEntries()));    // no. admin units : int 
      for (int i=0; i<l_entries.countEntries(); i++) {
        String[] parts = unit_info.get(i).split("\t");                 // For each unit
        int level = Integer.parseInt(parts[0]);
        dos.writeInt(Integer.reverseBytes(level));
        
        if (level==-1) dos.writeInt(Integer.reverseBytes(255));
        else {
          int grump = grump_codes.get(Integer.parseInt(parts[1]));
          dos.writeInt(Integer.reverseBytes(grump));
        }
        
        if (level>0) {                                                 // Only units of level >=1 have a parent to look for.
          String path_to_match = String.valueOf(level-1)+"\t";
          for (int k=0; k<level; k++) {
            path_to_match+=parts[k+1];
            if (k<level-1) path_to_match+="\t";      // Build a copy of the original path, truncate last "TAB ENTRY" - so matches parent.
          }
          boolean found=false;
          for (int m=0; m<unit_info.size(); m++) {
            if (unit_info.get(m).toString().equals(path_to_match)) {
              found=true;
              dos.writeInt(Integer.reverseBytes(m));
              m=unit_info.size();
            }
          }
          if (!found) System.out.println("ERROR - PARENT OF "+parts[0]+" NOT FOUND - Looking For "+path_to_match);
        }
        //   Level : byte
        //for (int j=0; j<=level; j++) {                                      //   For each level
//          dos.writeInt(Integer.reverseBytes(Integer.parseInt(parts[j+1]))); //  the id number of the admin unit for that level.
        //}
        
        // Can't see why we need the above bit
      
        pg.saveBinary(dos,i);
        pl.saveBinary(dos,i);
        os.saveBinary(dos,i);
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
    
    
  public void addGlobalSetting() {
    l_entries.addEntry("** Global Settings **");
    pl.addAdminUnit("** Global Settings **");
    pg.addAdminUnit("** Global Settings **");
    os.addAdminUnit("** Global Settings **");
    unit_info.add("-1");
  }
  
  public void loadXML(Element root) {
    Node units_node=root.selectSingleNode("adminunits");
    if (units_node!=null) {
      int no_units = units_node.selectNodes("unit").size();
      for (int i=0; i<no_units; i++) {
        String entry="";
        Node unit = units_node.selectSingleNode("unit["+(i+1)+"]");
        byte level = Byte.parseByte(unit.valueOf("@level"));
        entry=String.valueOf(level)+"\t";
        String[] path = unit.valueOf("@path").split(",");
        for (int j=0; j<path.length; j++) {
          entry+=path[j];
          if (j<path.length-1) entry+="\t";
        }
        String text = new String(decode(unit.valueOf("@text")));
        l_entries.addEntry(text);
        pl.addAdminUnit(i,text);
        pg.addAdminUnit(i,text);
        os.addAdminUnit(i,text);
        unit_info.add(entry);
      }
    }
  }
  
  public void clear() {
    for (int i=l_entries.countEntries()-1; i>=0; i--) {
      l_entries.removeEntry(i);
      unit_info.remove(i);
      pl.removeAdminUnit(i);
      pg.removeAdminUnit(i);
      os.removeAdminUnit(i);
    }
  }
  
  
  public void initGUI(GPanel a_units) {
    l_selection = (GList) a_units.addChild(new GList(20,34,400,300,a_units,null,gw,GList.MULTI_SELECTION,DRAW_POLY));
    pp_map = (GPolyPanel) a_units.addChild(new GPolyPanel(440,34,400,300,false,"",(byte)0,false,a_units,gw,GPolyPanel.SCALE_PROP));
    l_entries = (GList) a_units.addChild(new GList(20,394,820,200,a_units,null,gw,GList.MULTI_SELECTION,SELECT_ENTRY));
    b_previousLevel = (GButton) a_units.addChild(new GButton(20,354,120,22,a_units,gw,PREVIOUS_LEVEL,"Previous Level"));
    b_nextLevel = (GButton) a_units.addChild(new GButton(300,354,120,22,a_units,gw,NEXT_LEVEL,"Next Level"));
    b_addEntry = (GButton) a_units.addChild(new GButton(160,354,120,22,a_units,gw,ADD_ENTRY,"Add Unit(s)"));
    b_removeEntry = (GButton) a_units.addChild(new GButton(20,614,120,22,a_units,gw,REMOVE_ENTRY,"Remove Unit(s)"));
//    saveData = (GButton) a_units.addChild(new GButton(720,614,120,22,a_units,gw,ga.SAVE_DATA,"Write Files"));
    _unit = (GLabel) a_units.addChild(new GLabel(23,9,"Admin unit",GLabel.LEFT_ALIGN,ga.NONE,a_units,gw));
    tb_map_parent = (GTickBox) a_units.addChild(new GTickBox(660,354,a_units,gw,POLY_PARENT));
    tb_map_allparents = (GTickBox) a_units.addChild(new GTickBox(818,354,a_units,gw,POLY_ALLPARENTS));    
    _showallparents = (GLabel) a_units.addChild(new GLabel(808,360,"Show all parents",GLabel.RIGHT_ALIGN,ga.NONE,a_units,gw));
    _showparent = (GLabel) a_units.addChild(new GLabel(650,360,"Show parent unit",GLabel.RIGHT_ALIGN,ga.NONE,a_units,gw));
    tb_map_parent.setSelected(false);
    tb_map_allparents.setSelected(true);
    b_nextLevel.setEnabled(false);
    b_previousLevel.setEnabled(false);
    b_addEntry.setEnabled(false);
    b_removeEntry.setEnabled(false);
    currentLevel=0;
    initialiseRegions(l_selection);
  }
  
public String dequote(String s) { return s.substring(1,s.length()-1); }
  
  public int getIndex(String country) {
    int i=0;
    while (!countries.get(i).equals(country)) i++;
    return i;
  }

  public void createEmptyAUMap(String file) {
    byte[] minus_one = new byte[5120000];
    for (int i=0; i<5120000; i++) minus_one[i]=-1; // Integer -1 is represented as bytes -1,-1,-1,-1 - either LSB or MSB first!
    try {
      DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(new File(file))));
      for (int i=0; i<729; i++) dos.write(minus_one);
      dos.flush();
      dos.close();
      
    } catch (Exception e) { e.printStackTrace(); }
    minus_one=null;
  }
  
  public static final byte WRITE = 1;
  public static final byte REMEMBER = 2;  
  public static final byte RESTORE = 3;
  
  public void polygonToAUMap(GPolygon p, String file, int code,ArrayList<Integer> memory, byte function) {
    int mem_counter=0;
    try {
      RandomAccessFile raf = new RandomAccessFile(file,"rw");
      final Rectangle bounds = p.getBounds();
      final int booleanWidth = (int) Math.ceil((1+bounds.width)/8.0);
      final char[][] booleanGrid = new char[booleanWidth][bounds.height+1];
      int px,py;

      ArrayList<Integer> pixels;    
      for (int i=0; i<p.npoints; i++) {    // For each line segment in polygon
        if (i<p.npoints-1) pixels = GPolyPanel.getPixels(p.xpoints[i],p.ypoints[i],p.xpoints[i+1],p.ypoints[i+1]);
        else pixels = GPolyPanel.getPixels(p.xpoints[i],p.ypoints[i],p.xpoints[0],p.ypoints[0]);
        for (int j=0; j<pixels.size(); j+=2) { // For each pixel in line segment
          px = ((Integer) pixels.get(j)).intValue();
          py = ((Integer) pixels.get(j+1)).intValue();      

          final int px_b = px-bounds.x;
          final int py_b = py-bounds.y;
          final int xdiv8 = px_b/8;
          final int xmod8 = px_b%8;

          if (((booleanGrid[xdiv8][py_b]) & (1<<xmod8))==0) {
            booleanGrid[xdiv8][py_b] = (char) ((booleanGrid[xdiv8][py_b]) | (1<< (xmod8))); 
            long ls_x = px+21600;
            long ls_y = 21600-(py+10800);
            raf.seek(((ls_y*43200)+ls_x)*4);
            if (function==WRITE) {
              raf.writeInt(Integer.reverseBytes(code));
            } else if (function==REMEMBER) {
              memory.add(raf.readInt());
            } else if (function==RESTORE) {
              raf.writeInt(memory.get(mem_counter++));
            }
          }
        }
        pixels.clear();
      }

      if ((bounds.width>0) && (bounds.height>0)) {
        int start_i,end_i,i,j;
        for (j=0; j<=bounds.height; j++) {
          final int j_actual = j+bounds.y; 
          start_i=0;
          while ((start_i<bounds.width) && (((booleanGrid[start_i/8][j]) & (1<< (start_i%8)))==0)) start_i++; // Space before border
          while ((start_i<bounds.width) && (((booleanGrid[start_i/8][j]) & (1<< (start_i%8)))>0)) start_i++; // Actual border        
          end_i=bounds.width;
          while ((end_i>=start_i) && (((booleanGrid[end_i/8][j]) & (1<< (end_i%8)))==0)) end_i--; // Space to right of border
          while ((end_i>=start_i) && (((booleanGrid[end_i/8][j]) & (1<< (end_i%8)))>0)) end_i--; // Actual border
          boolean unknown=true;
          boolean inside=false;
          for (i=start_i; i<=end_i; i++) {
            final int i_actual = i+bounds.x;
            if (((booleanGrid[i/8][j]) & (1<< (i%8)))==0) {
              if (unknown) {
                inside=p.contains(i_actual,j_actual);
                unknown=false;
              }
              if (inside) {
                long ls_x = i_actual+21600;
                long ls_y = 21600-(j_actual+10800);
                raf.seek(((ls_y*43200)+ls_x)*4);
                if (function==WRITE) {
                  raf.writeInt(Integer.reverseBytes(code));
                } else if (function==REMEMBER) {
                  memory.add(raf.readInt());
                } else if (function==RESTORE) {
                  raf.writeInt(memory.get(mem_counter++));
                }
              }
            } else unknown=true;
          }
        }
      }
      raf.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void initCountries() {
    countries = new ArrayList<String>();
    iso_codes = new ArrayList<String>();
    detail_level = new ArrayList<Integer>();
    try {
      BufferedReader br = new BufferedReader(new FileReader(ga.gadmPath+"iso-3166-1.txt"));
      BufferedReader br2 = new BufferedReader(new FileReader(ga.gadmPath+"levels.txt"));      
      String s = br.readLine();
      String s2 = br2.readLine();
      while (s!=null) {
        String[] parts = s.split(",");
        String[] parts2 = s2.split("\t");
        countries.add(parts[0]);
        iso_codes.add(parts[2]);
        if (!parts[2].equals(parts2[0])) System.out.println("Sync error!");
        detail_level.add(Integer.parseInt(parts2[1]));
        s = br.readLine();
        s2 = br2.readLine();
        
      }
      br.close();
      br2.close();
      ArrayList<String> temp_grump = new ArrayList<String>();
      grump_codes = new ArrayList<Integer>();
      br = new BufferedReader(new FileReader(ga.gadmPath+"iso-grump.txt"));
      br.readLine();
      s = br.readLine();
      int i=0;
      while (s!=null) {
        String[] parts = s.split("\t");
        temp_grump.add(parts[2]);
        i++;
        s = br.readLine();
      }
      for (int j=0; j<iso_codes.size(); j++) {
        boolean found=false;
        for (i=0; i<temp_grump.size(); i++) {
          if (iso_codes.get(j).equals(temp_grump.get(i))) {
            grump_codes.add(i);
            i=temp_grump.size();
            found=true;
          }
        }
        if (!found) System.out.println("Error finding grump code for "+iso_codes.get(j)+" = "+countries.get(j));
      }
      br.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  
  public void polyParents(boolean all_clicked) {
    if (all_clicked) {
      boolean all_sel = tb_map_allparents.isSelected();
      if (all_sel) tb_map_allparents.setSelected(false);
      else {
        tb_map_allparents.setSelected(true);
        tb_map_parent.setSelected(false);
      }
    } else {
      boolean one_sel = tb_map_parent.isSelected();
      if (one_sel) tb_map_parent.setSelected(false);
      else {
        tb_map_allparents.setSelected(false);
        tb_map_parent.setSelected(true);
      }
    }
    pp_map.clearPolys();
    int[] theSelection = l_selection.getSelectedIndexes();
    if (currentLevel==0) runPolyThreadCountrys(theSelection,pp_map,true);
    else runPolyThreadSubCountry(iso_codes.get(menuIndexes[0]),theSelection,l_selection, pp_map,menuStrings,menuIds,currentLevel);
    pp_map.paintOn(gw.bi(),gw.g2d());
    if (all_clicked) tb_map_parent.paintOn(gw.bi(),gw.g2d());
    if (!all_clicked) tb_map_allparents.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
    
  }
  
  public void clickEntry() {
    int[] indexes = l_entries.getSelectedIndexes();
    boolean found = false;
    for (int i=0; i<indexes.length; i++) {
      if (okToRemove(indexes[i])) {
        found=true;
        i=indexes.length;
      }
    }
    b_removeEntry.setEnabled(found);
    
    if (l_entries.countSelection()==1) {
      l_selection.clearSelection();
      String[] stringParts = l_entries.getEntry(l_entries.getSelected()).split(",");
      String[] s_intParts = unit_info.get(l_entries.getSelected()).split("\t");
      int[] i_intParts = new int[s_intParts.length-1];
      byte _currentLevel=Byte.parseByte(s_intParts[0]);
      for (int i=0; i<s_intParts.length-1; i++) i_intParts[i]=Integer.parseInt(s_intParts[i+1]);
      if (i_intParts.length>0) {
        stringParts[0]=iso_codes.get(i_intParts[0]); 
        for (int i=1; i<stringParts.length; i++) stringParts[i]=stringParts[i].trim();
        if (_currentLevel==0) runPolyThreadCountrys(new int[] {},pp_map,true);
        else runPolyThreadSubCountry(stringParts[0],new int[] {},l_selection, pp_map,stringParts,i_intParts,(byte)(_currentLevel+1));
      } 
    }
    pp_map.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public boolean okToRemove(int index) {
    if (index==0) return false;
    else {
      String path = unit_info.get(index);          // Get the level, id... string
      path = path.substring(path.indexOf("\t")+1); // Remove level off the front.
      boolean found=false;
      for (int i=0; i<unit_info.size(); i++) {
        if (i!=index) {
          String path_compare = unit_info.get(i);
          path_compare = path_compare.substring(path_compare.indexOf("\t")+1);
          if ((path_compare+"\t").startsWith(path+"\t")) {
            found=true;
            i=unit_info.size();
          }
        }
      }
      return (!found);
    }
  }
  
  public void removeEntry() {
    int[] indexes = l_entries.getSelectedIndexes();
    for (int i=indexes.length-1; i>=0; i--) {
      if (indexes[i]>0) {
        if (okToRemove(indexes[i])) {
          ga.setUnSaved(true);
          l_entries.removeEntry(indexes[i]);
          unit_info.remove(indexes[i]);
          pl.removeAdminUnit(indexes[i]);
          pg.removeAdminUnit(indexes[i]);
          os.removeAdminUnit(indexes[i]);
        }
      }
    }
    l_entries.clearSelection();
    b_removeEntry.setEnabled(false);
    l_entries.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
    
  }
  
  public void addEntry() {
    ga.setUnSaved(true);
    int[] indexes = l_selection.getSelectedIndexes();
    String entry="";
    String data_entry=String.valueOf(currentLevel)+"\t";   // Data Entry is simply   level number.  Suppose we add UK, England, Bucks (level=2)
    if (currentLevel>0) {
      data_entry+=String.valueOf(menuIds[0])+"\t";         // Data Entry is now      2 TAB 231 TAB
      entry = countries.get(menuIds[0]);                   // String entry is now    United Kingdom
      for (int i=1; i<currentLevel; i++) {                 //   Loop until the menu before this one...   
        entry+=", "+menuStrings[i];                        // String entry is now    United Kingdom, England   
        data_entry+=String.valueOf(menuIds[i]+"\t");       // Data Entry is now      2 TAB 231 TAB 1030 TAB
      }
      entry+=", ";
    }
    
    for (int i=0; i<indexes.length; i++) {                          // Now process the list currently on screen. For each selected entry:-
      String one_entry = entry+l_selection.getEntry(indexes[i]);    // one_entry is    United Kingdom, England, Buckinghamshire
      
      int j=0;
      int total = l_entries.countEntries();
      while ((j<total) && (l_entries.getEntry(j).compareTo(one_entry)<0)) j++;    // Find place to insert alphabetically
      if (j<total) {
        String compare = l_entries.getEntry(j);
        if (!compare.equals(one_entry)) {
          l_entries.addEntry(j,one_entry);
          l_entries.getSlider().setValue(Math.min(l_entries.getSlider().getMax(),j));
          unit_info.add(j,data_entry+unit_ids.get(indexes[i]));                        // Append list index to data entry=> 2 TAB 231 TAB 1030 TAB 13947
          pl.addAdminUnit(j,one_entry);
          pg.addAdminUnit(j,one_entry);
          os.addAdminUnit(j,one_entry);
        }
        
      } else {
        l_entries.addEntry(one_entry);                                             // Add at end of list
        l_entries.getSlider().setValue(l_entries.getSlider().getMax());
        unit_info.add(data_entry+unit_ids.get(indexes[i]));                        // Final data entry = 2 TAB 231 TAB 1030 TAB 13947
        pl.addAdminUnit(one_entry);
        pg.addAdminUnit(one_entry);
        os.addAdminUnit(one_entry);
      }
    }
   
    // Now check that all of the parents are also added.
    // So we want units with a path taken out of menuIDs...
    
    for (int j=currentLevel-1; j>=0; j--) {
      String path=String.valueOf(j);
      entry= countries.get(menuIds[0]);
      for (int i=0; i<=j; i++) {
        path+="\t"+menuIds[i];
        if (i>0) entry+=", "+menuStrings[i];
      }
      boolean found=false;
      for (int i=0; i<unit_info.size(); i++) {
        if (unit_info.get(i).toString().equals(path)) {
          found=true;
          i=unit_info.size();
        }
      }
      if (!found) {    // Not already added - need to add the parent
        int k=0;
        int total = l_entries.countEntries();
        while ((k<total) && (l_entries.getEntry(k).compareTo(entry)<=0)) k++;    // Find place to insert alphabetically
        if (k<total) {
          String compare = l_entries.getEntry(k);
          if (!compare.equals(entry)) {
            l_entries.addEntry(k,entry);
            l_entries.getSlider().setValue(Math.min(l_entries.getSlider().getMax(),k));
            unit_info.add(k,path);                        // Append list index to data entry=> 2 TAB 231 TAB 1030 TAB 13947
            pl.addAdminUnit(k,entry);
            pg.addAdminUnit(k,entry);
            os.addAdminUnit(k,entry);
          }
          
        } else {
          l_entries.addEntry(entry);                                             // Add at end of list
          l_entries.getSlider().setValue(l_entries.getSlider().getMax());
          unit_info.add(path);                        // Final data entry = 2 TAB 231 TAB 1030 TAB 13947
          pl.addAdminUnit(entry);
          pg.addAdminUnit(entry);
          os.addAdminUnit(entry);
        }
      }
    }
    
    l_entries.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void clickList() {
    l_entries.clearSelection();
    b_addEntry.setEnabled(l_selection.countSelection()>0);
    b_previousLevel.setEnabled(currentLevel>0);
    boolean nextOk=false;
    if (l_selection.countSelection()==1) {
      if (currentLevel==0) {
        if (detail_level.get(l_selection.getSelected())>currentLevel) nextOk=true;
      } else {
        if (detail_level.get(menuIndexes[0])>currentLevel) nextOk=true;
      }
    }
    b_nextLevel.setEnabled(nextOk);
    pp_map.clearPolys();
    int[] theSelection = l_selection.getSelectedIndexes();
    if (currentLevel==0) runPolyThreadCountrys(theSelection,pp_map,true);
    else runPolyThreadSubCountry(iso_codes.get(menuIndexes[0]),theSelection,l_selection, pp_map,menuStrings,menuIds,currentLevel);
    l_entries.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
    
  }
  
  public void clickNextLevel() {
    int i = l_selection.getSelected();
    if (currentLevel>0) {
      menuStrings[currentLevel]=new String(l_selection.getEntry(i));
      menuIds[currentLevel]=unit_ids.get(i).intValue();
      menuIndexes[currentLevel]=i;
    }
    else {
      menuStrings[currentLevel]=iso_codes.get(i);
      menuIds[currentLevel]=i;
      menuIndexes[currentLevel]=i;
    }
    updateLabel();
    currentLevel++;
    initialiseRegions(l_selection);
    b_nextLevel.setEnabled(false);
    b_previousLevel.setEnabled(true);
    b_addEntry.setEnabled(false);
    l_selection.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void updateLabel() {
    String s="";
    for (int j=0; j<iso_codes.size(); j++) {
      if (iso_codes.get(j).equals(menuStrings[0])) {
        s=countries.get(j);
        if (currentLevel>=1) s+=", ";
        j=iso_codes.size();
      }
    }
    for (int j=1; j<=currentLevel; j++) {
      s+=menuStrings[j];
      if (j<currentLevel) s=s+", ";
    }
    _unit.unPaint(gw.bi(),gw.g2d());
    _unit.setText(s);
    _unit.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  public void clickPrevLevel() {
    currentLevel--;
    initialiseRegions(l_selection);
    updateLabel();
    // Find last selection:-
    l_selection.setSelected(menuIndexes[currentLevel]);
    l_selection.getSlider().setValue(menuIndexes[currentLevel]);
    
    l_entries.clearSelection();
    l_selection.paintOn(gw.bi(),gw.g2d());
    
    pp_map.clearPolys();
    int[] theSelection = l_selection.getSelectedIndexes();
    if (currentLevel==0) runPolyThreadCountrys(theSelection,pp_map,true);
    else runPolyThreadSubCountry(iso_codes.get(menuIndexes[0]),theSelection,l_selection, pp_map,menuStrings,menuIds,currentLevel);
    pp_map.paintOn(gw.bi(),gw.g2d());
    
    clickList();
    gw.requestRepaint();
  }
  
  public void initialiseRegions(GList _result) {
    _result.clearEntries();
    unit_ids.clear();
    if (currentLevel==0) {
      for (int i=0; i<countries.size(); i++) {
        _result.addEntry(countries.get(i));
        unit_ids.add(i);
      }
    } else {
      ga.db.getNames(currentLevel,menuIds,_result,unit_ids,iso_codes.get(menuIds[0]));
    }
  }

  public void readPolys(DataInputStream dis, boolean useThis, ArrayList<GPolygon> list,byte level) {
    try {
      if (useThis)  {
        int polys = dis.readInt();
        for (int j=0; j<polys; j++) {
          GPolygon gpoly = new GPolygon(au_fills[level],au_borders[level]);
          gpoly.setGroupNo(j);
          gpoly.setNoInGroup(polys);
          
          
          int points = dis.readInt();
          int p_no=0;
          
          for (int k=0; k<points; k++) {
            int new_x=dis.readInt();
            int new_y=dis.readInt();
            if (p_no==0) {
              gpoly.addPoint(new_x,new_y);
              p_no++;
            } else {
              boolean ok=true;
              if ((new_x==gpoly.xpoints[p_no-1]) && (new_y==gpoly.ypoints[p_no-1])) ok=false;
              if ((ok) && (p_no>=2)) {
                if ((new_x==gpoly.xpoints[p_no-2]) && (new_y==gpoly.ypoints[p_no-2])) {
                  gpoly.xpoints[p_no-1]=gpoly.xpoints[p_no-2];
                  gpoly.ypoints[p_no-1]=gpoly.ypoints[p_no-2];
                  ok=false;
                }
              }
              if (ok) {
                gpoly.addPoint(new_x,new_y);
                p_no++;
              }
            }
            gpoly.npoints=p_no;
          }
          gpoly.isAntiClockwise();
          list.add(gpoly);
        }
      } else {
        int polys = dis.readInt();
        for (int j=0; j<polys; j++) {
          int points = dis.readInt();
          for (int k=0; k<points*2; k++) dis.readInt();
        }
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void loadPolygons(String file, String[] matches, ArrayList<GPolygon> list, byte level,int[] parent_ids) {
    int[] lev = new int[6];
    int done=0;
    int i=0, placemarks=0;
    try {
      ZipFile zf = new ZipFile(ga.gadmPath+"bins.zip");
      ZipEntry ze = zf.getEntry(file);
      DataInputStream dis = new DataInputStream(new BufferedInputStream(zf.getInputStream(ze)));
      readString(dis); // Country
      dis.readByte();  // Level
      placemarks = dis.readInt();
      for (i=0; i<placemarks; i++) {
        String n = readString(dis);
        for (int j=1; j<=level; j++) {
           lev[j]=dis.readInt();
        }
        boolean useThis=false;
        for (int j=0; j<matches.length; j++) {
          if (compUTF(n,matches[j])) {
            useThis=true;
            for (int k=1; k<level; k++) {
              if (parent_ids[k]!=lev[k]) {
                useThis=false;
                k=parent_ids.length;
              }
            }
            j=matches.length;
          }
        }
        readPolys(dis,useThis,list,level);
        if (useThis) if (++done==matches.length) i=placemarks;
      }
//      fixExclusions(list);
      dis.close();
      zf.close();
    } catch (Exception e) { 
      e.printStackTrace(); 
      System.out.println("Error on i="+i+", placemarks="+placemarks);
    }
    
  }
  
  public String readString(DataInputStream dis) throws Exception {
    int len = dis.readInt();
    StringBuffer sb = new StringBuffer();
    for (int i=0; i<len; i++) sb.append(dis.readChar());
    return new String(sb.toString());
  }
  
  public boolean compUTF(String n1, String n2) {
    char[] c1 = n1.toCharArray();
    char[] c2 = n2.toCharArray();
    for (int i=0; i<c1.length; i++)
      if ((int) c1[i]>65000) c1[i]=(char) (256-(65536-c1[i]));
    for (int i=0; i<c2.length; i++)
      if ((int) c2[i]>65000) c2[i]=(char) (256-(65536-c2[i]));
    return (new String(c1).equals(new String(c2)));
  }
  
  public void loadPolygons(int[] indexes, ArrayList<GPolygon> list, boolean plot_all) {   
    try {
      ZipFile zf = new ZipFile(ga.gadmPath+"bins.zip");
      for (int c=0; c<indexes.length; c++) {
        String file = iso_codes.get(indexes[c])+"_adm0.bin";
        String name = countries.get(indexes[c]);
        ZipEntry ze = zf.getEntry(file);
        DataInputStream dis = new DataInputStream(new BufferedInputStream(zf.getInputStream(ze)));
        readString(dis); // Country
        dis.readByte();  // Level
        int placemarks = dis.readInt();
        for (int i=0; i<placemarks; i++) {
          String n = readString(dis);
          if ((plot_all) || (compUTF(n,name)))  {
            readPolys(dis,true,list,(byte)0);
            i=placemarks;
          } else {
            readPolys(dis,false,list,(byte)0);
          }
        } 
        dis.close();
      }
      zf.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void createMapFile(String out) { 
    try {
      createEmptyAUMap(out);
      int done=0;

      ArrayList<GPolygon> one_unit = new ArrayList<GPolygon>();
      for (byte level=0; level<=5; level++) {
        for (int i=0; i<unit_info.size(); i++) {
          String[] parts = unit_info.get(i).split("\t");
          byte _level = Byte.parseByte(parts[0]);
          if (_level==level) {
            if (level==0) {
              loadPolygons(new int[] {Integer.parseInt(parts[1])},one_unit,true);
            } else {
              String iso = iso_codes.get(Integer.parseInt(parts[1]));
              int[] other_ids = new int[level+1];
              for (int j=0; j<=level; j++) other_ids[j]=Integer.parseInt(parts[1+j]);
              String[] names = ga.db.namesForIDs(iso,other_ids);
              loadPolygons(iso+"_adm"+String.valueOf(level)+".bin",new String[] {names[level]},one_unit,level,other_ids);
            }
            done++;
            
            // Check for any anti-clockwise units, and copy data...
            ArrayList<ArrayList<Integer>> anti_memory = new ArrayList<ArrayList<Integer>>();
            for (int j=0; j<one_unit.size(); j++) {
              GPolygon one_poly = one_unit.get(j);
              if (one_poly.isAntiClockwise()) {
                ArrayList<Integer> remember = new ArrayList<Integer>();
                polygonToAUMap(one_poly,out,i,remember,REMEMBER);
                anti_memory.add(remember);
              }
            }
            if (anti_memory.size()>0) System.out.println("Remembered "+anti_memory.size()+" exclusions");
            
            for (int j=0; j<one_unit.size(); j++) {
              GPolygon one_poly = one_unit.get(j);
              if (!one_poly.isAntiClockwise()) polygonToAUMap(one_poly,out,i,null,WRITE);
            }
            int mem_counter=0;
            for (int j=0; j<one_unit.size(); j++) {
              GPolygon one_poly = one_unit.get(j);
              if (one_poly.isAntiClockwise()) {
                ArrayList<Integer> remember = anti_memory.get(mem_counter);
                polygonToAUMap(one_poly,out,i,remember,RESTORE);
                remember.clear();
                mem_counter++;
              }
            }
            anti_memory.clear();
            one_unit.clear();
            System.out.println(done+"/"+unit_info.size());
          }
        }
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void runPolyThreadCountrys(int[] _selection, GPolyPanel gp ,boolean plot_all) {
    LoadCountrysThread lct = new LoadCountrysThread(_selection,gp,plot_all);
    lct.startNonThreaded();
  }
  
  public void runPolyThreadSubCountry(String file, int[] indexes, GList gl, GPolyPanel gp,String[] ms, int[] mi,byte le) {
    LoadSubCountryThread lcst = new LoadSubCountryThread(file,indexes,gl,gp,ms,mi,le);
    lcst.startNonThreaded();
  }
  
  class LoadCountrysThread /*extends Thread */{
    
    GPolyPanel gp;
    boolean plot_all;
    int[] indexes;
    
    
    public LoadCountrysThread(int[] _indexes, GPolyPanel _gp, boolean _plot_all) {
      gp=_gp;
      plot_all=_plot_all;
      indexes = new int[_indexes.length];
      for (int i=0; i<_indexes.length; i++) indexes[i]=_indexes[i];
     
    }
      
    public void startNonThreaded() {
      if (gp.lock()) {
        pp_map.paintOn(gw.bi(),gw.g2d());
        gw.repaint();
        tb_map_allparents.setEnabled(false);
        tb_map_parent.setEnabled(false);
        loadPolygons(indexes,gp.polys(),plot_all);
        gp.updatePolyImage();
        tb_map_allparents.setEnabled(true);
        tb_map_parent.setEnabled(true);
        gp.unlock();
        pp_map.paintOn(gw.bi(),gw.g2d());
        gw.repaint();
      }
    }
  }
  
  class LoadSubCountryThread /* extends Thread */  {
    String file;
    int[] indexes;
    GList gl;
    GPolyPanel gp;
    String[] menu_s;
    int[] parent_ids;
    byte level;
    
    public LoadSubCountryThread(String _file, int[] _indexes, GList _gl, GPolyPanel _gp,String[] ms, int[] pids,byte _level) {
      file=new String(_file);
      indexes = new int[_indexes.length];
      for (int i=0; i<_indexes.length; i++) indexes[i]=_indexes[i];
      menu_s = new String[ms.length];
      for (int i=0; i<menu_s.length; i++) if (ms[i]!=null) menu_s[i]=new String(ms[i]);
      parent_ids = new int[pids.length];
      for (int i=0; i<parent_ids.length; i++) parent_ids[i]=pids[i];
      gl=_gl;
      gp=_gp;
      level=_level;
    }
    
    public void startNonThreaded() {
      if (gp.lock()) {
        tb_map_allparents.setEnabled(false);
        tb_map_parent.setEnabled(false);
        pp_map.paintOn(gw.bi(),gw.g2d());
        gw.repaint(); 
        String[] matches = new String[indexes.length];
        for (int i=0; i<indexes.length; i++) matches[i]=gl.getEntry(indexes[i]);
        if (tb_map_allparents.isSelected()) {
          for (byte j=0; j<level; j++) {
            if (j==0) loadPolygons(file+"_adm"+j+".bin",new String[] {countries.get(parent_ids[0])},gp.polys(),j,parent_ids);
            else loadPolygons(file+"_adm"+j+".bin",new String[] {menu_s[j]},gp.polys(),j,parent_ids);
          }
        } else if (tb_map_parent.isSelected()) {
            if (level>1) loadPolygons(file+"_adm"+(level-1)+".bin",new String[] {menu_s[currentLevel-1]},gp.polys(),(byte)(level-1),parent_ids);
            else if (level==1) loadPolygons(file+"_adm"+(level-1)+".bin",new String[] {countries.get(parent_ids[0])},gp.polys(),(byte)(level-1),parent_ids);
          
        } 
        if (matches.length>0) loadPolygons(file+"_adm"+level+".bin",matches,gp.polys(),level,parent_ids);
        gp.updatePolyImage();
        tb_map_allparents.setEnabled(true);
        tb_map_parent.setEnabled(true);
        gp.unlock();
        pp_map.paintOn(gw.bi(),gw.g2d());
        gw.repaint();
      }
    }
  }
}

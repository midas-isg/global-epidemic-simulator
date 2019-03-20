/* Seeding.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: page for defining locations/numbers of initial seeds 
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
import java.text.DecimalFormat;
import java.util.ArrayList;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

import GKit.GButton;
import GKit.GLabel;
import GKit.GLine;
import GKit.GList;
import GKit.GPage;
import GKit.GPanel;
import GKit.GSlider;
import GKit.GTextCell;
import GKit.GTextEntry;
import GKit.GWindow;


public class SeedingPage implements GPage {
  GWindow gw;
  GSim ga;
  GLabel _long,_lat,_time,_no;
  GTextEntry t_long,t_lat,t_time,t_no;
  GList l_cities;
  GButton b_add;
  GSlider gs_vert;
  
  ArrayList<GTextEntry> tt_long,tt_lat,tt_time,tt_no;
  ArrayList<GButton> bb_remove;
  
  ArrayList<String> longs,lats,times;
  ArrayList<Integer> nos;
  ArrayList<Integer> c_lon,c_lat;
  
  static int max_seeds_screen = 13;
  int no_seeds,first_seed;
  int t_starty=230;
  int t_spacingy=30;
  
  
  // Extra date stuff - keep in case we want it later!
  
  /*
  
  GListHeader lh_from_month,lh_from_day,lh_to_month,lh_to_day;
  GList l_from_month,l_from_day,l_to_month,l_to_day;
  GLabel _dates, _start_date, _end_date;
  GTextEntry t_from_year,t_to_year,t_seed1,t_seed2,t_nodemul;
  GregorianCalendar gc;
  
  
  public void updateCalendar(GTextEntry lYear, GList lMonth, GList lDay) {
      // Month or year changed, change day options
    int year = Integer.parseInt(lYear.getText());
    gc.set(GregorianCalendar.YEAR,year);
    gc.set(GregorianCalendar.MONTH,lMonth.getSelected());
    int max_day = gc.getActualMaximum(GregorianCalendar.DAY_OF_MONTH);
    int currentSelection = lDay.getSelected();
    if (currentSelection>max_day-1) currentSelection=max_day-1;
    lDay.clearEntries();
    for (int i=1; i<=max_day; i++) lDay.addEntry(String.valueOf(i));
    lDay.setSelected(currentSelection);
    lDay.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }
  
  */
  
  
  
  GLabel _seed1,_seed2;
  GTextEntry t_seed1,t_seed2;
  
 
  public void initCities() {
    try {
      SAXReader reader = new SAXReader();
      Document doc = reader.read(ga.dataPath+"cities.xml");
      Element root = doc.getRootElement();
      int no_cities = root.selectNodes("c").size();
      c_lon = new ArrayList<Integer>();
      c_lat = new ArrayList<Integer>();
      Node n;
      int lon,lat;
      String name;
      for (int i=1; i<=no_cities; i++) {
        n=root.selectSingleNode("c["+i+"]");
        lat=(int) (100*(Double.parseDouble(n.valueOf("@y"))));
        lon=(int) (100*(Double.parseDouble(n.valueOf("@x"))));
        name=n.valueOf("@n")+", "+n.valueOf("@c");
        int j=0;
        boolean found=false;
        while ((!found) && (j<l_cities.countEntries())) {
          if (l_cities.getEntry(j).compareTo(name)>0) {
            found=true;
            c_lon.add(j,lon);
            c_lat.add(j,lat);
            l_cities.addEntry(j,name);
          }
          else j++;
        }
        if (!found) {
          c_lon.add(lon);
          c_lat.add(lat);
          l_cities.addEntry(name);
        }
      }
      doc.clearContent();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public SeedingPage(GWindow _gw, GSim _ga) {
    gw=_gw;
    ga=_ga;
    //gc = new GregorianCalendar();
    //gc.set(GregorianCalendar.HOUR_OF_DAY,12); // Just to make sure we don't get any timezone silliness...
    tt_long= new ArrayList<GTextEntry>();
    tt_lat= new ArrayList<GTextEntry>();
    tt_time= new ArrayList<GTextEntry>();
    tt_no= new ArrayList<GTextEntry>();
    bb_remove = new ArrayList<GButton>();
    longs = new ArrayList<String>();
    lats = new ArrayList<String>();
    nos = new ArrayList<Integer>();
    times = new ArrayList<String>();
    first_seed=0;
    no_seeds=0;
    
  }
  

  //public final int FROM_SELECT = 800;
  //public final int TO_SELECT = 801;
  public final int CHANGE_VAL = 803;
  public final int CHECK_ADD = 804;
  public final int ADD_SEED = 805;
  public final int REMOVE_SEED = 806;
  public final int SLIDE = 807;
  public final int EDIT_ENTRY = 808;
  public final int SELECT_CITY = 809;
  
  public int getMinEvent() { return 800; }
  public int getMaxEvent() { return 899; }
  
  
  public void saveXML(Element root) {
    Node seed_node=root.selectSingleNode("seeding");
    if (seed_node!=null) {
      root.remove(seed_node);
    }
    Element seed_el=root.addElement("seeding");
    seed_el.addElement("seed1").addAttribute("value",t_seed1.getText());
    seed_el.addElement("seed2").addAttribute("value",t_seed2.getText());
    Element seeds_el = seed_el.addElement("seedinf");
    seeds_el.addAttribute("no",String.valueOf(longs.size()));
    for (int i=0; i<longs.size(); i++) {
      seeds_el.addElement("seed").addAttribute("lon",longs.get(i))
                                 .addAttribute("lat",lats.get(i))
                                 .addAttribute("time",times.get(i))
                                 .addAttribute("no",String.valueOf(nos.get(i)));
    }
  }
  
  public void saveBinary(DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(Integer.parseInt(t_seed1.getText())));
      dos.writeInt(Integer.reverseBytes(Integer.parseInt(t_seed2.getText())));
      dos.writeInt(Integer.reverseBytes(no_seeds));
      for (int i=0; i<no_seeds; i++) {
        dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(longs.get(i))))));
        dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(lats.get(i))))));
        dos.writeDouble(Double.longBitsToDouble(Long.reverseBytes(Double.doubleToRawLongBits(Double.parseDouble(times.get(i))))));
        dos.writeInt(Integer.reverseBytes(nos.get(i)));
      }
      
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void clear() {}
  
  public void createDefaultXML(Element root) {
    saveXML(root);
  }
  
  public void loadXML(Element root) {
    Node seed_node = root.selectSingleNode("seeding");
    longs.clear();
    lats.clear();
    times.clear();
    nos.clear();
    if (seed_node!=null) {
      t_seed1.setText(seed_node.selectSingleNode("seed1").valueOf("@value"));
      t_seed2.setText(seed_node.selectSingleNode("seed2").valueOf("@value"));
      no_seeds = Integer.parseInt(seed_node.selectSingleNode("seedinf").valueOf("@no"));
      for (int i=0; i<no_seeds; i++) {
        Node n = seed_node.selectSingleNode("seedinf").selectSingleNode("seed["+(i+1)+"]");
        longs.add(n.valueOf("@lon"));
        lats.add(n.valueOf("@lat"));
        times.add(n.valueOf("@time"));
        nos.add(Integer.parseInt(n.valueOf("@no")));
      }
    }
    updateGrid();
    gs_vert.updateSliderSize(longs.size(),max_seeds_screen);
  }

  
  
  public void doFunction(int func,Object component) {
    //if (func==FROM_SELECT) updateCalendar(t_from_year,l_from_month,l_from_day);
    //else if (func==TO_SELECT) updateCalendar(t_to_year,l_to_month,l_to_day);
    /*else */
    if (func==CHANGE_VAL) changeVal();
    else if (func==CHECK_ADD) checkAddStatus();
    else if (func==ADD_SEED) addSeed();
    else if (func==REMOVE_SEED) removeSeed(component);
    else if (func==SLIDE) slide_seed();
    else if (func==EDIT_ENTRY) editEntry(component);
    else if (func==SELECT_CITY) selectCity();

  }
  
  public String convert(int ll) {
    double d = ((int)(ll/100))+(100.0*(((double)ll/100.0)-(ll/100))/60.0);
    DecimalFormat twoDForm = new DecimalFormat("###.#####");
    return String.valueOf(twoDForm.format(d));
  }
    
  
  public void selectCity() {
    int i = l_cities.getSelected();
    if (i!=-1) {
      t_long.setText(convert(c_lon.get(i)));
      t_lat.setText(convert(c_lat.get(i)));
    }
  }
  
  public void slide_seed() {
    first_seed=gs_vert.getValue();
    updateGrid();
  }
  
  public void changeVal() {
    ga.setUnSaved(true);
    gw.requestRepaint();
  }
  
  
  public void updateGrid() {
    for (int i=0; i<max_seeds_screen; i++) {
      boolean show=false;
      if (i<no_seeds) {
        show=true;
        tt_long.get(i).setText(longs.get(i+first_seed));
        tt_lat.get(i).setText(lats.get(i+first_seed));
        tt_time.get(i).setText(times.get(i+first_seed));
        tt_no.get(i).setText(String.valueOf(nos.get(i+first_seed)));
      } else {
        show=false;
      }
      tt_long.get(i).setVisible(show);
      tt_lat.get(i).setVisible(show);
      tt_time.get(i).setVisible(show);
      tt_no.get(i).setVisible(show);
      bb_remove.get(i).setVisible(show);
      tt_long.get(i).paintOn(gw.bi(),gw.g2d());
      tt_lat.get(i).paintOn(gw.bi(),gw.g2d());
      tt_time.get(i).paintOn(gw.bi(),gw.g2d());
      tt_no.get(i).paintOn(gw.bi(),gw.g2d());
      bb_remove.get(i).paintOn(gw.bi(),gw.g2d());
    }
    //panel.paintOn(gw.bi(),gw.g2d());
    gw.requestRepaint();
  }

  public void checkAddStatus() {
    if ((t_long.getText().trim().length()>0) && (t_long.getText().trim().length()>0) && 
        (t_time.getText().trim().length()>0) && (t_no.getText().trim().length()>0)) {
      try {
        Double.parseDouble(t_long.getText());
        Double.parseDouble(t_lat.getText());
        Integer.parseInt(t_no.getText());
        Double.parseDouble(t_time.getText());
        b_add.setEnabled(true);
      } catch (Exception e) {
        b_add.setEnabled(false);
      }
    } else b_add.setEnabled(false);
  }
  
  public void addSeed() {
    boolean found=false;
    double compare_day=Double.parseDouble(t_time.getText());
    for (int i=0; i<times.size(); i++) {
      if (t_time.getText().equals(times.get(i))) {
        found=true;
        times.add(i,t_time.getText());
        nos.add(i,Integer.parseInt(t_no.getText()));
        longs.add(i,t_long.getText());
        lats.add(i,t_lat.getText());
        i=times.size();
      } else {
        double day = Double.parseDouble(times.get(i));
        if (compare_day<day) {
          found=true;
          times.add(i,t_time.getText());
          nos.add(i,Integer.parseInt(t_no.getText()));
          longs.add(i,t_long.getText());
          lats.add(i,t_lat.getText());
          i=times.size();
        }
      }
    }
    if (!found) {
      times.add(t_time.getText());
      nos.add(Integer.parseInt(t_no.getText()));
      longs.add(t_long.getText());
      lats.add(t_lat.getText());
    }
    no_seeds++;
    updateGrid();
    gs_vert.updateSliderSize(longs.size(),max_seeds_screen);
    changeVal();
    gw.update();
    gw.requestRepaint();
  }
  
  public void removeSeed(Object comp) {
    GButton b = (GButton) comp;
    int y_index=(int) ((b.getY()-t_starty)/t_spacingy);
    times.remove(first_seed+y_index);
    longs.remove(first_seed+y_index);
    lats.remove(first_seed+y_index);
    nos.remove(first_seed+y_index);
    no_seeds--;
    changeVal();
    updateGrid();
    gs_vert.updateSliderSize(longs.size(),max_seeds_screen);
    gw.update();
    gw.requestRepaint();
  }
  
  public void editEntry(Object comp) {
    if (comp instanceof GTextCell) {
      GTextCell t = (GTextCell) comp;
      int y_index=(int) ((t.getY()-t_starty)/t_spacingy);
      times.set(first_seed+y_index,tt_time.get(y_index).getText());
      longs.set(first_seed+y_index,tt_long.get(y_index).getText());
      lats.set(first_seed+y_index,tt_lat.get(y_index).getText());
      nos.set(first_seed+y_index,Integer.parseInt(tt_no.get(y_index).getText()));
      changeVal();
      gw.requestRepaint();
    }
  }

  
  public void initGUI(GPanel s_setup) {
    /*
     * String[] months = new String[] {"January","February","March","April","May","June","July","August","September","October","November","December"};
    String[] days = new String[31];
    for (int i=0; i<31; i++) days[i]=String.valueOf(i+1);
    
    
    _dates = (GLabel) s_setup.addChild(new GLabel(20,20,"Dates",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    
    _start_date = (GLabel) s_setup.addChild(new GLabel(50,50,"Start Date",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    l_from_day = (GList) s_setup.addChild(new GList(130,44,45,120,s_setup,days,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_from_day = (GListHeader) s_setup.addChild(new GListHeader(130,44,45,s_setup,gw,l_from_day,ga.NONE));
    l_from_day.setGListHeader(lh_from_day);
    l_from_day.setVisible(false);
    
    l_from_month = (GList) s_setup.addChild(new GList(180,44,100,120,s_setup,months,gw,GList.SINGLE_SELECTION,FROM_SELECT));
    lh_from_month = (GListHeader) s_setup.addChild(new GListHeader(180,44,100,s_setup,gw,l_from_month,ga.NONE));
    l_from_month.setGListHeader(lh_from_month);
    l_from_month.setVisible(false);
    t_from_year = (GTextEntry) s_setup.addChild(new GTextEntry(285,44,48,s_setup,gw,"2010",FROM_SELECT));
    
    _end_date = (GLabel) s_setup.addChild(new GLabel(380,50,"End Date",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    l_to_day = (GList) s_setup.addChild(new GList(480,44,45,120,s_setup,days,gw,GList.SINGLE_SELECTION,ga.NONE));
    lh_to_day = (GListHeader) s_setup.addChild(new GListHeader(480,44,45,s_setup,gw,l_to_day,ga.NONE));
    l_to_day.setGListHeader(lh_to_day);
    l_to_day.setVisible(false);
    
    l_to_month = (GList) s_setup.addChild(new GList(530,44,100,120,s_setup,months,gw,GList.SINGLE_SELECTION,TO_SELECT));
    lh_to_month = (GListHeader) s_setup.addChild(new GListHeader(530,44,100,s_setup,gw,l_to_month,ga.NONE));
    l_to_month.setGListHeader(lh_to_month);
    l_to_month.setVisible(false);
    t_to_year = (GTextEntry) s_setup.addChild(new GTextEntry(635,44,48,s_setup,gw,"2012",TO_SELECT));
    */
    
    s_setup.addChild(new GLine(0,120,864,120, new Color(gw.getGCS().getEdge()), s_setup, gw));
    
    //_seeds = (GLabel) s_setup.addChild(new GLabel(20,140,"Random Seeds",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    _seed1 = (GLabel) s_setup.addChild(new GLabel(180,50,"Seed 1:",GLabel.RIGHT_ALIGN,ga.NONE,s_setup,gw));
    t_seed1 = (GTextEntry) s_setup.addChild(new GTextEntry(200,44,60,s_setup,gw,"12345",CHANGE_VAL));
    _seed2 = (GLabel) s_setup.addChild(new GLabel(290,50,"Seed 2:",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    t_seed2 = (GTextEntry) s_setup.addChild(new GTextEntry(365,44,60,s_setup,gw,"67890",CHANGE_VAL));
    
    for (int j=0; j<max_seeds_screen; j++) {
      GTextEntry tlon = (GTextEntry) s_setup.addHiddenChild(new GTextEntry(20,t_starty+(j*t_spacingy),100,s_setup,gw,"",EDIT_ENTRY));
      tt_long.add(tlon);
      GTextEntry tlat = (GTextEntry) s_setup.addHiddenChild(new GTextEntry(130,t_starty+(j*t_spacingy),100,s_setup,gw,"",EDIT_ENTRY));
      tt_lat.add(tlat);
      GTextEntry ttime = (GTextEntry) s_setup.addHiddenChild(new GTextEntry(240,t_starty+(j*t_spacingy),60,s_setup,gw,"",EDIT_ENTRY));
      tt_time.add(ttime);
      GTextEntry tno = (GTextEntry) s_setup.addHiddenChild(new GTextEntry(310,t_starty+(j*t_spacingy),60,s_setup,gw,"",EDIT_ENTRY));
      tt_no.add(tno);
      GButton but = (GButton) s_setup.addHiddenChild(new GButton(380,t_starty+(j*t_spacingy),20,22,s_setup,gw,REMOVE_SEED,"X"));
      bb_remove.add(but);
      
    }
    _long = (GLabel) s_setup.addChild(new GLabel(20,140,"Longitude",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    _lat = (GLabel) s_setup.addChild(new GLabel(130,140,"Latitude",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    _time = (GLabel) s_setup.addChild(new GLabel(240,140,"Day",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    _no = (GLabel) s_setup.addChild(new GLabel(310,140,"No.",GLabel.LEFT_ALIGN,ga.NONE,s_setup,gw));
    t_long = (GTextEntry) s_setup.addChild(new GTextEntry(20,170,100,s_setup,gw,"",CHECK_ADD));
    t_lat = (GTextEntry) s_setup.addChild(new GTextEntry(130,170,100,s_setup,gw,"",CHECK_ADD));
    t_time = (GTextEntry) s_setup.addChild(new GTextEntry(240,170,60,s_setup,gw,"",CHECK_ADD));
    t_no = (GTextEntry) s_setup.addChild(new GTextEntry(310,170,60,s_setup,gw,"",CHECK_ADD));
    b_add = (GButton) s_setup.addChild(new GButton(380,170,20,22,s_setup,gw,ADD_SEED,"+"));
    gs_vert = (GSlider) s_setup.addChild(new GSlider(420,t_starty,(max_seeds_screen*t_spacingy),GSlider.SLIDE_VERT,s_setup,0,0,0,SLIDE,gw));
    l_cities = (GList) s_setup.addChild(new GList(500,t_starty,300,(max_seeds_screen*t_spacingy),s_setup,null,gw,GList.SINGLE_SELECTION,SELECT_CITY));
    initCities();
    checkAddStatus();
    gs_vert.updateSliderSize(longs.size(),max_seeds_screen);
    
  }
}

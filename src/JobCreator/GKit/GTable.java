/* GTable.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A table class with fine scroll. Selection not fully implemented 
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

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;


public class GTable extends GComponent implements GNotifier {
  int edit_function,select_function;
  private boolean fix_col_title,fix_row_title,cell_editing;
  private GTextCell edit_cell;
  private int total_height,total_width;
  ArrayList<ArrayList<GTextCell>> gtcs;
  private ArrayList<GTextCell> col_headers;
  int x_spacing,y_spacing;
  private ArrayList<Boolean>col_editable;
  private ArrayList<Boolean>row_editable;
    
  private Color cell_b_colour,cell_f_colour,title_b_colour,title_f_colour,cell_sel_colour;
  private int title_height=25;
  private BufferedImage temp;       // For off-screen cell rendering
  private Graphics2D tempg;         // Ditto
  private GSlider gs_horiz,gs_vert;
  
  private ArrayList<Boolean>col_selectable;
  private ArrayList<Boolean>row_selectable;
  
  private byte current_selection_mode;
  private byte selection_model;
  private static final byte NO_SELECTION = 0;
  private static final byte SINGLE_CELL = 1;
  private static final byte SELECT_MULTI = 2;
  
  public static final byte SINGLE_CELL_CELECTION = 1;
  public static final byte LINEAR_SELECTION = 2;
  public static final byte MULTI_SELECTION = 3;
  
  
  public void setSelectionContinuous(int from, int to) {
    select_y1=from;
    select_y2=to;
    select_y=to;
  }
  
  public void setSelected(int x, int y, boolean b) {
    gtcs.get(y).get(x).cell_selected=b;
  }
  
  public ArrayList<Integer> getSelectedCells() {
    ArrayList<Integer> ai = new ArrayList<Integer>();
    for (int j=0; j<gtcs.size(); j++) {
      for (int i=0; i<gtcs.get(j).size(); i++) {
        if (gtcs.get(j).get(i).cell_selected) {
          ai.add(i);
          ai.add(j);
        }
      }
    }
    return ai;
  }
        
  
  public void setSelectionModel(byte sel) { selection_model=sel; }
  
  public int getSelectionTop() {
    
    if (select_y1>=0) return select_y1;
    else return -1;
  }
  
  public int getSelectionBottom() {
    if (select_y2>=0) return select_y2;
    else return -1;
  }
  
  int select_y1,select_x1,select_x2,select_y2; // Indicating the range being selected
  int select_x,select_y;  // indicating the cell being clicked on for selection.
  
  
  public int countRows() { return gtcs.size(); }
  public int countCols() { return gtcs.get(0).size(); }
  
  public void setColHeaderEditable(boolean b) {
    for (int i=0; i<col_editable.size(); i++) col_headers.get(i).allow_edit=b;
  }
  
  public void setColHeaderEditable(int col, boolean b) { col_headers.get(col).allow_edit=b; }
  
  public void setColHeaderSelectable(boolean b) {
    for (int i=0; i<col_headers.size(); i++) col_headers.get(i).allow_cell_selection=b;
  }
  
  public void setColHeaderSelectable(int col, boolean b) { col_headers.get(col).allow_cell_selection=b; }
  public void setEditable(int x, int y, boolean b) { gtcs.get(x).get(y).allow_edit=b; }
    
  public void setColEditable(int x, boolean b) { 
    col_editable.set(x,b);
    for (int i=0; i<gtcs.size(); i++) {
      gtcs.get(i).get(x).allow_edit=b;
    }
  }
  
  public void setRowEditable(int y, boolean b) { 
    row_editable.set(y,b);
    if (gtcs.size()>0) {
      for (int i=0; i<gtcs.get(0).size(); i++) {
        gtcs.get(y).get(i).allow_edit=b;
      }
    }
  }
  
  public void setSelectable(int x, int y, boolean b) { gtcs.get(x).get(y).allow_cell_selection=b; }
  public void setColSelectable(int x, boolean b) { 
    col_selectable.set(x,b);
    for (int i=0; i<gtcs.size(); i++) {
      gtcs.get(i).get(x).allow_cell_selection=b;
    }
  }
  
  public void setRowSelectable(int y, boolean b) { 
    row_selectable.set(y,b);
    if (gtcs.size()>0) {
      for (int i=0; i<gtcs.get(0).size(); i++) {
        gtcs.get(y).get(i).allow_cell_selection=b;
      }
    }
  }
  public void setValue(int x, int y, String val) { gtcs.get(y).get(x).setValue(val,false); }
  
  public void notifyMe(Object component) { gw.ga.doFunction(edit_function,component); }
    
  public String getValue(int x, int y) { 
    return new String(gtcs.get(y).get(x).getValue()); 
  } 
  
  public void setCellBackColour(Color c) { cell_b_colour = new Color(c.getRGB()); }
  public void setCellForeColour(Color c) { cell_f_colour = new Color(c.getRGB()); }  
  public void setTitleBackColour(Color c) { title_b_colour = new Color(c.getRGB()); }
  public void setTitleForeColour(Color c) { title_f_colour = new Color(c.getRGB()); }
  public void setCellSelectionColour(Color c) { cell_sel_colour = new Color(c.getRGB()); }    
  
  
  public void setBackColour(int x, int y, Color c) { gtcs.get(y).get(x).setBackColour(c); }
  public void setColBackColour(int x, Color c) { for (int i=0; i<gtcs.size(); i++) gtcs.get(i).get(x).setBackColour(c); }
  public void setRowBackColour(int y, Color c) { for (int i=0; i<gtcs.get(0).size(); i++) gtcs.get(y).get(i).setBackColour(c); }
  public void setAllBackColour(Color c) { for (int i=0; i<gtcs.size(); i++) setRowBackColour(i,c); }
  public void setForeColour(int x, int y, Color c) { gtcs.get(y).get(x).setTextColour(c); }
  public void setColForeColour(int x, Color c) { for (int i=0; i<gtcs.size(); i++) gtcs.get(i).get(x).setBackColour(c); }
  public void setRowForeColour(int y, Color c) { for (int i=0; i<gtcs.get(0).size(); i++) gtcs.get(y).get(i).setBackColour(c); }
  public void setAllForeColour(Color c) { for (int i=0; i<gtcs.size(); i++) setRowBackColour(i,c); }
    
  
  public void setColTitleBackColour(int x, Color c) { col_headers.get(x).setBackColour(c); }
  public void setAllColTitleBackColour(Color c) { for (int i=0; i<col_headers.size(); i++) col_headers.get(i).setBackColour(c); }
  public void setColTitleForeColour(int x, Color c) { col_headers.get(x).setTextColour(c); }
  public void setAllColTitleForeColour(Color c) { for (int i=0; i<col_headers.size(); i++) col_headers.get(i).setTextColour(c); }
  
  public void updateScrollLimits() {
    gs_vert.setMin(0);
    gs_vert.setMax(Math.max(0,(total_height-height)));
    gs_vert.updateSliderSize(total_height,height-20);
    gs_horiz.setMin(0);
    gs_horiz.setMax(Math.max(0,(total_width-width)));
    gs_horiz.updateSliderSize(total_width,width-20);

  }
  
  
  public GTextCell defaultCell(int _width,String def,Color fore, Color back, Color select) {
    GTextCell _gtc = new GTextCell(0,0,_width,25,parent,gw,0,new String(def));
    _gtc.setTextColour(fore);
    _gtc.setBackColour(back);
    _gtc.notify=this;
    _gtc.setSelectedColour(select);
    return _gtc;
  }
  
  public void addRow(String[] _data, int index) {
    ArrayList<GTextCell> n = new ArrayList<GTextCell>();
    for (int i=0; i<_data.length; i++) {
      GTextCell _new = defaultCell(col_headers.get(i).width,_data[i],cell_f_colour,cell_b_colour,cell_sel_colour);
      _new.allow_edit=col_editable.get(i);
      _new.allow_cell_selection=col_selectable.get(i);
      n.add(_new);
      
    }
    if (index==-1) {
      gtcs.add(n);
      row_editable.add(true);
      row_selectable.add(true);
    } else { 
      gtcs.add(index,n);
      row_editable.add(index,true);
      row_selectable.add(index,true);
    }
    total_height+=27;
    updateScrollLimits();
  }
  
  public void addRow(String[] _data) { addRow(_data,-1); }
  
  public void removeRow(int index) {
    total_height-=(gtcs.get(index).get(0).height+1);
    gtcs.remove(index);
    row_editable.remove(index);
    row_selectable.remove(index);
    updateScrollLimits();
  }
  
  public void setColWidths(int[] _widths) {
    total_width=0;
    for (int i=0; i<_widths.length; i++) {
      total_width+=_widths[i]+1;
      for (int j=0; j<gtcs.size(); j++) gtcs.get(j).get(i).width=_widths[i];
      col_headers.get(i).width=_widths[i];
    }
  }
  
  public void setColWidth(int index, int wid) {
    total_width-=(col_headers.get(index).width+1);
    total_width+=wid+1;
    for (int j=0; j<gtcs.size(); j++) gtcs.get(j).get(index).width=wid;
    col_headers.get(index).width=wid;
  }
  
  public void setRowHeights(int[] _heights) {
    total_height=0;
    for (int i=0; i<_heights.length; i++) {
      total_height+=_heights[i]+1;
      for (int j=0; j<gtcs.size(); j++) gtcs.get(j).get(i).height=_heights[i];
    }
  }
  
  public void setRowHeight(int index, int hei) {
    total_height-=(gtcs.get(index).get(0).width+1);
    total_width+=hei+1;
    for (int j=0; j<gtcs.size(); j++) gtcs.get(j).get(index).height=hei;
    if (index==-1) col_headers.get(index).height=hei;
  }
  
  
  public void addColumns(String[] title, String[] def) {
    for (int i=0; i<title.length; i++) addColumn(title[i],def[i]);
  }
  
  public void addColumn(String title,String def) { addColumn(title,def,-1,null); }
    
  public void addColumn(String title,String def,int index,ArrayList<String> vals) {
    for (int i=0; i<gtcs.size(); i++) {
      if (vals!=null) def=vals.get(i);
      GTextCell _gtc= defaultCell(100,def,cell_f_colour,cell_b_colour,cell_sel_colour);
      if (index==-1) {
        gtcs.get(i).add(_gtc);
        _gtc.allow_edit=row_editable.get(i);
        _gtc.allow_cell_selection=row_selectable.get(i);
        
      } else { 
        gtcs.get(i).add(index,_gtc);
        _gtc.allow_edit=row_editable.get(i);
        _gtc.allow_cell_selection=row_selectable.get(i);
      }
    }
    
    if (index==-1) {
      col_headers.add(defaultCell(100,title,title_f_colour,title_b_colour,cell_sel_colour));
      col_editable.add(true);
      col_selectable.add(true);
    } else {
      col_headers.add(index,defaultCell(100,title,title_f_colour,title_b_colour,cell_sel_colour));
      col_editable.add(index,true);
      col_selectable.add(index,true);
    }
    total_width+=101;
  }
  
  public void removeColumn(int index) {
    total_width-=(gtcs.get(0).get(index).width+1);
    for (int i=0; i<gtcs.size(); i++) {
      gtcs.get(i).remove(index);
    }
    col_headers.remove(index);
    col_editable.remove(index);
    col_selectable.remove(index);
  }
  
  public GTable(int _x, int _y, int _wid, int _hei, GContainer _parent, GWindow _gw, int ed_function,int sel_function) {
    super(_parent,_gw);
    gw=_gw;
    parent=_parent;
    width=_wid;
    height=_hei;
    pos_x=_x;
    pos_y=_y;
    x_spacing=1;
    y_spacing=2;
    cell_editing=false;
    total_height=title_height; // Title - scroll bar size+borders
    total_width=0;
    gtcs = new ArrayList<ArrayList<GTextCell>>();
    col_headers = new ArrayList<GTextCell>();
    cell_b_colour = new Color(gw.gcs.table_back);
    cell_f_colour = new Color(gw.gcs.table_fore);    
    title_b_colour = new Color(gw.gcs.table_title_back);
    title_f_colour = new Color(gw.gcs.table_title_fore);
    cell_sel_colour = new Color(gw.gcs.table_cell_select);
    temp = new BufferedImage(800,50,BufferedImage.TYPE_3BYTE_BGR);
    tempg = (Graphics2D) temp.getGraphics();
    tempg.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    tempg.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
    tempg.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
    tempg.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
    tempg.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
    tempg.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
    tempg.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
    tempg.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,RenderingHints.VALUE_STROKE_NORMALIZE);
    gs_vert = new GSlider(pos_x+width-13,pos_y+4,height-16,GSlider.SLIDE_VERT,parent,0,10,0,0,gw);
    parent.addChild(gs_vert);
    gs_horiz = new GSlider(pos_x+4,pos_y+height-16,width-20,GSlider.SLIDE_HORIZ,parent,0,10,0,0,gw);
    parent.addChild(gs_horiz);
    gs_horiz.owner=this;
    gs_vert.owner=this;
    edit_function=ed_function;
    select_function=sel_function;
    row_editable=new ArrayList<Boolean>();
    col_editable=new ArrayList<Boolean>();
    row_selectable=new ArrayList<Boolean>();
    col_selectable=new ArrayList<Boolean>();
    current_selection_mode=NO_SELECTION;
    selection_model=LINEAR_SELECTION;
    
  }
  
  public void paintRow(int x_pix, int y_pix, int row_no, int clip_top, int clip_bottom, BufferedImage bi, FontMetrics fm) {
    int rel_x = parent.get_parent_pos_x();
    int rel_y = parent.get_parent_pos_y();
    int abs_left_x=parent.get_parent_pos_x()+pos_x+2;
    int col_start=0;
    int ok_width=width-20;
    if (fix_row_title) {
      if (row_no>=0) {
        gtcs.get(row_no).get(0).renderCell(abs_left_x,y_pix,clip_top,0,clip_bottom,0,fm,bi);
        gtcs.get(row_no).get(0).pos_x=abs_left_x-rel_x;
        gtcs.get(row_no).get(0).pos_y=y_pix-rel_y;
      }
      else {
        col_headers.get(0).renderCell(abs_left_x,y_pix,clip_top,0,clip_bottom,0,fm,bi);
        col_headers.get(0).pos_x=abs_left_x-rel_x;
        col_headers.get(0).pos_y=y_pix-rel_y;
      }
      x_pix+=gtcs.get(0).get(0).width;
      abs_left_x+=gtcs.get(0).get(0).width+1;
      col_start=1;
      ok_width-=(gtcs.get(0).get(0).width+1);
    }
    
    x_pix-=gs_horiz.getValue();
    for (int i=col_start; i<col_headers.size(); i++) {
      if ((x_pix+gtcs.get(0).get(i).width>=abs_left_x) && (x_pix<abs_left_x+ok_width)) {
        int clip_left=0;
        int clip_right=0;
        if (x_pix<abs_left_x) clip_left=abs_left_x-x_pix;
        if (x_pix+gtcs.get(0).get(i).width>abs_left_x+ok_width) clip_right=x_pix+gtcs.get(0).get(i).width-(abs_left_x+ok_width);
        if (row_no>=0) {
          gtcs.get(row_no).get(i).renderCell(x_pix+clip_left,y_pix,clip_top,clip_left,clip_bottom,clip_right,fm,bi);
          gtcs.get(row_no).get(i).pos_x=x_pix+clip_left-rel_x;
          gtcs.get(row_no).get(i).pos_y=y_pix-rel_y;
        } else { 
          col_headers.get(i).renderCell(x_pix+clip_left,y_pix,clip_top,clip_left,clip_bottom,clip_right,fm,bi);
          col_headers.get(i).pos_x=x_pix+clip_left-rel_x;
          col_headers.get(i).pos_y=y_pix-rel_y;
        }
      }
      x_pix+=gtcs.get(0).get(i).width+x_spacing;
    }
  }
  
  public void setFixedColHeader(boolean set, boolean apply_colour) { 
    fix_col_title=set; 
    if (apply_colour) {
      setColForeColour(0,new Color(gw.gcs.table_title_fore));
      setColBackColour(0,new Color(gw.gcs.table_title_back));
    }
  }
  
  public void setFixedRowHeader(boolean set, boolean apply_col) { 
    fix_row_title=set; 
    if (apply_col) {
      setColForeColour(0,new Color(gw.gcs.table_title_fore));
      setColBackColour(0,new Color(gw.gcs.table_title_back));
    }
  }
  
  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      FontMetrics fm = g.getFontMetrics(GWindow.TFIELD_FONT);
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }
      gs_vert.setBigJump(height/3);
      gs_horiz.setBigJump(width/3);
      g.setStroke(new BasicStroke(2.0f)); // 2-pixel lines
      RoundRectangle2D roundedRectangle = new RoundRectangle2D.Float(par_x+pos_x,par_y+pos_y,width,height,10,10);
      g.setColor(backColour);
      g.fill(roundedRectangle);
      g.setColor(edgeColour);
      g.draw(roundedRectangle);
      g.setFont(GWindow.TFIELD_FONT);
      int y_pix=par_y+pos_y+2;
      int x_pix=par_x+pos_x+2;
      int clip_top=0;
      int clip_bottom=0;
     
      if (fix_col_title) {
        paintRow(x_pix,y_pix,-1,0,0,bi,fm);
        y_pix+=title_height+y_spacing;
        y_pix-=gs_vert.getValue();
        for (int j=0; j<gtcs.size(); j++) {
          clip_bottom=0;
          clip_top=0;
          if (y_pix+gtcs.get(j).get(0).height>=par_y+pos_y+4+title_height) {
            if (y_pix<par_y+pos_y+4+title_height) clip_top=(par_y+pos_y+4+title_height)-y_pix;
            if ((y_pix+gtcs.get(j).get(0).height)>par_y+pos_y+(height-20)) clip_bottom=(y_pix+gtcs.get(j).get(0).height)-(par_y+pos_y+(height-20));
            if (y_pix<par_y+pos_y+height-10) paintRow(x_pix,y_pix+clip_top,j,clip_top,clip_bottom,bi,fm);
          }
          y_pix+=gtcs.get(j).get(0).height+y_spacing;
        }
      }
      
      else if (!fix_col_title) {
        y_pix-=gs_vert.getValue();
        clip_bottom=0;
        clip_top=0;
        if (y_pix+title_height>=par_y+pos_y+2) {
          if (y_pix<par_y+pos_y+2) clip_top=(par_y+pos_y+2)-y_pix;
          if ((y_pix+title_height)>par_y+pos_y+(height-20)) clip_bottom=(y_pix+title_height)-(par_y+pos_y+(height-20));
          if (y_pix<par_y+pos_y+height-10) paintRow(x_pix,y_pix+clip_top,-1,clip_top,clip_bottom,bi,fm);
        }
        y_pix+=title_height+y_spacing;
        for (int j=0; j<gtcs.size(); j++) {
          clip_bottom=0;
          clip_top=0;
          if (y_pix+gtcs.get(j).get(0).height>=par_y+pos_y+2) {
            if (y_pix<par_y+pos_y+2) clip_top=(par_y+pos_y+2)-y_pix;
            if ((y_pix+gtcs.get(j).get(0).height)>par_y+pos_y+(height-20)) clip_bottom=(y_pix+gtcs.get(j).get(0).height)-(par_y+pos_y+(height-20));
            if (y_pix<par_y+pos_y+height-10) paintRow(x_pix,y_pix+clip_top,j,clip_top,clip_bottom,bi,fm);
          }
          y_pix+=gtcs.get(j).get(0).height+y_spacing;
        }
      } 
      
      gs_vert.paintOn(bi,g);
      gs_horiz.paintOn(bi,g);
      
    }
  }

  public GComponent mouseExit(GComponent next) {
    if (next instanceof GSlider) {
      GSlider _gs = (GSlider) next;   // Moved from glist to gslider - check they're not brothers.
      if (_gs.owner!=this) {
        highlight=false;
        gs_horiz.highlight=false;
        gs_vert.highlight=false;
      }
    } else {
      highlight=false;
      gs_horiz.highlight=false;
      gs_vert.highlight=false;
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();    
    return next;
  }


  public void mouseWithin(int x, int y) {
  }

  public GComponent mouseEnter(int x, int y, GComponent previous) {
    highlight=true;
    gs_horiz.highlight=true;
    gs_vert.highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }

  public void mouseClick(int x, int y, MouseEvent e) {
    
    int sx=x;
    int sy=y;
    int ok_width=width-20;
    int ok_height=height-20;
    int par_x=0,par_y=0;
    if (parent!=null) {
      par_x = parent.get_parent_pos_x();
      par_y = parent.get_parent_pos_y();
    }
    
    x-=(pos_x+par_x);
    y-=(pos_y+par_y);
    
    // Below, establish column and row of cell clicked, and also ensure cell is entirely visible.
    // This assumes onscreen table is wider than cell...
    
    int x_col=-1;
    int test=0;
    int compare=0;
    if (fix_row_title) {
      if (x<gtcs.get(0).get(0).width) x_col=0;
      else {
        test=gs_horiz.getValue()+(x-gtcs.get(0).get(0).width);
      }
    } else {
      test=gs_horiz.getValue()+x;
      if (test<gtcs.get(0).get(0).width) x_col=0;  
      else compare=gtcs.get(0).get(0).width;
    }
    
    // Regardless of fix_row_title, we have now dealt with column 0!
    
    if (x_col==-1) {
      x_col=1;
      while ((compare<test) && (x_col<gtcs.get(0).size())) {
        compare+=gtcs.get(0).get(x_col).width+x_spacing;
        x_col++;
      }
      x_col--;
    }
    
    
    if (fix_row_title) {
      if (compare>(gs_horiz.getValue()+(ok_width-gtcs.get(0).get(0).width))) {
        gs_horiz.setValue(gs_horiz.getValue()+(compare-(gs_horiz.getValue()+(ok_width-gtcs.get(0).get(0).width))));
      }
    } else {
      if (compare-gs_horiz.getValue()>ok_width) gs_horiz.step_delta(compare-gs_horiz.getValue()-ok_width);
    }
    
    compare-=gs_horiz.getValue();
    if (fix_row_title) {
      if (compare<gtcs.get(0).get(0).width) {
        compare-=(gtcs.get(0).get(x_col).width+x_spacing);
        if (x_col>0) {
          gs_horiz.setValue(gs_horiz.getValue()+compare);
        }
      }
        
    } else {
//      compare-=(gtcs.get(0).get(x_col).width+x_spacing);
 //     if (compare<0) gs_horiz.setValue(gs_horiz.getValue()+compare);
    }
    
    
    int y_row=-1;
    test=0;
    compare=0;
    if (fix_col_title) {
      if (y<title_height) y_row=0;
      else {
        test=gs_vert.getValue()+(y-title_height)-y_spacing;
      }
    } else test=gs_vert.getValue()+y;
      
    if (y_row==-1) {
      y_row=0;
      while (compare<test) {
        compare+=gtcs.get(y_row).get(0).height+y_spacing;
        y_row++;
      }
    }
    
    // Now check for partial visibility issues on y-axis...
    
    if (fix_col_title) {
      if (compare>(gs_vert.getValue()+(ok_height-gtcs.get(0).get(0).height))) {
        gs_vert.setValue(gs_vert.getValue()+(compare-(gs_vert.getValue()+(ok_height-gtcs.get(0).get(0).height))));
      }
    } else {
      if (compare-gs_vert.getValue()>ok_height) gs_vert.step_delta(compare-gs_vert.getValue()-ok_height);
    }
    
    compare-=gs_vert.getValue();
    if (fix_col_title) {
      if (compare<gtcs.get(0).get(0).height) {
        compare-=(gtcs.get(y_row).get(0).height+y_spacing);
        if (y_row>0) {
          gs_vert.setValue(gs_vert.getValue()+compare);
        }
      }
        
    } else {
      compare-=(gtcs.get(y_row).get(0).height+y_spacing);
      if (compare<0) gs_vert.setValue(gs_vert.getValue()+compare);
    }
    // So click on entry x_col,y_row. Note, y_row=0 means title. y_row=1 means data.get(0)
    if (e.getClickCount()==2) {
      clearSelection();
      if (y_row>0) edit_cell=gtcs.get(y_row-1).get(x_col);
      else edit_cell=col_headers.get(x_col);
      edit_cell.cell_selected=false;
      edit_cell.mouseClick(sx,sy,e);
      gw.modalComponent=edit_cell;
    } else {
      GTextCell sel_cell;
      if (y_row>0) sel_cell=gtcs.get(y_row-1).get(x_col);
      else sel_cell=col_headers.get(x_col);
      
      if (current_selection_mode==NO_SELECTION) {
        if (sel_cell.allow_cell_selection) {
          current_selection_mode=SINGLE_CELL;
          sel_cell.cell_selected=true;
          select_y=y_row-1;
          select_x=x_col;
          select_y1=select_y;
          select_y2=select_y;
          select_x1=select_x;
          select_x2=select_x;
        }
      } else {
        if ((e.isShiftDown()) || (e.isControlDown())) {
          
          if ((selection_model==LINEAR_SELECTION) || (e.isShiftDown())) {
            int remember_sx=select_x;
            int remember_sy=select_y;
            clearSelection();
            select_x=remember_sx;
            select_y=remember_sy;
            if ((select_x!=x_col) || (select_y!=y_row-1)) { // clicked somewhere interesting
              if (select_x==x_col) { // Same x-coord, different y.
                select_y1=select_y;
                current_selection_mode=SELECT_MULTI;
                select_y2=Math.max(1,y_row-1);
                if (select_y1>select_y2) { int swap=select_y1; select_y1=select_y2; select_y2=swap; }
                for (int j=select_y1; j<=select_y2; j++) {
                  if (gtcs.get(j).get(x_col).allow_cell_selection) {
                    gtcs.get(j).get(x_col).cell_selected=true;
                  }
                }
              } else if (select_y==y_row-1) {
                select_x1=select_x;
                current_selection_mode=SELECT_MULTI;
                select_x2=x_col;
                if (select_x1>select_x2) { int swap=select_x1; select_x1=select_x2; select_x2=swap; }
                for (int i=select_x1; i<=select_x2; i++) {
                  if (gtcs.get(y_row).get(i).allow_cell_selection) {
                    gtcs.get(y_row).get(i).cell_selected=true;
                  }
                }
              }
            }
          } else {
            sel_cell.cell_selected=!sel_cell.cell_selected;
          }
        } else {
          clearSelection();
          if (sel_cell.allow_cell_selection) {
            current_selection_mode=SINGLE_CELL;
            sel_cell.cell_selected=true;
            select_y=y_row-1;
            select_x=x_col;
            select_y1=select_y;
            select_y2=select_y;
            select_x1=select_x;
            select_x2=select_x;
          }
        }
      }
      gw.ga.doFunction(select_function,GTable.this);
    }
    
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    
    
  }

  public void mouseWheel(MouseWheelEvent e) {
    int delta = e.getWheelRotation();
    gs_vert.step_delta(delta);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    
  }
  
  public void clearSelection() {
    for (int i=0; i<gtcs.size(); i++) {
      for (int j=0; j<gtcs.get(i).size(); j++) {
        gtcs.get(i).get(j).cell_selected=false;
      }
    }
    current_selection_mode=NO_SELECTION;
    select_x=-2;
    select_y=-2;
    select_x1=-2;
    select_x2=-2;
    select_y1=-2;
    select_y2=-2;
  }

  public void keyPress(KeyEvent e) {
    if (cell_editing) {
      edit_cell.keyPress(e);
    } else {
      if (e.getKeyCode()==KeyEvent.VK_DOWN) gs_vert.step_delta(20);
      else if (e.getKeyCode()==KeyEvent.VK_PAGE_DOWN) gs_vert.jump_inc();
      else if (e.getKeyCode()==KeyEvent.VK_UP) gs_vert.step_delta(-20);
      else if (e.getKeyCode()==KeyEvent.VK_PAGE_UP) gs_vert.jump_dec();
      else if (e.getKeyCode()==KeyEvent.VK_LEFT) {
        if (e.isControlDown()) gs_horiz.step_delta(-20);
        else if (e.isShiftDown()) gs_horiz.setValue(0);
        else gs_horiz.jump_dec();
      }
      else if (e.getKeyCode()==KeyEvent.VK_RIGHT) {
        if (e.isControlDown()) gs_horiz.step_delta(20);
        else if (e.isShiftDown()) gs_horiz.setValue(gs_horiz.getMax());
        else gs_horiz.jump_inc();
      }
      
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    }
  }
  
}

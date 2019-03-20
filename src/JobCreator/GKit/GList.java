/* GList.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: (Init invisible with GListHeader) - a potentially
/*       dropdown list component, or use with GSlider as a standard list
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

import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;
import java.util.ArrayList;


public class GList extends GComponent {
  private ArrayList<String> entries;
  private ArrayList<Boolean> itemsSelected;
  Color textColour;
  Color high_textColour;
  Color entry_textColour;
  Color entry_backColour;
  int entriesInBox;
  Font textFont;
  private int itemSelected;
  private int itemHighlighted;
  int rememberItemSelected;
  int line_height;
  boolean show_slider;
  byte selection_mode;
  int click_function;
  GSlider gs;
  GListHeader glh;
  
  public final static byte SINGLE_SELECTION=1;
  public final static byte MULTI_SELECTION=2;
  public GSlider getSlider() { return gs; }
  public void setTextColour(Color c) { textColour = new Color(c.getRGB()); }
  public void setHighlightedTextColour(Color c) { high_textColour = new Color(c.getRGB()); }
  public void setEntryTextColour(Color c) { entry_textColour = new Color(c.getRGB()); }
  public void setEntryBackColour(Color c) { entry_backColour = new Color(c.getRGB()); }
  public void setFont(Font f) { textFont=f; }
  public void setGListHeader(GListHeader _glh) { glh = _glh; }
  public int getItemHighlighted() { return itemHighlighted; }
  public void setItemHighlighted(int i) { itemHighlighted=i;
    if (itemHighlighted<0) itemHighlighted=0;
    if (itemHighlighted>=countEntries()) itemHighlighted=countEntries()-1;
  }
  
  public boolean isSelected(int i) {
    if (selection_mode==SINGLE_SELECTION) return (itemSelected==i);
    else return itemsSelected.get(i);
  }
  
  public int countSelection() {
    int j=0;
    for (int i=0; i<itemsSelected.size(); i++) if (itemsSelected.get(i)) j++;
    return j;
  }
  
  public int[] getSelectedIndexes() {
    int count = countSelection();
    int[] indexes = new int[count];
    int i=0;
    int j=0;
    while (j<count) {
      if (itemsSelected.get(i)) indexes[j++]=i;
      i++;
    }
    return indexes;
  }
  
  public int getSelected() {
    if (selection_mode==SINGLE_SELECTION) return itemSelected;
    else {
      int i=0;
      while (i<itemsSelected.size()) {
        if (itemsSelected.get(i)) return i;
        else i++;
      }
      return -1;
    }
  }
  
  public void clearEntries() { 
    entries.clear();
    itemsSelected.clear();
    if (gs!=null) gs.setValue(0);
  }
  
  public void clearSelection() {
    itemSelected=-1;
    for (int i=0; i<itemsSelected.size(); i++) itemsSelected.set(i,new Boolean(false));
  }
  
  public int countEntries() { return entries.size(); }
  public String getEntry(int i) { return entries.get(i); }
  public void setSelected(int i) {
    if (selection_mode==SINGLE_SELECTION) itemSelected=i;
    else itemsSelected.set(i,new Boolean(true));
  }
  public void setEntry(int i, String s) { entries.set(i,s); }
  public void addEntry(String s) { 
    entries.add(s); 
    itemsSelected.add(new Boolean(false));       
    gs.setMax(Math.max(0,entries.size()-entriesInBox));
  }
  public void addEntry(int i, String s) { entries.add(i,s); itemsSelected.add(i,new Boolean(false)); }
  public void removeEntry(int i) { 
    entries.remove(i); 
    itemsSelected.remove(i);
    gs.setMax(Math.max(0,entries.size()-entriesInBox));
  }

  public GList(int _x, int _y, int _wid, int _hei, GContainer parent, String[] _entries, GWindow _gw, byte _selmode,int function_no) {
    super(parent,_gw);
    width=_wid;
    height=_hei;
    pos_x=_x;
    pos_y=_y;
    entries = new ArrayList<String>();
    itemsSelected = new ArrayList<Boolean>();
    if (_entries!=null) {
      for (int i=0; i<_entries.length; i++) {
        entries.add(_entries[i]);
        itemsSelected.add(new Boolean(false));
      }
    }
    textColour = new Color(gw.gcs.list_text);
    high_textColour = new Color(gw.gcs.list_high_text);
    entry_textColour = new Color(gw.gcs.list_entry_text);
    entry_backColour = new Color(gw.gcs.list_entry_back);
    textFont=gw.gcs.list_font;
    gs = new GSlider(pos_x+width-13,pos_y+4,height-8,GSlider.SLIDE_VERT,parent,0,Math.max(0,entries.size()-1),0,0,gw);
    gs.owner=this;
    parent.addChild(gs);
    while ((gs.getValue()>0) && (gs.getValue()+entriesInBox>entries.size()-1)) gs.setValue(gs.getValue()-1);
    selection_mode=_selmode;
    click_function=function_no;
  }
  
  public GComponent mouseExit(GComponent next) {
    if (next instanceof GSlider) {
      GSlider _gs = (GSlider) next;   // Moved from glist to gslider - check they're not brothers.
      if (_gs.owner!=this) {
        highlight=false;
        if (gs!=null) gs.highlight=false;
      }
    } else {
      itemHighlighted=-1;
      highlight=false;
      if (gs!=null) gs.highlight=false;
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();    
    return next;
  }
 

  public void mouseWithin(int x, int y) {
    FontMetrics fm = gw.g2d.getFontMetrics(textFont);
    line_height=fm.getHeight()+2;
    int par_x=0;
    int par_y=0;
    if (parent!=null) {
      par_x = parent.get_parent_pos_x();
      par_y = parent.get_parent_pos_y();
    }
    if ((gs==null) || (x<pos_x+par_x+width-(gs.width+3))) {
      int index = ((y-(pos_y+par_y+5))/line_height)+gs.getValue();
      if (index<0) index=0;
      if (index>=entries.size()) index=entries.size()-1;
      if (selection_mode==SINGLE_SELECTION) itemSelected=index;
      else itemHighlighted=index;
    } 
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();  
  }
  /*
  public boolean overSlider(int x, int y, GContainer container) {
    boolean over=false;
    if (gs!=null) {
      int par_x=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
      }
      
      if (x>par_x+pos_x+width-13) over=true;
    }
    return over;
  }
  */
  public GComponent mouseEnter(int x, int y, GComponent previous) {
    highlight=true;
    if (gs!=null) gs.highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void selectionEvent(int x, int y, MouseEvent e) {

    int par_y=0;
    if (parent!=null) {
      par_y = parent.get_parent_pos_y();
    }
    if (selection_mode==SINGLE_SELECTION) {
      int index = ((y-(pos_y+par_y+5))/line_height)+gs.getValue();
      if (index<0) index=0;
      if (index>=entries.size()) index=entries.size()-1;
      setSelected(index);
      if (glh!=null) {
        glh.setVisible(true);
        setVisible(false);
        gw.ga.doFunction(glh.select_function,glh);
        gw.modalComponent=null;
      } 
      
    } else {
      int index = ((y-(pos_y+par_y+5))/line_height)+gs.getValue();
      if (index<0) index=0;
      if (index>=entries.size()) index=entries.size()-1;
      if (e.isControlDown()) itemsSelected.set(index,new Boolean(!itemsSelected.get(index)));
      else if (e.isShiftDown()) {
        int first=index-1;
        while ((first>=0) && (!itemsSelected.get(first))) first--;
        int last=index+1;
        while ((last<itemsSelected.size()) && (!itemsSelected.get(last))) last++;
        if (first<0) first=index;
        if (last>=itemsSelected.size()) last=index;
        for (int i=first; i<=last; i++) {
          itemsSelected.set(i,new Boolean(true));
        }
      } else {
        for (int i=0; i<itemsSelected.size(); i++) {
          if (i==index) itemsSelected.set(i,new Boolean(true));
          else itemsSelected.set(i,new Boolean(false));
        }
      }
    }
    gw.ga.doFunction(click_function,this);
    if (glh!=null) {
      if (this.itemSelected==this.gs.getValue()) glh.setJustClicked(true); // Catch an erroneous double click detection
    }
  }
  
  public void modalMouseClick(int x, int y, MouseEvent e) {
    selectionEvent(x,y,e);
    if (parent instanceof GFilePanel) {
      ((GFilePanel)parent).clickList(e,this);
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  public void abort() {
    itemSelected=rememberItemSelected;
    modalMouseClick(0,0,null);
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    selectionEvent(x,y,e);
    if (parent instanceof GFilePanel) {
      ((GFilePanel)parent).clickList(e,this);
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  public void mouseWheel(MouseWheelEvent e) {
    int delta = e.getWheelRotation();
    gs.step_delta(delta);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint(); 
  }
  
  public void keyPress(KeyEvent e) {
    if (e.getKeyCode()==KeyEvent.VK_DOWN) {
      itemHighlighted++;
      if (itemHighlighted>=entries.size()) itemHighlighted=entries.size()-1;
      if (itemHighlighted>=(gs.getValue()+entriesInBox)) gs.setValue(gs.getValue()+1);
    
    } else if (e.getKeyCode()==KeyEvent.VK_UP) {
      itemHighlighted--;
      if (itemHighlighted<0) itemHighlighted=0;
      if (entries.size()==0) itemHighlighted=-1;
      else if (itemHighlighted<gs.getValue()) gs.setValue(gs.getValue()-1);
    
    } else if (e.getKeyCode()==KeyEvent.VK_PAGE_DOWN) {
      itemHighlighted+=entriesInBox;
      if (itemHighlighted>=entries.size()) itemHighlighted=entries.size()-1;
      if (itemHighlighted>=(gs.getValue()+entriesInBox)) {
        gs.setValue(gs.getValue()+entriesInBox);
        if (gs.getValue()+entriesInBox>entries.size()) gs.setValue(entries.size()-entriesInBox);
      }
    
    } else if (e.getKeyCode()==KeyEvent.VK_PAGE_UP) {
      itemHighlighted-=entriesInBox;
      if (itemHighlighted<0) itemHighlighted=0;
      if (entries.size()==0) itemHighlighted=-1;
      else if (itemHighlighted<gs.getValue()) {
        gs.setValue(gs.getValue()-entriesInBox);
        if (gs.getValue()<0) gs.setValue(0);
      }
      
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      FontMetrics fm = g.getFontMetrics(textFont);
      line_height=fm.getHeight()+2;
      entriesInBox=(int) Math.floor(height/line_height);
      gs.updateSliderSize(entries.size(),entriesInBox);
      int topEntry=0;
      if (gs!=null) topEntry=gs.getValue();
      while ((topEntry>0) && (topEntry+entriesInBox>entries.size())) topEntry--; 
      gs.setBigJump(entriesInBox);
      gs.setMax(Math.max(0,entries.size()-entriesInBox));
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
      g.setColor(backColour);
      g.fillRect(par_x+pos_x-2,par_y+pos_y-2,width+4,height+4);
      g.setColor(highlight?high_backColour:backColour);
      
      RoundRectangle2D border = new RoundRectangle2D.Float(par_x+pos_x,par_y+pos_y,width,height,10,10);
      g.fill(border);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      if (gs!=null) {
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.drawLine(pos_x+par_x+width-17,(pos_y+par_y),pos_x+par_x+width-17,((pos_y+par_y)+height));
        gs.paintOn(bi,g);
      }
      g.setColor(highlight?high_edgeColour:edgeColour);
      g.draw(border);
      g.setFont(textFont);

      for (int i=gs.getValue(); i<gs.getValue()+entriesInBox; i++) {
        if ((i>=0) && (i<entries.size())) {
          g.setColor(highlight?high_textColour:textColour);
          String s="";
          if (i<entries.size()) s = entries.get(i);
          while (fm.stringWidth(s)>width-20) s=s.substring(0,s.length()-1);
          
          if (((selection_mode==SINGLE_SELECTION) && (i==itemSelected)) || 
              ((selection_mode==MULTI_SELECTION) && (itemsSelected.get(i)))) {
            g.setColor(entry_backColour);
            if (gs==null) g.fillRect(par_x+pos_x+2,((par_y+pos_y+2))+((i-topEntry)*line_height),width-2,line_height);
            else g.fillRect(par_x+pos_x+2,((par_y+pos_y+2))+((i-topEntry)*line_height),width-(8+gs.width),line_height);
            
          }
          
          if (i==itemHighlighted) {
            g.setColor(entry_backColour);
            if (gs==null) g.drawRect(par_x+pos_x+2,((par_y+pos_y+2))+((i-topEntry)*line_height),width-2,line_height);
            else g.drawRect(par_x+pos_x+2,((par_y+pos_y+2))+((i-topEntry)*line_height),width-(9+gs.width),line_height);
          }
          
          
          if (i<entries.size()) {
            g.setColor(entry_textColour);
            int y = (par_y+pos_y-4)+(((i+1)-topEntry)*line_height);
            if (y>=par_y+pos_y) {
              g.drawString(s,par_x+pos_x+5,y);
            }

          }
        }
      }
      
      
    }
  }
}

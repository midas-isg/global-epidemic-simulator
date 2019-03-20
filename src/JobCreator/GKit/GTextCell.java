/* GTextCell.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: a borderless text cell - used in GTextEntry, GTable, GLabel 
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

/* 
*/

package GKit;

import java.awt.Color;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.StringSelection;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.UnsupportedFlavorException;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;
import java.io.IOException;


public class GTextCell extends GComponent {
  int sel_start,sel_end;
  int edit_function;
  StringBuffer data;
  String remember_orig;
  Color textColour,selectedColour;
  GNotifier notify;
  boolean allow_edit;
  boolean allow_cell_selection;
  boolean cell_selected;
  
  int buf_width;                    // X-size for off-screen buffer.
  int caret_pix_x,caret_ch_x;       // Caret position for editing (pixel no. and character no.)
  boolean editing;                  // Are we editing this?
  int edit_clip_left;
  
  static String SPACERS = new String(" ,./\\;:\t'\"");
  public void setValue(String val,boolean repaint) {   
    data.setLength(0); 
    data.append(val); 
    if (repaint) paintOn(gw.bi,gw.g2d); 
    gw.requestRepaint();
  }
  public String getValue() { return new String(data); }
  public void setTextColour(Color c) { textColour=new Color(c.getRGB()); }
  public void setSelectedColour(Color c) { selectedColour = new Color(c.getRGB()); }

  public void clearSelection() {
    sel_start=-1;
    sel_end=-1;
  }
  
  public void setSelection(int start, int end) {
    sel_start=start;
    sel_end=end;
  }
  
  protected GTextCell(int _x, int _y, int _wid, int _hei, GContainer parent, GWindow _gw, int function_no,String dat) {
    super(parent,_gw);
    sel_start=-1;
    sel_end=-1;
    width=_wid;
    height=_hei;
    pos_x=_x;
    pos_y=_y;
    edit_function=function_no;
    edit_clip_left=0;
    data=new StringBuffer();
    setValue(dat,false);
    setTextColour(new Color(_gw.gcs.table_fore));
    setBackColour(new Color(_gw.gcs.table_back));
    setHighlightBackColour(new Color(_gw.gcs.table_title_back));
    initTempBuffer(1000);
    allow_edit=true;
    allow_cell_selection=false;
    
  }
  
  public void initTempBuffer(int wid) {
    if (wid>buf_width) {
      gw.tcell_bi=null;
    }
    buf_width=wid;
    if (gw.tcell_bi==null) {
      gw.tcell_bi = new BufferedImage(wid,50,BufferedImage.TYPE_3BYTE_BGR);
      gw.tcd = (Graphics2D) gw.tcell_bi.getGraphics();
      gw.tcd.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      gw.tcd.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
      gw.tcd.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
      gw.tcd.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
      gw.tcd.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
      gw.tcd.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
      gw.tcd.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
      gw.tcd.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,RenderingHints.VALUE_STROKE_NORMALIZE);
    }
  }
  
  public void renderCell(int paste_x, int paste_y, int clip_top,int clip_left,int clip_bottom, int clip_right, FontMetrics fm, BufferedImage orig) {
    if (fm.stringWidth(String.valueOf(data))>buf_width-50) initTempBuffer(fm.stringWidth(String.valueOf(data))+100);
    if (editing) renderCellEdit(paste_x,paste_y,clip_top,clip_bottom,fm,orig);
    gw.tcd.setFont(GWindow.TFIELD_FONT);
    if (cell_selected) gw.tcd.setColor(selectedColour);
    else gw.tcd.setColor(backColour);
    gw.tcd.fillRect(0,0,width,height);
    gw.tcd.setColor(textColour);
    int y_plot=height-((height-fm.getAscent())/2)-3;
    gw.tcd.drawString(data.toString(),4,y_plot);
    for (int i=clip_left; i<width-clip_right; i++) {
      for (int j=clip_top; j<height-clip_bottom; j++) {
        orig.setRGB(paste_x+i-clip_left,paste_y+j-clip_top,gw.tcell_bi.getRGB(i,j));
      }
    }
  }
  
  public void renderCellEdit(int paste_x, int paste_y, int clip_top,int clip_bottom, FontMetrics fm, BufferedImage orig) {
    if (fm.stringWidth(String.valueOf(data))>buf_width-50) initTempBuffer(fm.stringWidth(String.valueOf(data))+100);
    String s = data.substring(0,Math.min(caret_ch_x,data.length()));
    gw.tcd.setFont(GWindow.TFIELD_FONT);
    gw.tcd.setColor(backColour);
    gw.tcd.fillRect(0,0,buf_width,height);
    gw.tcd.setColor(textColour);
    int y_plot=height-((height-fm.getAscent())/2)-3;
    
    int caret_x = 4+(int) (fm.getStringBounds(s, gw.tcd).getWidth());
        
    if (caret_x-edit_clip_left>(width-2)) {
      edit_clip_left=Math.max(0,(caret_x-width));
    }
    if (caret_x<edit_clip_left) {
      edit_clip_left=caret_x;
    }
    gw.tcd.drawString(data.toString(),4,y_plot);
    
    
    if ((sel_start>=0) && (sel_start!=sel_end)) {
      int sel_start_pix=4+fm.stringWidth(data.substring(0,Math.min(sel_start,sel_end)));
      int sel_end_pix=4+fm.stringWidth(data.substring(0,Math.max(sel_start,sel_end)));
      for (int i=sel_start_pix; i<=sel_end_pix; i++) {
        for (int j=clip_top; j<height-clip_bottom; j++) {
          Color c = new Color(gw.tcell_bi.getRGB(i, j));
          Color d = new Color(256-c.getRed(),256-c.getBlue(),256-c.getGreen());
          gw.tcell_bi.setRGB(i, j, d.getRGB());
          
        }
      }
    }
    
    gw.tcd.drawLine(caret_x,2,caret_x,height-2);
    gw.tcd.drawLine(caret_x-1,2,caret_x+1,2);
    gw.tcd.drawLine(caret_x-1,height-2,caret_x+1,height-2);
    
    
    int i=0,j=0;
    try {
      for (i=edit_clip_left; i<width+edit_clip_left; i++) {
        for (j=clip_top; j<height-clip_bottom; j++) {
          int rgb = gw.tcell_bi.getRGB(i,j);
          orig.setRGB(paste_x+i-edit_clip_left,paste_y+j-clip_top,rgb);
        }
      }
    } catch (Exception e) { e.printStackTrace(); }
  }

  
  
  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      FontMetrics fm = g.getFontMetrics(GWindow.TFIELD_FONT);
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }
      if (editing) renderCellEdit(par_x+pos_x,par_y+pos_y,0,0,fm,bi);
      else renderCell(par_x+pos_x,par_y+pos_y,0,0,0,0,fm,bi);
    }
  }

  public GComponent mouseExit(GComponent next) {
    if (next instanceof GSlider) {
      GSlider _gs = (GSlider) next;   // Moved from glist to gslider - check they're not brothers.
      if (_gs.owner!=this) {
        highlight=false;
      }
    } else {
      highlight=false;
    }
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();    
    return next;
  }


  public void mouseWithin(int x, int y) {
  }
  
  public void acceptEntry() {
    editing=false;
    gw.modalComponent=null;
    gw.ga.doFunction(edit_function,this);
    if (notify!=null) notify.notifyMe(this);
    sel_start=-1;
    sel_end=-1;
    edit_clip_left=0;
  }

  public GComponent mouseEnter(int x, int y, GComponent previous) {
    highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    if (!e.isShiftDown()) {
      sel_start=-1;
      sel_end=-1;
    }
    
    if ((!editing) && (allow_edit)) {
      editing=true;
      remember_orig=data.toString();
      gw.modalComponent=this;
    }
    FontMetrics fm = gw.g2d.getFontMetrics(GWindow.TFIELD_FONT);
    int par_x=0;
    if (parent!=null) par_x = parent.get_parent_pos_x();
    int pix_hit = edit_clip_left+x-(pos_x+par_x);
    int ch=0;
    while ((ch<=data.length()) && (fm.getStringBounds(data.substring(0,ch),gw.g2d).getWidth()<pix_hit)) ch++;
    if (e.isShiftDown() && (sel_start==-1) && (caret_ch_x>=0)) sel_start=caret_ch_x;
    caret_ch_x=ch-1;
    if (e.isShiftDown()) {
      if (sel_start==-1) sel_start=caret_ch_x;
      sel_end=caret_ch_x;
    }
  }
  
  public void dragStart(MouseEvent e) {
    FontMetrics fm = gw.g2d.getFontMetrics(GWindow.TFIELD_FONT);
    int par_x=0;
    if (parent!=null) par_x = parent.get_parent_pos_x();
       
    int pix_hit = edit_clip_left+e.getX()-(pos_x+par_x);
    int ch=0;
    while ((ch<=data.length()) && (fm.getStringBounds(data.substring(0,ch),gw.g2d).getWidth()<pix_hit)) ch++;
    sel_start=ch-1;
    sel_end=ch-1;
    caret_ch_x=ch-1;
    paintOn(gw.bi,gw.g2d);
    gw.repaint();
  }
  
  public void dragSelect(MouseEvent e) {
   
    FontMetrics fm = gw.g2d.getFontMetrics(GWindow.TFIELD_FONT);
    int par_x=0;
    if (parent!=null) par_x = parent.get_parent_pos_x();
    if ((e.getX()>=par_x+pos_x) && (e.getX()<=par_x+pos_x+width)) {
      int pix_hit = edit_clip_left+e.getX()-(pos_x+par_x);
      int ch=0;
      while ((ch<=data.length()) && (fm.getStringBounds(data.substring(0,ch),gw.g2d).getWidth()<pix_hit)) ch++;
      sel_end=ch-1;
    } else if (e.getX()<=par_x+pos_x) sel_end--;
    else if (e.getX()>par_x+pos_x+width) sel_end++;
    
    if (sel_end<0) sel_end=0;
    if (sel_end>data.length()) sel_end=data.length();
    caret_ch_x=sel_end;
    paintOn(gw.bi,gw.g2d);
    gw.repaint();
  }

  public void mouseWheel(MouseWheelEvent e) {
  }
  
  public String getClipboardContents() {
    String result = "";
    Clipboard clipboard = Toolkit.getDefaultToolkit().getSystemClipboard();
    //odd: the Object param of getContents is not currently used
    Transferable contents = clipboard.getContents(null);
    boolean hasTransferableText =
      (contents != null) &&
      contents.isDataFlavorSupported(DataFlavor.stringFlavor)
    ;
    if ( hasTransferableText ) {
      try {
        result = (String)contents.getTransferData(DataFlavor.stringFlavor);
      }
      catch (UnsupportedFlavorException ex){
        //highly unlikely since we are using a standard DataFlavor
        System.out.println(ex);
        ex.printStackTrace();
      }
      catch (IOException ex) {
        System.out.println(ex);
        ex.printStackTrace();
      }
    }
    return result.trim();
  }
  
  public void setClipboardContents(String s) {
    StringSelection stringSelection = new StringSelection(s);
    Toolkit.getDefaultToolkit().getSystemClipboard().setContents(stringSelection, null);
  }
 


  
  public void deleteSelection() {
    if (sel_start!=sel_end) data.delete(Math.min(sel_start,sel_end),Math.max(sel_start,sel_end));
    caret_ch_x=Math.min(sel_start,sel_end);
    sel_start=-1;
    sel_end=-1;
  }
  
  public void keyPress(KeyEvent e) {
    if (editing) {
      int key=e.getKeyCode();
     
      if (key==KeyEvent.VK_ESCAPE) {
        data.setLength(0);
        data.append(remember_orig);
        editing=false;
        gw.modalComponent=null;
        sel_start=-1;
        sel_end=-1;
      
      } else if (key==KeyEvent.VK_ENTER) {
        acceptEntry();
        sel_start=-1;
        sel_end=-1;
      
      } else if (key==KeyEvent.VK_RIGHT) {
        if (!e.isShiftDown()) { sel_start=-1; sel_end=-1; }
        if ((e.isShiftDown()) && (sel_start==-1)) {
          sel_start=caret_ch_x;
          sel_end=caret_ch_x;
        }
        if (e.isControlDown()) {
          if (caret_ch_x<data.length()) caret_ch_x++;
          while (caret_ch_x<data.length() && (SPACERS.indexOf(data.charAt(caret_ch_x))==-1)) caret_ch_x++;
        }
        caret_ch_x=Math.min(data.length(),caret_ch_x+1);
        if (e.isShiftDown()) {
          sel_end=caret_ch_x;
        }
      }
      else if (key==KeyEvent.VK_LEFT) {
        if (!e.isShiftDown()) { sel_start=-1; sel_end=-1; }
        if ((e.isShiftDown()) && (sel_start==-1)) {
          sel_start=caret_ch_x;
          sel_end=caret_ch_x;
        }

        if (e.isControlDown()) {
          if (caret_ch_x>0) caret_ch_x--;
          while ((caret_ch_x>0) && (SPACERS.indexOf(data.charAt(caret_ch_x))==-1)) caret_ch_x--;
        } else caret_ch_x=Math.max(0,caret_ch_x-1);
        if (e.isShiftDown()) {
          sel_end=caret_ch_x;
        }
      }
      else if (key==KeyEvent.VK_HOME) {
        if (!e.isShiftDown()) { sel_start=-1; sel_end=-1; }
        if (e.isShiftDown()) {
          if (sel_start==-1) {
            sel_start=caret_ch_x;
            sel_end=0;
          } else {
            sel_end=0;
          }
        }
        caret_ch_x=0;
      }
      
      else if (key==KeyEvent.VK_END) {
        if (!e.isShiftDown()) { sel_start=-1; sel_end=-1; }
        if (e.isShiftDown()) {
          if (sel_start==-1) {
            sel_start=caret_ch_x;
            sel_end=data.length();
          } else sel_end=data.length();
        }
        caret_ch_x=data.length();
      }
        
      else if (key==KeyEvent.VK_DELETE) {
        if (sel_start>-1) deleteSelection();
        else if (caret_ch_x<data.length()) data.deleteCharAt(caret_ch_x);
      }
      
      else if (key==KeyEvent.VK_BACK_SPACE) {
        if ((sel_start!=-1) && (sel_start!=sel_end)) {
         deleteSelection();
        
        } else {
          if (caret_ch_x>0) {
            data.deleteCharAt(caret_ch_x-1);
            caret_ch_x--;
          }
        }
      }
        
      else if ((key==KeyEvent.VK_V) && (e.isControlDown())) {
        String s = getClipboardContents();
        if (caret_ch_x>=data.length()) {
          data.append(s);
          caret_ch_x+=s.length();
        }
        else {
          data.insert(caret_ch_x,s);
          caret_ch_x+=s.length();
        }
      
      
      } else if (((key==KeyEvent.VK_C) || (key==KeyEvent.VK_X)) && (e.isControlDown())) {
        if (sel_start>-1) {
          String s = data.substring(Math.min(sel_start,sel_end),Math.max(sel_start,sel_end));
          setClipboardContents(s);
          if (key==KeyEvent.VK_X) {
            data.delete(Math.min(sel_start,sel_end),Math.max(sel_start,sel_end));
            sel_start=-1;
            sel_end=-1;
          }
        }
      }
        
        
      else {
        if (sel_start>-1) {
          deleteSelection();
        }
        
        char ch = e.getKeyChar();
        if (((int)ch<256) && ((int)ch>31)) {
          if (caret_ch_x>=data.length()) {
            data.append(e.getKeyChar());
            caret_ch_x++;
          }
          else {
            data.insert(caret_ch_x,e.getKeyChar());
            caret_ch_x++;
          }
        }
      }
        
        
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    }
  }
}

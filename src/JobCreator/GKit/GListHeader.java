/* GListHeader.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A one-line gui component always linked with a GList. 
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
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;


public class GListHeader extends GComponent {
  GList gl;
  int box_width;
  Color textColour;
  Color triColour;
  Color high_triColour;
  
  public void setTextColour(Color c) { textColour=new Color(c.getRGB()); }
  public void setTriColour(Color c) { triColour=new Color(c.getRGB()); }
  public void setHighTriColour(Color c) { high_triColour=new Color(c.getRGB()); }
  public void setJustClicked(boolean b) {just_clicked=b; }
  
  boolean highlight_triangle;
  boolean highlight_text;
  boolean just_clicked=false;
  boolean editing=false;
  int select_function;
  
  public GListHeader(int _pos_x,int _pos_y, int _box_width, GContainer _parent, GWindow _gw, GList _gl, int _selfunc) {
    super(_parent,_gw);
    select_function=_selfunc;
    textColour = new Color(gw.gcs.list_entry_text);
    box_width=_box_width;
    triColour = new Color(gw.gcs.tri);
    high_triColour = new Color(gw.gcs.htri);
    gl=_gl;
    gl.setGListHeader(this);
    pos_x=_pos_x;
    pos_y=_pos_y;
    height=22;
    width=box_width;
    mouse_over=false;
  }
  
  public GComponent mouseExit(GComponent next) {
    highlight_triangle=false;
    highlight_text=false;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return next;
  }
  
  public void mouseWithin(int x, int y) {
    int par_x=0;
    if (parent!=null) par_x = parent.get_parent_pos_x();
    boolean h = (x>=pos_x+par_x+width-17);
    if (highlight_triangle!=h) {
      highlight_triangle=h && enabled;
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    }
  }
  
  public GComponent mouseEnter(int x, int y, GComponent previous) {
    int par_x=0;
    if (parent!=null) par_x = parent.get_parent_pos_x();
    highlight_text=true;
    highlight_triangle=(enabled) && (x>=pos_x+par_x+width-17);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    if (enabled) {
      if (!just_clicked) {
        gl.rememberItemSelected=gl.getSelected();
        setVisible(false);
        gl.setVisible(true);
        gw.modalComponent=gl;
        paintOn(gw.bi,gw.g2d);
        gl.paintOn(gw.bi,gw.g2d);
        gw.requestRepaint();
      }
      just_clicked=false;
    }
  }
  
  public void mouseWheel(MouseWheelEvent e) {}
  public void keyPress(KeyEvent e) {}
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
      g.setColor(backColour);
      g.fillRect(par_x+pos_x-2,par_y+pos_y-2,box_width+4,26);
      RoundRectangle2D rr2d = new RoundRectangle2D.Float(par_x+pos_x,(par_y+pos_y),box_width,22,5,5);
      g.setColor(highlight_text?high_backColour:backColour);
      g.fill(rr2d);
      g.setColor(highlight_text?high_edgeColour:edgeColour);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      g.draw(rr2d);
      g.drawLine(par_x+pos_x+width-17,par_y+pos_y,par_x+pos_x+width-17,par_y+pos_y);
      Polygon triangle = new Polygon();
      triangle.addPoint(par_x+pos_x+width-14,(pos_y+par_y)+6);
      triangle.addPoint(par_x+pos_x+width-4,(pos_y+par_y)+6);
      triangle.addPoint(par_x+pos_x+width-9,(pos_y+par_y)+15);
      g.setColor(highlight_triangle?high_triColour:triColour);
      g.fill(triangle);
      g.setColor(highlight_triangle?high_edgeColour:edgeColour);
      g.draw(triangle);
      g.setFont(GWindow.TFIELD_FONT);
      if (!editing) {
        g.setColor(textColour);
        if ((gl.getSelected()>=0) && (gl.countEntries()>=1) && (gl.getSelected()<gl.countEntries())) {
          String s = gl.getEntry(gl.getSelected());
          FontMetrics fm = g.getFontMetrics(GWindow.TFIELD_FONT);
          while (fm.stringWidth(s)>width-20) s=s.substring(0,s.length()-1);
          g.drawString(s,par_x+pos_x+5,((par_y+pos_y))+17);
        }
      }
    }
  }
  

}

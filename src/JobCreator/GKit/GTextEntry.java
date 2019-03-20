/* GTextEntry.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: a text cell inside a box. 
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
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;


public class GTextEntry extends GComponent implements GNotifier {
  int box_width;
  GTextCell gtc;
  public Color textColour;
  int edit_function;
  
  public void setTextColour(Color c) { textColour = new Color(c.getRGB()); }
  
  public void notifyMe(Object component) {
    editFunction(component);
  }
  
  public void editFunction(Object component) {
    gw.ga.doFunction(edit_function, gtc);
  }
  
  public GTextEntry(int _pos_x,int _pos_y, int _box_width, GContainer _parent, GWindow _gw,String s,int _function) {
    super(_parent,_gw);
    box_width=_box_width;
    pos_x=_pos_x;
    pos_y=_pos_y;
    height=22;
    width=box_width;
    mouse_over=false;
    textColour = new Color(gw.gcs.list_entry_text);
    edit_function=_function;
    gtc=new GTextCell(_pos_x+3,_pos_y+3,_box_width-6,height-6,_parent,_gw,_function,s);
    gtc.notify=this;
    gtc.setBackColour(backColour);
    gtc.setTextColour(textColour);

  }
  
  public String getText() { return new String(gtc.getValue()); }
  
  public void setText(String s) { gtc.setValue(s,isVisible()); }
  
  public GComponent mouseExit(GComponent next) {
    if (!gtc.editing) {
      highlight=false;
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
      return next;
    } else {
      return this;   
    }
  }
  
  public void mouseWithin(int x, int y) {
    gtc.mouseWithin(x, y);
  }
  
  public void mouseWheel(MouseWheelEvent e) {
    gtc.mouseWheel(e);
  }
  
  public void keyPress(KeyEvent e) {
    gtc.keyPress(e);
    if (!gtc.editing) {
      gw.modalComponent=null;
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
      gw.ga.setUnSaved(true);
    }
  }
  
  
  public GComponent mouseEnter(int x, int y, GComponent previous) {
    if (!gtc.editing) {
      boolean overide=false;
      if ((previous!=null) && (previous instanceof GTextEntry)) {
        GTextEntry lge = (GTextEntry) previous;
        if (lge.gtc.editing) overide=true;
      }
      if (!overide) {
        highlight=true;
        paintOn(gw.bi,gw.g2d);
        gw.requestRepaint();
      }
    }
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    gtc.mouseClick(x, y, e);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }
      
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
      g.setColor(backColour);
      g.fillRect(par_x+pos_x-2,par_y+pos_y-2,width+4,height+4);
       RoundRectangle2D rr2d = new RoundRectangle2D.Float(par_x+pos_x,(par_y+pos_y),box_width,height,5,5);
      g.setColor(highlight?high_backColour:backColour);
      g.fill(rr2d);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      gtc.paintOn(bi,g);
      g.setColor(highlight?high_edgeColour:edgeColour);
      g.draw(rr2d);
    }
  }
}


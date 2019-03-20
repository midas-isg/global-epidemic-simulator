/* GTickBox.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: simple tick-box component.
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
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;

public class GTickBox extends GComponent {
  private boolean selected;
  int click_event;
   
  public void clicked(Object component) {
    gw.ga.doFunction(click_event, component);
  }
  public boolean isSelected() { return selected; }
  
  public void setSelected(boolean s) { 
    selected=s;
  }  
  
  public GComponent mouseExit(GComponent next) {
    highlight=false;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return next;
  }
  
  public void mouseWithin(int x, int y) {}
  public void mouseWheel(MouseWheelEvent e) {}
  public void keyPress(KeyEvent e) {}
  
  public GComponent mouseEnter(int x, int y, GComponent previous) {
    highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    clicked(this);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
  
  
  
  public GTickBox(int _pos_x,int _pos_y, GContainer _parent, GWindow _gw, int _click_event) {
    super(_parent,_gw);
    pos_x=_pos_x;
    pos_y=_pos_y;
    height=22;
    width=22;
    mouse_over=false;
    selected=true;
    click_event=_click_event;
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
      RoundRectangle2D rr2d = new RoundRectangle2D.Float(par_x+pos_x,(par_y+pos_y),width,height,5,5);
      g.setColor(highlight?high_backColour:backColour);
      g.fill(rr2d);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      g.setColor(highlight?high_edgeColour:edgeColour);
      g.draw(rr2d);
      if (selected) g.drawImage(highlight?gw.tick_lit:gw.tick_dim,par_x+pos_x,((par_y+pos_y)),null);
      g.setFont(GWindow.TFIELD_FONT);
    }
  }
  
 
}



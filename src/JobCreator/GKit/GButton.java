/* GButton.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A clickable, non-toggling button component. 
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
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;

public class GButton extends GComponent {
  String data;
  int click_function;
  Color textColour;
  boolean show_icon;
  BufferedImage image_lit,image_dim;
  
  
  public void setIcon(BufferedImage lit,BufferedImage dim) {
    image_lit=lit;
    image_dim=dim;
    show_icon=(lit!=null);
    paintOn(gw.bi,gw.g2d);
  }
  
  public BufferedImage getLitIcon() { return image_lit; }
  public BufferedImage getDimIcon() { return image_dim; }
  
  public void setShowIcon(boolean b) {
    show_icon=b;
  }
  
  public String getText() {
    return data;
  }
  
  public void setText(String s) {
    data=new String(s);
  }
  public void setTextColour(Color c) { textColour=new Color(c.getRGB()); }
  
  public GButton(int _pos_x,int _pos_y, int _width, int _height, GContainer _parent, GWindow _gw, int _click_function,String text) {
    super(_parent,_gw);
    pos_x=_pos_x;
    pos_y=_pos_y;
    width=_width;
    height=_height;
    click_function=_click_function;
    mouse_over=false;
    textColour=new Color(gw.gcs.text);
    data=new String(text);
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
    if (parent instanceof GFilePanel) {
      if (image_lit==gw.prev_lit) {
        ((GFilePanel) parent).previousDirectory();
      }
    }
    
    if (enabled) gw.ga.doFunction(click_function,this);
  }
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      int par_x=0;
      int par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }
      
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
      g.setColor(backColour);
      g.fillRect(par_x+pos_x-2,par_y+pos_y-2,width+4,height+4);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      
      RoundRectangle2D rr2d = new RoundRectangle2D.Float(par_x+pos_x,(par_y+pos_y),width,height,5,5);
      g.setColor(highlight&enabled?high_backColour:backColour);
      g.fill(rr2d);
      g.setColor(highlight&enabled?high_edgeColour:edgeColour);
      g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
      g.draw(rr2d);
      g.setFont(GWindow.TFIELD_FONT);
      g.setColor(textColour);
      FontMetrics fm = g.getFontMetrics();
      int textwidth=fm.stringWidth(data);
      int margin=(width-textwidth)/2;
      if (!show_icon) {
        g.drawString(data,par_x+pos_x+margin,((par_y+pos_y))+17);
      } else {
        if (highlight?(image_lit!=null):(image_dim!=null)) 
          g.drawImage(highlight?image_lit:image_dim,par_x+pos_x,((par_y+pos_y)),null);
      }
    }
  }
} 
 
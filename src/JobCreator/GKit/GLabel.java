/* GLabel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A text label (with events) 
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
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;


public class GLabel extends GComponent {
  public static final byte LEFT_ALIGN = 1;
  public static final byte CENTRE_ALIGN = 2;
  public static final byte RIGHT_ALIGN = 3;  
  public Font font;
  public String text;
  public Color textColour;
  public Color h_textColour;
  public byte align;
  int click_function;
  
  public void setFont(Font f) {
    font=f;
  }
  
  
  public void setText(String s) { text=new String(s); height=0; }
  public void setTextColour(Color c) { textColour = new Color(c.getRGB()); }
  public void setHighlightTextColour(Color c) { h_textColour = new Color(c.getRGB()); }  
  public GLabel(int _x, int _y, String _text, byte _align, int func, GContainer _parent,GWindow _gw) {
    super(_parent,_gw);
    pos_x=_x;
    pos_y=_y;
    text=new String(_text);
    textColour = new Color(gw.gcs.text);
    h_textColour = new Color(gw.gcs.list_high_text);
    align = _align;
    height=0;
    font=gw.gcs.label_font;
    click_function=func;
  }
  
  public GComponent mouseExit(GComponent next) {
    unPaint(gw.bi,gw.g2d);
    highlight=false;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return next; 
   }
  
  public void mouseWithin(int x, int y) {}
  
  public boolean contains(int x, int y) {
    int s_x=pos_x; int s_y=pos_y;
    if (parent!=null) {
      s_x+=parent.get_parent_pos_x();
      s_y+=parent.get_parent_pos_y();
    }
    if (align==LEFT_ALIGN) 
      return ((x>=s_x)&& (x<=s_x+width) && (y>=s_y) && (y<=s_y+height));
    else if (align==RIGHT_ALIGN)
      return ((x>=s_x-width)&& (x<=s_x) && (y>=s_y) && (y<=s_y+height));
    else if (align==CENTRE_ALIGN)
      return ((x>=s_x-(width/2))&& (x<=s_x+(width/2)) && (y>=s_y) && (y<=s_y+height));
    else return false;
  }

  
  public GComponent mouseEnter(int x, int y, GComponent previous) {
    unPaint(gw.bi,gw.g2d);
    highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    gw.ga.doFunction(click_function,e.getSource());
    
  }
  public void mouseWheel(MouseWheelEvent e) {}
  public void keyPress(KeyEvent e) {}

  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      g.setFont(font);
      g.setColor((highlight&&click_function>0)?h_textColour:textColour);
      
      if (height==0) {
        FontMetrics fm = g.getFontMetrics(font);
        width=fm.stringWidth(text);
        height=fm.getHeight()-7;
      }
      if (align>LEFT_ALIGN) {
        if (align==CENTRE_ALIGN) par_x-=(width/2);
        else if (align==RIGHT_ALIGN) par_x-=width;
        
      }
      g.drawString(text,par_x+pos_x,(par_y+pos_y+height));
    }
  }
  
  public void unPaint(BufferedImage bi, Graphics2D g) {
    int par_x=0,par_y=0;
    if (parent!=null) {
      par_x = parent.get_parent_pos_x();
      par_y = parent.get_parent_pos_y();
    }

    g.setColor(backColour);
    if (height==0) {
      FontMetrics fm = g.getFontMetrics(font);
      width=fm.stringWidth(text);
      height=fm.getHeight()-7;
    }
    if (align>LEFT_ALIGN) {
      if (align==CENTRE_ALIGN) par_x-=(width/2);
      else if (align==RIGHT_ALIGN) par_x-=width;
      
    }
    //g.drawString(text,par_x+pos_x,(par_y+pos_y+height));
    
    g.fillRect(par_x+pos_x,(par_y+pos_y),width+1,height+7);
  }
}

/* GVerticalLabel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: use specifically for 90-degree rotated text labels 
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


public class GVerticalLabel extends GComponent {
  public static final byte LEFT_ALIGN = 1;
  public static final byte CENTRE_ALIGN = 2;
  public static final byte RIGHT_ALIGN = 3;  
  public static final byte UP = 4;
  public static final byte DOWN = 5;
  public Font font;
  public String text;
  public Color textColour;
  public Color h_textColour;
  public byte align;
  int click_function;
  byte direction;
    
  public void setFont(Font f) {
    font=f;
  }
  
  
  public void setText(String s) { text=new String(s); }
  public void setTextColour(Color c) { textColour = new Color(c.getRGB()); }
  public void setHighlightTextColour(Color c) { h_textColour = new Color(c.getRGB()); }  
  public GVerticalLabel(int _x, int _y, String _text, byte _align, byte _direction,int func, GContainer _parent,GWindow _gw) {
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
    direction=_direction;
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
    int x_1=s_x;
    int x_2=x_1+height;
    int y_1=(direction==UP)?s_y-width:s_y;
    int y_2=y_1+width;
    if (align==CENTRE_ALIGN) {
      y_1+=(direction==UP)?(width/2):(-height/2);
      y_2+=(direction==UP)?(width/2):(-width/2);
    } else if (align==RIGHT_ALIGN) {
      y_1+=(direction==UP)?width:(-width);
      y_2+=(direction==UP)?width:(-width);
    }
    //gw.g2d.setColor(Color.YELLOW);
    //gw.g2d.drawRect(x_1,y_1,(x_2-x_1),(y_2-y_1));
    boolean b=((x>=x_1) && (x<=x_2) && (y>=y_1) && (y<=y_2));
    //if (b) System.out.println("b:"+x);
    return b;
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
        height=fm.getHeight()-4;
      }
      gw.t2d.setColor(backColour);
      gw.t2d.fillRect(0,0,gw.text_bi.getWidth(),gw.text_bi.getHeight());
      gw.t2d.setFont(font);
      gw.t2d.setColor((highlight&&click_function>0)?h_textColour:textColour);
      gw.t2d.drawString(text,0,height-2);
      int shift_y=0;
      if (align==CENTRE_ALIGN) shift_y=(width/2);
      else if (align==RIGHT_ALIGN) shift_y=width;
      
      if (direction==UP) {
        for (int i=0; i<width; i++) {
          for (int j=0; j<height+2; j++) {
            int col = gw.text_bi.getRGB(i,j);
            bi.setRGB((pos_x+par_x)+j-2,pos_y+par_y-i+shift_y,col);
          }
        }
      } else if (direction==DOWN) {
        for (int i=0; i<width; i++) {
          for (int j=0; j<height+2; j++) {
            int col = gw.text_bi.getRGB(i,j);
            bi.setRGB(pos_x+par_x-j,pos_y+par_y+i-shift_y,col);
          }
        }
      }
    }
    
  }
  
  public void unPaint(BufferedImage bi, Graphics2D g) {
    if (g!=null) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      g.setColor(backColour);
      if (height==0) {
        FontMetrics fm = g.getFontMetrics(font);
        height=fm.stringWidth(text);
        width=fm.getHeight()-4;
      }
    
      int shift_y=0;
      if (align==CENTRE_ALIGN) shift_y=-(height/2);
      else if (align==RIGHT_ALIGN) shift_y=-height;
      g.fillRect(pos_x+par_x+shift_y-2,pos_y+par_y-(width+1),height+4,width+1);
    }
    
  }
}

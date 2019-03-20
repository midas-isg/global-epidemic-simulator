/* GSlider.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Horizontal and Vertical slide bars 
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
import java.awt.Polygon;
import java.awt.RenderingHints;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;


public class GSlider extends GComponent {
  public static final byte SLIDE_HORIZ = 1;
  public static final byte SLIDE_VERT = 2;
  
  Color triColour;
  Color high_triColour;
  Color ringColour;
  Color high_ringColour;
  Color drag_ringColour;
  Color ballColour;
  Color high_ballColour;
  Color drag_ballColour;
  
  public void setTriColour(Color c) { triColour = new Color(c.getRGB()); }
  public void setHiTriColour(Color c) { high_triColour = new Color(c.getRGB()); }
  public void setRingColour(Color c) { ringColour = new Color(c.getRGB()); }
  public void setHighRingColour(Color c) { high_ringColour = new Color(c.getRGB()); }  
  public void setDragRingColour(Color c) { drag_ringColour = new Color(c.getRGB()); }  
  public void setBallColour(Color c) { ballColour = new Color(c.getRGB()); }
  public void setHighBallColour(Color c) { high_ballColour = new Color(c.getRGB()); }  
  public void setDragBallColour(Color c) { drag_ballColour = new Color(c.getRGB()); }  
  
  byte orientation;
  int size;
  private int max,min,value;
  int ball_width=1;
  int big_jump=0;
  int ball_pos=0;
  int start_drag_pos=0;
  boolean highlight_min=false;
  boolean highlight_max=false;
  boolean highlight_ball=false;
  private boolean dragging=false;
  GComponent owner;
  int function;
  
  public void setDragging(boolean b) { dragging=b; }
  public boolean isDragging() { return dragging; }
  public void setMax(int _max) { max=_max; if (value>max) value=max; }
  public int getMax() { return max; }
  public void setMin(int _min) { min=_min; if (value<min) value=min; }
  public int getMin() { return min; }
  public int getValue() { return value; }
  public int getSize() { return size; }
  
  
  public boolean isVisible() {
    if (owner==null) return super.isVisible();
    else return super.isVisible() && owner.isVisible();
  }
  
  public GComponent mouseExit(GComponent next) {
    if (!dragging) {
      highlight_max=false;
      highlight_min=false;
      highlight_ball=false;
      if (owner!=null) {
        if (owner instanceof GList) {
          GList gl = (GList) owner;
          if (next instanceof GList) {
            GList gl2 = (GList) next;
            if (gl2!=gl) gl.highlight=false;
          } else gl.mouseExit(next);
        
        } else if (owner instanceof GTable) {
          GTable gt = (GTable) owner;
          if (next instanceof GTable) {
            GTable gt2 = (GTable) next;
            if (gt!=gt2) gt.highlight=false;
          } else gt.mouseExit(next);
        }
      }
      paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
      

    }
    return next;
  }
  
  public void mouseWithin(int x, int y) {
    if (owner!=null) owner.mouseWithin(x,y);
    mouseEnter(x,y,null);
    
  }
  public GComponent mouseEnter(int x, int y, GComponent previous) {
     boolean over_thin=false;
    int s_x=pos_x,s_y=pos_y;
    if (parent!=null) {
      s_x += parent.get_parent_pos_x();
      s_y += parent.get_parent_pos_y();
    }
    if (orientation==GSlider.SLIDE_HORIZ) {
      over_thin=((y>s_y) && (y<s_y+size*2));
      highlight_min=over_thin && (x<s_x+(size*2));
      highlight_max=over_thin && (x>s_x+width-(size*2));
      highlight_ball=over_thin && (x>s_x+ball_pos+(size*2))&& (x<s_x+ball_pos+ball_width+(size*4));
    } else {
      over_thin=((x>s_x) && (x<s_x+size*2));
      highlight_min=over_thin && (y<(s_y)+(size*2));
      highlight_max=over_thin &&(y>s_y+height-(size*2));
      highlight_ball=over_thin &&(y>s_y+(size*2)+ball_pos)&& (y<s_y+ball_pos+ball_width+(size*4));
    }
    if (value==max) highlight_max=false; 
    if (value==min) highlight_min=false;
    if (owner!=null) owner.highlight=true;
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
    return this;
  }
  
  public void mouseClick(int x, int y, MouseEvent e) {
    if (highlight_max) {
      step_inc();
      paintOn(gw.bi,gw.g2d);
      if (owner!=null) owner.paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();    
    } else if (highlight_min) {
      step_dec();
      paintOn(gw.bi,gw.g2d);
      if (owner!=null) owner.paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    } else {
      int s_x=pos_x,s_y=pos_y;
      if (parent!=null) {
        s_x += parent.get_parent_pos_x();
        s_y += parent.get_parent_pos_y();
      }
      if (orientation==GSlider.SLIDE_HORIZ) {
        if (x<s_x+(size*2)+ball_pos) {
          jump_dec();
        } else jump_inc();
      } else {
        if (y<(s_y)+(size*2)+ball_pos) {
          jump_dec();
        } else jump_inc();
      }
      paintOn(gw.bi,gw.g2d);
      if (owner!=null) owner.paintOn(gw.bi,gw.g2d);
      gw.requestRepaint();
    }
  }
  
  public void mouseWheel(MouseWheelEvent e) {
    int delta = e.getWheelRotation();
    step_delta(delta);
    gw.ga.doFunction(function,this);
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }
 
  public void keyPress(KeyEvent e) {
    if (owner!=null) owner.keyPress(e);
    else {
      if (e.getKeyCode()==KeyEvent.VK_UP) step_dec();
      else if (e.getKeyCode()==KeyEvent.VK_DOWN) step_inc();
      else if (e.getKeyCode()==KeyEvent.VK_PAGE_UP) jump_dec();
      else if (e.getKeyCode()==KeyEvent.VK_DOWN) jump_inc();
    }
    
  }
  
  public void setBigJump(int i) { big_jump=i; }
  public void setBallWidth(int i) { ball_width=i; }
  
  
  private void updateSliderSize(int total_entries,int screen_entries,int max_size) {
    int bsize;
    if (total_entries-screen_entries<=0) bsize=max_size;
    else {
      bsize = (int) (max_size*screen_entries/(1+total_entries));
      if (bsize<0) bsize=1;
      if (bsize>max_size) bsize=max_size;
    }
    setBallWidth(bsize);
    setMin(0);
    setMax(Math.max(0,total_entries-screen_entries));
  }

  public void updateSliderSize(int total, int screen) {
    if (orientation==SLIDE_VERT) updateSliderSize(total,screen,(getHeight()-(getSize()*6))-4);
    else updateSliderSize(total,screen,(getWidth()-(getSize()*6))-4);
  }
  
  public GSlider(int _x, int _y, int _wid, byte _orient,GContainer _parent, int _min, int _max, int _val, int _function, GWindow _gw) {
    super(_parent,_gw);
    pos_x=_x;
    pos_y=_y;
    orientation=_orient;
    size=5;
    ball_width=1;
    big_jump=0;
    width=(orientation==SLIDE_HORIZ)?_wid:size*2;
    height=(orientation==SLIDE_HORIZ)?size*2:_wid;
    triColour=new Color(gw.gcs.tri);
    high_triColour=new Color(gw.gcs.htri);
    ballColour=new Color(gw.gcs.ball);
    ringColour=new Color(gw.gcs.ring);
    high_ballColour=new Color(gw.gcs.hball);
    high_ringColour=new Color(gw.gcs.hring);
    drag_ballColour=new Color(gw.gcs.dball);
    drag_ringColour=new Color(gw.gcs.dring);
    owner=null;
    max=_max;
    min=_min;
    value=_val;
    function=_function;
  }
  
  public void step_dec() { value=Math.max(value-1,min); gw.ga.doFunction(function,this); updateBallFromVal(); }
  public void step_inc() { value=Math.min(value+1,max); gw.ga.doFunction(function,this); updateBallFromVal(); }
  public void step_delta(int delta) {
    value+=delta;
    if (value>max) value=max;
    if (value<min) value=min;
    gw.ga.doFunction(function,this);
    updateBallFromVal();
  }
  
  public void jump_dec() { 
    value-=big_jump;
    if (value<min) value=min;
    gw.ga.doFunction(function,this);
    updateBallFromVal();
  }
  
  public void jump_inc() { 
    value+=big_jump;
    if (value>max) value=max;
    gw.ga.doFunction(function,this);
    updateBallFromVal();
  }
  
  public void inc_select() {
    if (owner==null) step_inc();
    else {
      if (owner instanceof GList) {
        GList gl = (GList) owner;
        if (gl.selection_mode==GList.SINGLE_SELECTION) {
          int i = gl.getSelected();
          if ((i+1)<gl.countEntries()) i++;
          gl.setSelected(i);
          if (value>i) value=i;
          while (value+gl.entriesInBox<=i) value++;
        } else {
          gl.setItemHighlighted(gl.getItemHighlighted()+1);
        }
      } else if (owner instanceof GTable) {
        //GTable gt = (GTable) owner;
        
        
      }
      updateBallFromVal();
    }
  }
  
  public void dec_select() {
    if (owner==null) step_inc();
    else {
      if (owner instanceof GList) {
        GList gl = (GList) owner;
        if (gl.selection_mode==GList.SINGLE_SELECTION) {
          int i = gl.getSelected();
          if ((i-1)>0) i--;
          gl.setSelected(i);
          if (value>i) value=i;
          while (value+gl.entriesInBox<=i) value++;
        } else {
          gl.setItemHighlighted(gl.getItemHighlighted()-1);
        }
      } else if (owner instanceof GTable) {
        //GTable gt = (GTable) owner;
        
      }
      updateBallFromVal();
    }  
  }
  


  
  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      Polygon triangle = new Polygon();
      if (orientation==SLIDE_HORIZ) {
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g.setColor(backColour);
        g.fillRect(par_x+pos_x-2,par_y+pos_y-2,width+size,5+(size*2));
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
          
        g.drawLine(pos_x+par_x+size,pos_y+par_y+size,pos_x+par_x+width-size,pos_y+par_y+size);
        triangle.addPoint(par_x+pos_x,pos_y+par_y+size);
        triangle.addPoint(par_x+pos_x+(size*2),pos_y+par_y);
        triangle.addPoint(par_x+pos_x+(size*2),pos_y+par_y+(size*2));
        g.setColor(highlight_min?high_triColour:triColour);
        g.fillPolygon(triangle);
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.drawPolygon(triangle);
        triangle.reset();
        triangle.addPoint(par_x+pos_x+width,pos_y+par_y+size);
        triangle.addPoint(par_x+pos_x+width-(size*2),pos_y+par_y);
        triangle.addPoint(par_x+pos_x+width-(size*2),pos_y+par_y+(size*2));
        g.setColor(highlight_max?high_triColour:triColour);
        g.fillPolygon(triangle);
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.drawPolygon(triangle);
        updateBallFromVal();
        g.setColor(dragging?drag_ballColour:highlight_ball?high_ballColour:ballColour);
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g.fillOval(par_x+pos_x+2+(size*2)+ball_pos,par_y+pos_y,size*2,size*2);
        g.fillOval(par_x+pos_x+2+(size*2)+ball_pos+ball_width,par_y+pos_y,size*2,size*2);
        g.fillRect(par_x+pos_x+2+(size*3)+ball_pos,par_y+pos_y,ball_width,size*2);
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g.setColor(highlight_ball?high_ringColour:ringColour);
        //ball_width=30;
        g.drawArc(par_x+pos_x+2+(size*2)+ball_pos,par_y+pos_y,(size*2),(size*2),90,180);   // Left side
        g.drawArc(par_x+pos_x+2+(size*2)+ball_pos+ball_width,par_y+pos_y,(size*2),(size*2),90,-180);  // Right side
        g.drawLine(par_x+pos_x+2+(size*3)+ball_pos+1,par_y+pos_y,par_x+pos_x+(size*3)+2+ball_pos+ball_width-1,par_y+pos_y);
        g.drawLine(par_x+pos_x+2+(size*3)+ball_pos+1,par_y+pos_y+(size*2),par_x+pos_x+(size*3)+2+ball_pos+ball_width-1,par_y+pos_y+(size*2));
      } else {
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g.setColor(backColour);
        g.fillRect(par_x+pos_x-2,par_y+pos_y-1,4+(size*2),height+size-2);
        //g.drawLine(par_x+pos_x-1,par_y+pos_y-1,par_x+pos_x+1+(size*2),par_y+pos_y-1);
        //g.drawLine(par_x+pos_x-1,par_y+pos_y-2,par_x+pos_x+(size*2),par_y+pos_y-2);
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g.drawLine(pos_x+par_x+size,pos_y+par_y+size,pos_x+par_x+size,(pos_y+par_y+height)-size);
        triangle.addPoint(par_x+pos_x+size,pos_y+par_y);
        triangle.addPoint(par_x+pos_x+(size*2),(pos_y+par_y)+(size*2));
        triangle.addPoint(par_x+pos_x,(pos_y+par_y)+(size*2));
        g.setColor(highlight_min?high_triColour:triColour);
        g.fillPolygon(triangle);
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.drawPolygon(triangle);
        triangle.reset();
        triangle.addPoint(par_x+pos_x+size,pos_y+par_y+height);
        triangle.addPoint(par_x+pos_x+(size*2),pos_y+par_y+height-(size*2));
        triangle.addPoint(par_x+pos_x,pos_y+par_y+height-(size*2));
        g.setColor(highlight_max?high_triColour:triColour);
        g.fillPolygon(triangle);
        g.setColor(highlight?high_edgeColour:edgeColour);
        g.drawPolygon(triangle);
        updateBallFromVal();
        g.setColor(dragging?drag_ballColour:highlight_ball?high_ballColour:ballColour);
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        g.fillOval(par_x+pos_x,(par_y+pos_y)+2+(size*2)+ball_pos,size*2,size*2);
        g.fillOval(par_x+pos_x,(par_y+pos_y)+2+(size*2)+ball_pos+ball_width,size*2,size*2);
        g.fillRect(par_x+pos_x,(par_y+pos_y)+2+(size*3)+ball_pos,size*2,ball_width);
        g.setColor(highlight_ball?high_ringColour:ringColour);
        
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g.drawArc(par_x+pos_x,par_y+pos_y+2+(size*2)+ball_pos,size*2,size*2,0,180); // Top side
        g.drawArc(par_x+pos_x,par_y+pos_y+2+(size*2)+ball_pos+ball_width,size*2,size*2,0,-180); // Bottom side
        g.drawLine(par_x+pos_x,par_y+pos_y+2+(size*3)+ball_pos+1,par_x+pos_x,par_y+pos_y+2+(size*3)+ball_pos+ball_width-1);
        g.drawLine((size*2)+par_x+pos_x,par_y+pos_y+2+(size*3)+ball_pos+1,(size*2)+par_x+pos_x,par_y+pos_y+2+(size*3)+ball_pos+ball_width-1);
        
                
      }
    }
  }

  public void updateBallFromVal() {
    int max_pix = (((orientation==SLIDE_VERT?height:width)-(6*size))-ball_width)-4;
    ball_pos=(int) (max_pix* ((double)value-min)/((double)max-min));
  }
  
  public void setBall(int mouse_x) {
    int max_pix = (((orientation==SLIDE_VERT?height:width)-(6*size))-ball_width)-4;
    ball_pos = mouse_x;
    if (ball_pos<0) ball_pos=0;
    if (orientation==SLIDE_VERT) { 
      ball_pos=(int) Math.round(Math.min(ball_pos,max_pix));
    } else {
      ball_pos=(int) Math.round(Math.min(ball_pos,max_pix));
    }
    updateValFromBall();

  }
  
  public void setValue(int v) {
    value=v;
    updateBallFromVal();
    gw.ga.doFunction(function,this);
  }
  
  public void updateValFromBall() {
    int max_pix = (((orientation==SLIDE_VERT?height:width)-(6*size))-ball_width)-4;
    value = min+ (int) (((max-min)*((double)ball_pos/max_pix)));
    if (value<min) value=min;
    if (value>max) value=max;
    gw.ga.doFunction(function,this);
  }
}

/* GLine.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A line! 
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
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;


public class GLine extends GComponent {
  Color colour;
  int x2,y2;
  
  public GComponent mouseExit(GComponent next) { return next; }
  public void mouseWithin(int x, int y) {}
  public GComponent mouseEnter(int x, int y, GComponent previous) { return this;} 
  public void mouseClick(int x, int y, MouseEvent e) {}
  public void mouseWheel(MouseWheelEvent e) {}
  public void keyPress(KeyEvent e) {}
  
  public GLine(int _x, int _y, int _x2, int _y2,Color _col,GContainer _parent,GWindow gw) {
    super(_parent,gw);
    x2=_x2;
    y2=_y2;
    pos_x=_x;
    pos_y=_y;
    width=Math.abs(x2-_x);
    height=Math.abs(y2-_y);
    colour = new Color(_col.getRGB());
  }
  
  public void paintOn(BufferedImage bi, Graphics2D g) {
    if (isVisible()) {
      g.setColor(colour);
      int parent_pos_x = parent.get_parent_pos_x();
      int parent_pos_y = parent.get_parent_pos_y();
      g.drawLine(parent_pos_x+pos_x,parent_pos_y+pos_y,parent_pos_x+x2,parent_pos_y+y2);
    }
  }
}

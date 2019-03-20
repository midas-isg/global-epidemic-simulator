/* GItem.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Parent for all visible components 
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
import java.awt.image.BufferedImage;

public abstract class GItem {
  
  private boolean visible;
  protected int pos_x, pos_y;
  int width, height;
  GContainer parent;
  Color high_edgeColour;
  Color edgeColour;
  Color backColour;
  Color high_backColour;
  boolean enabled;
  GWindow gw;

  public abstract void paintOn(BufferedImage bi,Graphics2D g);

  public GItem(GContainer _parent,GWindow _gw) {
    parent=_parent;
    gw=_gw;
    visible=true;
    edgeColour=new Color(gw.gcs.edge);
    high_edgeColour=new Color(gw.gcs.hedge);
    backColour=new Color(gw.gcs.back);
    high_backColour=new Color(gw.gcs.hback);
    enabled=true;
  }
  
  public int get_parent_pos_x() {
    if (parent!=null) return pos_x+parent.get_parent_pos_x();
    else return pos_x;
  }
  
  public int get_parent_pos_y() {
    if (parent!=null) return pos_y+parent.get_parent_pos_y();
    else return pos_y;
  }
  
  public void setX(int x) { pos_x=x; }
  public void setY(int y) { pos_y=y; }
  public int getX() { return pos_x; }
  public int getY() { return pos_y; }
  public void setWidth(int w) { width=w; }
  public void setHeight(int h) { height=h; }
  public int getWidth() { return width; }
  public int getHeight() { return height; }

  
  public void setEdgeColour(Color c) { edgeColour = new Color(c.getRGB()); }
  public void setHighlightEdgeColour(Color c) { high_edgeColour = new Color(c.getRGB()); }
  public void setBackColour(Color c) { backColour = new Color(c.getRGB()); }
  public void setHighlightBackColour(Color c) { high_backColour = new Color(c.getRGB()); }
  public void setEnabled(boolean b) { 
    if (enabled!=b) {
      enabled=b;
      paintOn(gw.bi(),gw.g2d());
      gw.requestRepaint();
    }
  }
  
  public boolean isEnabled() { return enabled; }
  public void setVisible(boolean vis) {
    visible=vis;
    if (gw.g2d()!=null) paintOn(gw.bi(),gw.g2d());
  }
  
  public boolean isVisible() { 
    if (parent!=null) return visible && parent.isVisible();
    else return visible;
  }
  
  public boolean contains(int x, int y) {
    int par_x=0,par_y=0;
    if (parent!=null) {
      par_x = parent.get_parent_pos_x();
      par_y = parent.get_parent_pos_y();
    }
    return ((x>=pos_x+par_x)&& (x<=pos_x+par_x+width) && (y>=pos_y+par_y) && (y<=pos_y+par_y+height));
  }
}

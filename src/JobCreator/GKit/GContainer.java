/* GContainer.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Superclass of panels and other "parents" 
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
import java.awt.image.BufferedImage;
import java.util.ArrayList;

import javax.swing.JComponent;


public abstract class GContainer extends GItem {
  ArrayList<GItem> children;
  ArrayList<JComponent> jChildren;
  boolean enableDrag;
  byte titleHeight;
  
  public GContainer(GContainer _parent,GWindow gw) {
    super(_parent,gw);
    children = new ArrayList<GItem>();
    jChildren = new ArrayList<JComponent>();
    setVisible(true);
  }
    
  public GItem addChild(GItem o) { children.add(o); return o; }
  public GItem addHiddenChild(GItem o) { children.add(o); o.setVisible(false); return o; }  
  public GItem addChild(GItem o, boolean vis) { children.add(o); o.setVisible(vis); return o; }
  public void addChildren(GItem[] oo) { 
    for (int i=0; i<oo.length; i++) addChild(oo[i]);
  }
  public void addJComponent(JComponent o) { jChildren.add(o); }
  public void removeJComponent(JComponent o) { jChildren.remove(o); }
  
  public GComponent getComponent(int x, int y) {
    GComponent result=null;
    for (int i=0; i<children.size(); i++) {
      Object o = children.get(i);
      if (o instanceof GComponent) {
        GComponent gc = (GComponent) o;
        if (gc.isVisible()) {
          if (!(gc instanceof GLine)) {
            if (gc.contains(x,y)) {
              i=children.size();
              result=gc;
            }
          }
        }
      }
    }
    return result;
  }
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    for (int i=0; i<children.size(); i++) {
      Object o = children.get(i);
      if (o instanceof GContainer) ((GContainer)o).paintOn(bi,g);
      else if (o instanceof GComponent) ((GComponent)o).paintOn(bi,g);
    }
  }
  
  //public void setVisible(boolean vis) {
   // visible=vis;
    //for (int i=0; i<jChildren.size(); i++)
//      ((JComponent) jChildren.get(i)).setVisible(vis);
    //for (int i=0; i<children.size(); i++)
//      children.get(i).setVisible(vis);
  //}
  
}

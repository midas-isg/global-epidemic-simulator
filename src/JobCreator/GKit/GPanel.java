/* GPanel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A generic panel - parent for other components (or panels) 
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

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import java.awt.image.BufferedImage;

public class GPanel extends GContainer {
  boolean title;
  Color titleColour;
  String titleString;
  Color titleFontColour;
  
     
  public void setWidth(int _width) { width=_width; }
  public void setHeight(int _height) { height=_height;}
  public void setX(int _x) { pos_x=_x; }
  public void setY(int _y) { pos_y=_y; }
  public void setTitle(boolean _title) { title=_title;}
  public void setTitleColour(Color t) { titleColour = new Color(t.getRGB()); }
  public void setTitleFontColour(Color t) { titleFontColour = new Color(t.getRGB()); }  
  public void setTitleString(String s) { titleString = new String(s); }
  public void setTitleHeight(byte b) { titleHeight = b; }
  public void setDragEnabled(boolean b) { enableDrag = b; }
  
  public GPanel(int _pos_x, int _pos_y, int _width, int _height, 
                boolean _title, String _titleString, byte _titleHeight, boolean _enableDrag,GContainer _parent,GWindow _gw) {
    
    super(_parent,_gw);
    width=_width;
    height=_height;
    pos_x=_pos_x;
    pos_y=_pos_y;
    title=_title;
    if (title) titleColour=new Color(_gw.gcs.titleBack);
    titleString=new String(_titleString);
    titleFontColour = new Color(_gw.gcs.titleText);
    titleHeight=_titleHeight;
    enableDrag=_enableDrag;
  }
 
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      g.setStroke(new BasicStroke(2.0f)); // 2-pixel lines
      RoundRectangle2D roundedRectangle = new RoundRectangle2D.Float(par_x+pos_x,par_y+pos_y,width,height,10,10);
      if (title) {
        g.setColor(titleColour);
        g.fill(roundedRectangle);
        g.setColor(backColour);
        RoundRectangle2D rest_bottom = new RoundRectangle2D.Float(par_x+pos_x,par_y+pos_y+titleHeight,width,height-titleHeight,10,10);
        g.fill(rest_bottom);
        Rectangle2D rest_top = new Rectangle2D.Float(par_x+pos_x,par_y+pos_y+titleHeight,width,10);
        g.fill(rest_top);
        g.setColor(edgeColour);
        g.draw(roundedRectangle);
        g.drawLine(par_x+pos_x+1,par_y+pos_y+titleHeight,par_x+pos_x+width-1,par_y+pos_y+titleHeight);
        if (titleString.length()>0) {
          g.setFont(GWindow.TITLE_FONT);
          g.setColor(Color.DARK_GRAY);
          g.drawString(titleString,par_x+pos_x+11,par_y+pos_y+17);
          g.setColor(titleFontColour);
          g.drawString(titleString,par_x+pos_x+10,par_y+pos_y+17);
        }
      } else {
        g.setColor(backColour);
        g.fill(roundedRectangle);
        g.setColor(edgeColour);
        g.draw(roundedRectangle);
      }
    }
    
    super.paintOn(bi,g);
  }

}

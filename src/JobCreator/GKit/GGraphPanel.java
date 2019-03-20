/* GGraphPanel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A panel with a single line graph. (Simple) 
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
import java.awt.image.BufferedImage;


public class GGraphPanel extends GPanel {
  double max_x,max_y,min_x,min_y;
  BufferedImage gImage;
  int graph_func;
  byte graph_type;
  double[] results;
  Color lineColour;
  boolean auto;
  
  public void setMinX(double x) { min_x=x; }
  public void setMaxX(double x) { max_x=x; }
  public void setMinY(double y) { min_y=y; }
  public void setMaxY(double y) { max_y=y; }
  public void setAuto(boolean _auto) { auto=_auto; } 
  public void setGFunc(int x) { graph_func=x; }
  public void setLineColour(Color c) { lineColour = new Color(c.getRGB()); }
  public void setType(byte b) { graph_type=b; }
  public static final byte LINE_GRAPH = 1;
  public static final byte HISTOGRAM = 2;
  
  
  public void updateGraph() {
    if (graph_type==LINE_GRAPH) updateLineGraph();
  }
  
  public void updateLineGraph() {
    Graphics2D pg = (Graphics2D) gImage.getGraphics();
    pg.setColor(backColour);
    pg.fillRect(0,0,gImage.getWidth(),gImage.getHeight());
    pg.setColor(Color.WHITE);
    int y_border = 20;
    int y_height = gImage.getHeight()-y_border;
    int y_top = 5;
    int x_border = 15;
    int x_width = gImage.getWidth()-x_border;
    
    pg.drawLine(x_border,y_top,x_border,y_top+y_height);
    
    pg.drawLine(x_border,y_top+y_height,x_border+x_width,y_top+y_height);
    pg.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    pg.setColor(lineColour);
    for (int i=0; i<results.length; i++) {
      double x_val = min_x+((max_x-min_x)*((double)i/x_width));
      results[i]=gw.ga.graphFunction(this,graph_func,x_val);
      if (Double.isNaN(results[i])) results[i]=0;
      if (auto) {
        if (i==0) {
          min_y=results[0];
          max_y=results[0];
        } else {
          if (results[i]>max_y) max_y=results[i];
          if (results[i]<min_y) min_y=results[i];
        }
      }
    }
    if (max_y==min_y) {
      if (max_y==0) {
        max_y=0.5;
        min_y=-0.5;
      } else {
        max_y+=min_y;
        min_y-=min_y;
      }
      
    }
    double scale_y=y_height/(max_y-min_y);
    double old_y=y_top+y_height-((results[0]-min_y)*scale_y);
    for (int i=1; i<results.length; i++) {
      double new_y=y_top+y_height-((results[i]-min_y)*scale_y);
      pg.drawLine(x_border+i-1,(int)old_y,x_border+i,(int)new_y);
      old_y=new_y;
    }
    
    pg.setFont(GWindow.TGRAPH_FONT);
    pg.setColor(Color.WHITE);

    int no_x_axis_points = x_width/28;
    for (int i=0; i<=no_x_axis_points; i++) {
      int x_mid = (int) ((double)i*(x_width/no_x_axis_points));
      String x_val = String.valueOf((int)Math.round(10.0*(min_x+  ((max_x-min_x)*((double)i/(double)no_x_axis_points)))));
      while (x_val.length()<2) x_val="0"+x_val;
      x_val=x_val.substring(0,x_val.length()-1)+"."+x_val.substring(x_val.length()-1);
      pg.drawString(x_val,x_border+x_mid-2,y_top+y_height+12);
      pg.drawLine(x_border+x_mid,y_top+y_height,x_border+x_mid,y_top+y_height+2);
    }
    
    
    int no_y_axis_points = y_height/16;
    for (int j=0; j<=no_y_axis_points; j++) {
      int y_mid = y_height - (int) ((double)j*(y_height/no_y_axis_points));
      String y_val = String.valueOf((int)Math.round(10.0*(min_y+  ((max_y-min_y)*((double)j/(double)no_y_axis_points)))));
      if (y_val.length()==1) y_val="0"+y_val;
      y_val=y_val.substring(0,y_val.length()-1)+"."+y_val.substring(y_val.length()-1);
      pg.drawString(y_val,1,y_mid+y_top+3);
      pg.drawLine(x_border,y_top+y_mid,x_border-2,y_top+y_mid);
    }
    
   
    paintOn(gw.bi,gw.g2d);
    gw.requestRepaint();
  }

  public GGraphPanel(int posX, int posY, int width, int height, boolean title, String titleString, 
      byte titleHeight, boolean enableDrag, GContainer parent,GWindow gw, int _func) {

    super(posX,posY,width, height, title, titleString, titleHeight, enableDrag, parent,gw);
    gImage = new BufferedImage(width-6,height-6,BufferedImage.TYPE_3BYTE_BGR);
    graph_func=_func;
    results = new double[width-21];
    lineColour = Color.white;
  }
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      super.paintOn(bi,g);
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }
      g.drawImage(gImage,pos_x+par_x+3,pos_y+par_y+3,null);
    }
  }
}

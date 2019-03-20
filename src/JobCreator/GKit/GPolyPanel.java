/* GPolyPanel.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A panel with polygon display. (Written for GSIM admin unit page) 
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
import java.awt.image.BufferedImage;
import java.util.ArrayList;


public class GPolyPanel extends GPanel {
  ArrayList<GPolygon> polys;
  ArrayList<GPolygon> scaled_polys;
  public final static byte SCALE_XY =1;
  public final static byte SCALE_PROP =2;
  byte scale_op;
  int max_x,max_y,min_x,min_y;
  BufferedImage polyImage;
  private boolean pending;
  protected boolean cancel=false;
  
  public void clearPolys() { polys.clear(); }
  public ArrayList<GPolygon> polys() { return polys; }
  
  public static ArrayList<Integer> getPixels(int xP, int yP, int xQ, int yQ) {
    ArrayList<Integer> pixels = new ArrayList<Integer>();
    if ((xP==xQ) && (yP==yQ)) {
      pixels.add(new Integer(xP));
      pixels.add(new Integer(yP));
    } else {
      int x=xP;
      int y=yP;
      int D=0;
      int HX = xQ-xP;
      int HY = yQ-yP;
      int c,M;
      int xInc=1;
      int yInc=1;

      if (HX<0) { xInc = -1; HX = -HX; }
      if (HY<0) { yInc = -1; HY = -HY; }

      if (HY<=HX) { 
        c = 2*HX; 
        M = 2*HY;
        while (x!=xQ) {
          pixels.add(new Integer(x));
          pixels.add(new Integer(y));
          x+=xInc;
          D+=M;
          if (D>HX) { y += yInc; D -=c; }
        }
      } else {
        c=2*HY;
        M=2*HX;
        while (y!=yQ) {
          pixels.add(new Integer(x));
          pixels.add(new Integer(y));
          y+=yInc;
          D+=M;
          if (D>HY){ x+= xInc; D-=c; }
        }
      }
      pixels.add(new Integer(xQ));
      pixels.add(new Integer(yQ));
    }
    return pixels;
  }
  
  public boolean isCanceled() {
    return cancel;
  }
  
  public void cancel() {
    cancel=true;
  }
  
  public boolean lock() {
    if (pending) {
      cancel();
    }
    
    while (pending)  {
      try {
        Thread.sleep(1000);
      } catch (Exception e) {}
    }
    pending=true;
    return pending;
  }
  
  public void unlock() {
    while (!pending) {
      try {
        Thread.sleep(1000);
      } catch (Exception e) {}
    }
    pending=false;
  }
  
  public void updatePolyImage() {
    max_x=Integer.MIN_VALUE;
    min_x=Integer.MAX_VALUE;
    max_y=Integer.MIN_VALUE;
    min_y=Integer.MAX_VALUE;
  
    for (int i=0; i<polys.size(); i++) {
      GPolygon gp = polys.get(i);
      int min2=Integer.MAX_VALUE;
      int max2=Integer.MIN_VALUE;

      for (int j=0; j<gp.npoints; j++) {
        min2=Math.min(min2,gp.xpoints[j]);
        max2=Math.min(max2,gp.xpoints[j]);
        if (gp.xpoints[j]>max_x) max_x=gp.xpoints[j];
        if (gp.ypoints[j]>max_y) max_y=gp.ypoints[j];
        if (gp.xpoints[j]<min_x) min_x=gp.xpoints[j];
        if (gp.ypoints[j]<min_y) min_y=gp.ypoints[j];
      }
      if ((min2<0) && (max2>0)) {
        System.out.println("SIGN PROBLEM!");
      }
   }
    
    for (int i=0; i<scaled_polys.size(); i++) scaled_polys.get(i).reset();
    scaled_polys.clear();
    double scale_x=1;
    double scale_y=1;
    if (scale_op==SCALE_XY) {
      scale_x=((double)width-6)/((double)max_x-min_x);
      scale_y=((double)height-6)/((double)max_y-min_y);
    } else if (scale_op==SCALE_PROP) {
      int spread_x = max_x-min_x;
      int spread_y = max_y-min_y;
      if (spread_x>2*spread_y) {
        int diff = ((spread_x/2)-spread_y)/2;
        min_y-=diff;
        max_y+=diff;
      }
      if (spread_x<2*spread_y) {
        int diff = ((2*spread_y)-spread_x)/2;
        max_x+=diff;
        min_x-=diff;
      }
      
      scale_x=((double)width-6)/((double)max_x-min_x);
      scale_y=((double)height-6)/((double)max_y-min_y);
    }
    
    for (int i=0; i<polys.size(); i++) {
      GPolygon gp = new GPolygon(polys.get(i).getFillColour(),polys.get(i).getLineColour());
      for (int j=0; j<polys.get(i).npoints; j++)
        gp.addPoint(3+(int) (scale_x*(polys.get(i).xpoints[j]-min_x)),
                    height-(int) (3+(scale_y*(polys.get(i).ypoints[j]-min_y))));
      scaled_polys.add(gp);

      if (isCanceled()) {
        i=polys.size()+1;
      }
    }
    Graphics2D g = (Graphics2D) polyImage.getGraphics();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
    g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
    g.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
    g.setRenderingHint(RenderingHints.KEY_FRACTIONALMETRICS, RenderingHints.VALUE_FRACTIONALMETRICS_ON);
    g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR);
    g.setRenderingHint(RenderingHints.KEY_COLOR_RENDERING, RenderingHints.VALUE_COLOR_RENDER_QUALITY);
    //g.setRenderingHint(RenderingHints.KEY_TEXT_LCD_CONTRAST, 140);
    g.setRenderingHint(RenderingHints.KEY_STROKE_CONTROL,RenderingHints.VALUE_STROKE_NORMALIZE);

    g.setColor(backColour);
    g.fillRect(0,0,width-6,height-6);
    for (int i=0; i<scaled_polys.size(); i++) {
      GPolygon gp = scaled_polys.get(i);
      
      g.setColor(gp.getFillColour());
      g.fillPolygon(gp);
      g.setColor(gp.getLineColour());
      g.drawPolygon(gp);
      if (isCanceled()) {
        i=scaled_polys.size();
        g.setColor(backColour);
        g.fillRect(0,0,width-6,height-6);
      }
    }
    if (isCanceled()) {
      scaled_polys.clear();
      cancel=false;
    }
    
  }

  public GPolyPanel(int posX, int posY, int width, int height, boolean title, String titleString, 
      byte titleHeight, boolean enableDrag, GContainer parent,GWindow gw,byte scale_mode) {

    super(posX,posY,width, height, title, titleString, titleHeight, enableDrag, parent,gw);
    polys = new ArrayList<GPolygon>();
    scaled_polys = new ArrayList<GPolygon>();
    scale_op=scale_mode;
    polyImage = new BufferedImage(width-6,height-6,BufferedImage.TYPE_3BYTE_BGR);
  }
  
  public void paintOn(BufferedImage bi,Graphics2D g) {
    if (isVisible()) {
      super.paintOn(bi,g);
      int par_x=0,par_y=0;
      if (parent!=null) {
        par_x = parent.get_parent_pos_x();
        par_y = parent.get_parent_pos_y();
      }

      if (!pending) g.drawImage(polyImage,pos_x+par_x+3,pos_y+par_y+3,null);
      else g.drawImage(gw.time_img,pos_x+par_x+((width-102)/2),pos_y+par_y+((height-186)/2),null);
    }
  }
}

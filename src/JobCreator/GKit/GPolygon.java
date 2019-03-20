/* GPolygon.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: A polygon with a group no, and clockwise status 
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
import java.awt.Polygon;

@SuppressWarnings("serial")
public class GPolygon extends Polygon {
  private Color line_colour;
  private Color fill_colour;
  private byte anti_clockwise=UNSET;
  private int group_no;
  private int no_in_group;
  
  private static final byte ANTI=1;
  private static final byte NOT_ANTI=2;
  private static final byte UNSET = 3;
    
  public GPolygon(Color l, Color f) {
    line_colour=new Color(l.getRGB());
    fill_colour=new Color(f.getRGB());
  }
  
  public void setGroupNo(int i) { group_no=i; }
  public void setNoInGroup(int i) { no_in_group=i; }
  public int getGroupNo() { return group_no; }
  public int getNoInGroup() { return no_in_group; }
  public Color getLineColour() { return line_colour; }
  public Color getFillColour() { return fill_colour; }
    
  public boolean isAntiClockwise() {
    if (anti_clockwise==UNSET) {
      int sum=((360+xpoints[0])-(360+xpoints[npoints-1]))*(180+ypoints[0]+180+ypoints[npoints-1]);
      for (int i=1; i<npoints; i++) {
        sum+=((360+xpoints[i])-(360+xpoints[i-1]))*(180+ypoints[i]+180+ypoints[i-1]);
      }
      if (sum<0) anti_clockwise=ANTI;
      else anti_clockwise=NOT_ANTI;
    }
    return (anti_clockwise==ANTI);
  }
}

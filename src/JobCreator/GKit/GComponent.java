/* GComponent.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Superclass of GItems and GContainers 
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
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.image.BufferedImage;


public abstract class GComponent extends GItem {
  
  public GComponent(GContainer _parent,GWindow gw) {
    super(_parent,gw);
  }
  
  public abstract void paintOn(BufferedImage bi,Graphics2D g);
  public abstract GComponent mouseExit(GComponent next);
  public abstract void mouseWithin(int x, int y);
  public abstract GComponent mouseEnter(int x, int y, GComponent previous);
  public abstract void mouseClick(int x, int y, MouseEvent e);
  public abstract void mouseWheel(MouseWheelEvent e);
  public abstract void keyPress(KeyEvent e);
  
  boolean mouse_over;
  boolean highlight;
  
}

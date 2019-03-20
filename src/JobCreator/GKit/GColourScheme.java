/* GColourScheme.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Colour schemes. Only one we like so far. 
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


public class GColourScheme {
  int edge,hedge,back,hback,tri,htri,ring,hring;
  int dring,ball,hball,dball,titleBack,titleText,text;
  int list_text, list_high_text, list_entry_text, list_entry_back, table_back, table_fore,table_title_fore,table_title_back;
  int table_cell_select;
  Font list_font,label_font;
  
  public int getEdge() { return edge; }
  public int getHedge() { return hedge; }
  public Font getLabelFont() { return label_font; }
  public int getListHighText() { return list_high_text; }
  public int getText() { return text; }
  
  public GColourScheme() {}

  public static GColourScheme getDefault() {
    GColourScheme standard= new GColourScheme(); 
    standard.edge = new Color(70,61,55).getRGB();
    standard.hedge = new Color(103,89,78).getRGB();
    standard.back = new Color(20,20,20).getRGB();
    standard.hback = new Color(31,27,28).getRGB();
    standard.tri = new Color(80,80,80).getRGB();
    standard.htri = new Color(180,180,180).getRGB();
    standard.ball = new Color(80,80,80).getRGB();
    standard.ring = new Color(120,120,120).getRGB();
    standard.hball = new Color(200,200,200).getRGB();
    standard.hring = new Color(160,160,160).getRGB();
    standard.dball = new Color(160,160,240).getRGB();
    standard.dring = new Color(120,120,240).getRGB();
    standard.titleBack = new Color(60,60,60).getRGB();
    standard.titleText = new Color(255,255,255).getRGB();
    standard.text = new Color(230,230,230).getRGB();
    standard.list_text = new Color(200,200,200).getRGB();
    standard.list_high_text = new Color(255,255,255).getRGB();
    standard.list_entry_text = new Color(255,255,255).getRGB();
    standard.list_entry_back = new Color(46,55,143).getRGB();
    standard.list_font = new Font("Tahoma",Font.PLAIN,15);
    standard.label_font = new Font("Arial",Font.PLAIN,15);
    standard.table_back = new Color(80,80,80).getRGB();
    standard.table_fore = new Color(255,255,255).getRGB();    
    standard.table_title_back = new Color(80,80,160).getRGB();
    standard.table_title_fore = new Color(255,255,255).getRGB();
    standard.table_cell_select = new Color(80,160,80).getRGB();
    
    return standard;
  }
}

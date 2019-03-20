/* GApp.java, part of the Global Epidemic Simulation v1.0 BETA
/* GKit: Start building a GKit based toolkit implementing this. 
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

public interface GApp {
  public void doFunction(int func,Object component);
  public double graphFunction(GItem gc, int func, double x_val);
  public boolean isUnSaved();
  public void setUnSaved(boolean b);
  public void save();
  public void saveINIFile();
  public void loadINIFile();
}

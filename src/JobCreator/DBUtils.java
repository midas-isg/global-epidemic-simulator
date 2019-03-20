/* DBUtils.java, part of the Global Epidemic Simulation v1.0 BETA
/* Job Creator: initialise admin unit polygon support 
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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;

import GKit.GList;


public class DBUtils {
  ArrayList<String> d_iso = new ArrayList<String>();
  @SuppressWarnings("unchecked") 
  ArrayList<Integer>[] d_id = new ArrayList[6];
  @SuppressWarnings("unchecked") 
  ArrayList<String>[] d_name = new ArrayList[6];
  
  GSim gs;
  
  public DBUtils(GSim gs) { 
    this.gs=gs;
    for (int i=0; i<5; i++) {
      if (!new File(gs.gadmPath+"no_shapes_id"+i+".txt").exists()) {
        System.out.println("Couldn't find "+gs.gadmPath+"no_shapes_id"+i+".txt - check gsim_conf.xml and/or run bin/getAdminUnits.bat");
      }
      if (!new File(gs.gadmPath+"no_shapes_name"+i+".txt").exists()) {
        System.out.println("Couldn't find "+gs.gadmPath+"no_shapes_name"+i+".txt - check gsim_conf.xml and/or run bin/getAdminUnits.bat");
      }
    }
    if (!new File(gs.gadmPath+"no_shapes_iso.txt").exists()) {
      System.out.println("Couldn't find "+gs.gadmPath+"no_shapes_iso.txt - check gsim_conf.xml and/or run bin/getAdminUnits.bat");
    }
    if (!new File(gs.gadmPath+"bins.zip").exists()) {
      System.out.println("Couldn't find "+gs.gadmPath+"bins.zip - check gsim_conf.xml and/or run bin/getAdminUnits.bat");
    }
    
    
  }
  
  public void initDB() {
    try {
      for (int i=0; i<6; i++) {
        d_id[i]=new ArrayList<Integer>();
        d_name[i]=new ArrayList<String>();
      }
      for (int i=0; i<6; i++) {
        BufferedReader br_i = new BufferedReader(new FileReader(gs.gadmPath+"no_shapes_id"+i+".txt"));
        BufferedReader br_n = new BufferedReader(new FileReader(gs.gadmPath+"no_shapes_name"+i+".txt"));
        String s_i = br_i.readLine();
        String s_n = br_n.readLine();
        while (s_i!=null) {
          d_id[i].add(Integer.parseInt(s_i.trim()));
          d_name[i].add(s_n.trim());
          s_i = br_i.readLine();
          s_n = br_n.readLine();
        }
        br_i.close();
        br_n.close();
        if (i==0) { 
          for (int j=1; j<6; j++) {
            d_id[j].ensureCapacity(d_id[0].size());
            d_name[j].ensureCapacity(d_name[0].size());
          }
        }
      }
      BufferedReader br_iso = new BufferedReader(new FileReader(gs.gadmPath+"no_shapes_iso.txt"));
      for (int i=0; i<d_id[0].size(); i++) d_iso.add(br_iso.readLine().trim());
      br_iso.close();
    } catch (Exception e) { e.printStackTrace(); }
  }

  
  public String dq(String s) { if (s.length()<2) return "";
    else return s.substring(1,s.length()-1); 
  }  
  
  public void getNames(int level, int[] info, GList result, ArrayList<Integer> ids,String iso) {
    result.clearEntries();
    ids.clear();
    try {
      int i=0;
      while (i<d_iso.size()) {
        if (d_iso.get(i).equals(iso)) {
          boolean ok=true;
          for (int j=1; j<level; j++) {
            if (d_id[j].get(i).intValue()!=info[j]) ok=false;
          }
          if (ok) {
            boolean duplicate=false;
            int insertion=-1;
            for (int j=0; j<ids.size(); j++) {
              if (ids.get(j).intValue()==d_id[level].get(i).intValue()) {
                j=ids.size();
                duplicate=true;
              } else if ((insertion==-1) && (d_name[level].get(i).compareTo(result.getEntry(j))<0)) {
                insertion=j;
              }
            }
            if (!duplicate) {
              if (insertion==-1) {
                result.addEntry(d_name[level].get(i));
                ids.add(d_id[level].get(i));
              } else {
                result.addEntry(insertion,d_name[level].get(i));
                ids.add(insertion,d_id[level].get(i));
              }
            }
          }
        }
        i++;
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
    
  
  public String[] namesForIDs(String iso, int[] ids) {
    String[] result = null;
    int i=0;
    while (i<d_iso.size()) {
      if (d_iso.get(i).equals(iso)) {     // Find first i such that d_iso[i]==iso
        boolean ok=true;
        for (int j=1; j<ids.length; j++) {
          if (d_id[j].get(i)!=ids[j]) {
            j=ids.length;
            ok=false;
          }
        }
        if (ok) {
          result = new String[ids.length];
          for (int j=0; j<ids.length; j++) result[j]=d_name[j].get(i);
          i=d_iso.size();
        }
      }
      i++;
    }
    return result;
  }
}

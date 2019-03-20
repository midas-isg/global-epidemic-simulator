/* FFReader.java, part of the Global Epidemic Simulation v1.0 BETA
/* Standalone helper to extract per-unit info from output flat file.
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
import java.io.FileReader;


public class FFReader {
  public static void main(String[] args) throws Exception {
    
    // Flat file format: Time, Unit, Com Cases, HH Cases, P0-P3 Cases, Com Infs, HH Infs, P0-P3 Infs, Total symptomatic, Total non-symptomatic
    
    if (args.length!=2) { 
      System.out.println("Usage: ");
      System.out.println("  java FFReader filename.txt unit_id\n\n");
      System.out.println("Output:  tab-separated file:-");
      System.out.println("  Time (days) | Unit ID | New Comm Cases | New HH Cases | New P0..P3 Cases | New Comm Infs | New HH Infs | New P0..P3 cases | Total sympt | Total non-sympt");
      System.exit(1);
    } else {
      String file = args[0];
      int unit_id=Integer.parseInt(args[1]);
    
      BufferedReader br = new BufferedReader(new FileReader(file));
      String s = br.readLine();
      int time=0;
      while (s!=null) {
        String[] parts = s.split("\t");
        int unit = Integer.parseInt(parts[1]);
        if (unit==unit_id) {
          int new_time = Integer.parseInt(parts[0]);
          while (new_time>time) {
            System.out.print((time/24.0)+"\t"+unit_id+"\t");
            for (int i=2; i<parts.length; i++) System.out.print("0\t");
            System.out.print("\n");
            time+=6;
          }
          if (new_time==time) {
            System.out.print((time/24.0)+"\t"+unit_id+"\t");
            for (int i=2; i<parts.length; i++) System.out.print(parts[i]+"\t");
            System.out.print("\n");
            time+=6;
          }
        }
        s = br.readLine();
      }
      br.close();
    }
  }
}

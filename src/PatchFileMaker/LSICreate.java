/* LSICreate.java, part of the Global Epidemic Simulation v1.0 BETA
/* Standalone tool to create patch files for given configuration
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


import java.awt.Polygon;
import java.awt.Rectangle;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Arrays;

// This is a standalone Java program. Part of the Global Epidemic Simulator package.
//
// It takes three arguments:
//    (1) Path to a Landscan-style .FLT file. Assumes the accompanying .HDR is in the same folder.
//    (2) Path to an LSC file - this is the node boundary file created by the LSC-Creator. (not public yet).
//    (3) Output path and filename stub - files will have _n.lsi appended for each node n.
//
// It takes the LSC files, which give the polygons that are "local" for each node. For each node, it then
// decides where patches are required (by looking up in Landscan whether there are people in a patch), and
// allocates local patches of size 20 to the owner of that patch, and remote patches (size between 20 and 320
// to nodes that do not own that patch.  (20 and 320 are no. of landscan cells). 
//
// The LSI file is full of LSBFirst ints:  no_local_patches, no_remote_patches, then a list of
//                                         {x,y,size,node} - for each patch. One LSI file per node.
//
// Version 1
// Wes Hinsley 2011
// 
// Imperial College London

public class LSICreate {
  ArrayList<ArrayList<Integer>> nodeConfigs;
  LandScanData lsd;
  
  public LSICreate() {
    nodeConfigs=new ArrayList<ArrayList<Integer>>();
    lsd=null;
  }
  
  public ArrayList getPixels(int xP, int yP, int xQ, int yQ) {
    // Bresnan algorithm
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

  
  public void writePatch(int startX,int startY,int size,int node, int pop,DataOutputStream dos) {
    try {
      dos.writeInt(Integer.reverseBytes(startX));
      dos.writeInt(Integer.reverseBytes(startY));
      dos.writeInt(Integer.reverseBytes(size));
      dos.writeInt(Integer.reverseBytes(node));
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public int[] getNodesInBox(int startX,int startY, int size) {
    final Polygon p = new Polygon();
    final ArrayList<Integer> theList = new ArrayList<Integer>();
    for (int node=0; node<nodeConfigs.size(); node++) {
      final ArrayList<Integer> nodeConfig = nodeConfigs.get(node);
      int polyCount=0;
      int pointer=0;
      while (pointer<nodeConfig.size()) {
        pointer+=2*nodeConfig.get(pointer).intValue(); // Skip points;
        pointer++;
        polyCount++;
      }
      pointer=0;
      
      
      for (int i=0; i<polyCount; i++) {
        while (pointer<nodeConfig.size()) {
          p.reset();
          int npoints = nodeConfig.get(pointer++).intValue();
          
          for (int j=0; j<npoints; j++) p.addPoint(nodeConfig.get(pointer++).intValue(),nodeConfig.get(pointer++).intValue());
          if (p.intersects(startX,startY,size,size)) {
            theList.add(new Integer(node));
            pointer=nodeConfig.size();
            i=polyCount;
          }
        }
      }
    }
    final int[] nodeList = new int[theList.size()];
    for (int i=0; i<nodeList.length; i++) nodeList[i]=(theList.get(i)).intValue();
    theList.clear();
  
    return nodeList;
  }
  
  public int[] divideConquer(int x, int y, int size, int node, int threshold, Polygon[] polys, DataOutputStream dos) {
    int[] results = new int[2];
    results[0]=0;
    results[1]=0;
    Polygon p = new Polygon();
    p.reset();
    int endXLS=x+((size)-1);
    int endYLS=y+((size)-1);
    p.addPoint(x,y);
    p.addPoint(endXLS,y);
    p.addPoint(endXLS,endYLS);
    p.addPoint(x,endYLS);
    final long pop = getPop(p);
    if (pop>0) {
      
      boolean intersects=false;
      for (int poly=0; poly<polys.length; poly++) {
        if (polys[poly].intersects(x,y,size,size)) {
          intersects=true;
          poly=polys.length;
        }
      }
      if (intersects) {
        if (size>threshold) {
          int[] res = divideConquer(x,y,size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x+(size/2),y,size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x+(size/2),y+(size/2),size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x,y+(size/2),size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
        } else {
          writePatch(x,y,size,node,(int)pop,dos);
          results[0]++;
        }
      } else {
        int[] nodeList = getNodesInBox(x,y,size);
        if (nodeList.length==1) {
          writePatch(x,y,size,nodeList[0],(int)pop,dos);
          results[1]++;
        } else if (nodeList.length>1) {
          int[] res = divideConquer(x,y,size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x+(size/2),y,size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x+(size/2),y+(size/2),size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
          res = divideConquer(x,y+(size/2),size/2,node, threshold,polys,dos);
          results[0]+=res[0];
          results[1]+=res[1];
        }
      }
      
    }
    return results;
    
  }
  
  public long getPop(Polygon poly) {

    long pop=0;    
    if ((poly.npoints==4) && (poly.xpoints[0]==poly.xpoints[3]) && (poly.xpoints[1]==poly.xpoints[2]) && (poly.ypoints[0]==poly.ypoints[1]) && (poly.ypoints[2]==poly.ypoints[2])) {
      int[] rowData = new int[1+(poly.xpoints[1]-poly.xpoints[0])];
      for (int y=poly.ypoints[1]; y<=poly.ypoints[2]; y++) {
        lsd.getRowData(rowData,y,poly.xpoints[0],poly.xpoints[1]);
        for (int x=0; x<rowData.length; x++) {
          pop+=rowData[x];
        }
      }

      return pop;
    } else {
      long pop_total=0;
      final Rectangle bounds = poly.getBounds();
      final int booleanWidth = (int) Math.ceil((1+bounds.width)/8.0);
      final char[][] booleanGrid = new char[booleanWidth][bounds.height+1];
      int px,py;

      ArrayList pixels;    
      for (int i=0; i<poly.npoints; i++) {    // For each line segment in polygon
        if (i<poly.npoints-1) pixels = getPixels(poly.xpoints[i],poly.ypoints[i],poly.xpoints[i+1],poly.ypoints[i+1]);
        else pixels = getPixels(poly.xpoints[i],poly.ypoints[i],poly.xpoints[0],poly.ypoints[0]);
        for (int j=0; j<pixels.size(); j+=2) { // For each pixel in line segment
          px = ((Integer) pixels.get(j)).intValue();
          py = ((Integer) pixels.get(j+1)).intValue();      

          final int px_b = px-bounds.x;
          final int py_b = py-bounds.y;
          final int xdiv8 = px_b/8;
          final int xmod8 = px_b%8;

          if (((booleanGrid[xdiv8][py_b]) & (1<<xmod8))==0) {
            booleanGrid[xdiv8][py_b] = (char) ((booleanGrid[xdiv8][py_b]) | (1<< (xmod8))); 
            pop_total+=lsd.getData(px,py);
          }
        }
        pixels.clear();
      }

      if ((bounds.width>0) && (bounds.height>0)) {

        int start_i,end_i,i,j;
        for (j=0; j<=bounds.height; j++) {
          final int j_actual = j+bounds.y; 

          start_i=0;
          while ((start_i<bounds.width) && (((booleanGrid[start_i/8][j]) & (1<< (start_i%8)))==0)) start_i++; // Space before border
          while ((start_i<bounds.width) && (((booleanGrid[start_i/8][j]) & (1<< (start_i%8)))>0)) start_i++; // Actual border        
          end_i=bounds.width;
          while ((end_i>=start_i) && (((booleanGrid[end_i/8][j]) & (1<< (end_i%8)))==0)) end_i--; // Space to right of border
          while ((end_i>=start_i) && (((booleanGrid[end_i/8][j]) & (1<< (end_i%8)))>0)) end_i--; // Actual border
          boolean unknown=true;
          boolean inside=false;
          int[] rowData = new int[1+(end_i-start_i)];
          lsd.getRowData(rowData,j_actual,start_i+bounds.x,end_i+bounds.x);
          for (i=start_i; i<=end_i; i++) {
            final int i_actual = i+bounds.x;
            if (((booleanGrid[i/8][j]) & (1<< (i%8)))==0) {
              if (unknown) {
                inside=poly.contains(i_actual,j_actual);
                unknown=false;
              }
              if (inside) pop_total+=rowData[i-start_i];
            } else unknown=true;
          }
        }
      }
      return pop_total;
    }
  }

  public int[] assignPatches(int node, DataOutputStream dos) {
    int[] results = new int[2];
    results[0]=0;
    results[1]=0;
    final ArrayList<Integer> nodeConfig = nodeConfigs.get(node);
    int polyCount=0;
    int pointer=0;
    while (pointer<nodeConfig.size()) {
      pointer+=2; // Skip name, population
      pointer+=2*nodeConfig.get(pointer).intValue(); // Skip points;
      pointer++;
      polyCount++;
    }
    Polygon[] thePolys = new Polygon[polyCount];
    pointer=0;
    polyCount=0;
    while (pointer<nodeConfig.size()) {
      pointer+=2; // Skip name, population
      int npoints = nodeConfig.get(pointer++).intValue();
      thePolys[polyCount]=new Polygon();
      for (int i=0; i<npoints; i++) {
        thePolys[polyCount].addPoint(nodeConfig.get(pointer++).intValue(),nodeConfig.get(pointer++).intValue());
      }
      polyCount++;
    }

    Polygon p = new Polygon();
    for (int x=0; x<lsd.lsh.ncols; x+=320) {
      for (int y=0; y<lsd.lsh.nrows; y+=320) {
        final int endXLS = Math.min(lsd.lsh.ncols-1,x+319);
        final int endYLS = Math.min(lsd.lsh.nrows-1,y+319);
        p.reset();
        p.addPoint(x,y);
        p.addPoint(endXLS,y);
        p.addPoint(endXLS,endYLS);
        p.addPoint(x,endYLS);
        final long pop = getPop(p);
        if (pop>0) {
          boolean intersects=false;
          for (int poly=0; poly<thePolys.length; poly++) {
            if (thePolys[poly].intersects(x,y,320,320)) {
              intersects=true;
              poly=thePolys.length;
            }
          }
          if (intersects) {
            int[] res = divideConquer(x,y,160,node,20,thePolys,dos);
            results[0]+=res[0];
            results[1]+=res[1];
            res = divideConquer(x+160,y,160,node,20,thePolys,dos);
            results[0]+=res[0];
            results[1]+=res[1];
            res = divideConquer(x+160,y+160,160,node,20,thePolys,dos);
            results[0]+=res[0];
            results[1]+=res[1];
            res = divideConquer(x,y+160,160,node,20,thePolys,dos);
            results[0]+=res[0];
            results[1]+=res[1];

          } else {
            int[] nodeList = getNodesInBox(x,y,320);
            if (nodeList.length==1) {
              writePatch(x,y,320,nodeList[0],(int)pop,dos);
              results[1]++;
            } else if (nodeList.length>1){
              int[] res = divideConquer(x,y,160,node,20,thePolys,dos);
              results[0]+=res[0];
              results[1]+=res[1];
              res = divideConquer(x+160,y,160,node,20,thePolys,dos);
              results[0]+=res[0];
              results[1]+=res[1];
              res = divideConquer(x+160,y+160,160,node,20,thePolys,dos);
              results[0]+=res[0];
              results[1]+=res[1];
              res = divideConquer(x,y+160,160,node,20,thePolys,dos);
              results[0]+=res[0];
              results[1]+=res[1];
            }
          }
        }
      }
    }
    System.out.println("Yellow squares: "+results[0]+", blue squares: "+results[1]);
    return results;
  }

  
  public void createLSIs(String out_stub) {
    final int nodeCount = nodeConfigs.size();
    try {
      DataOutputStream dos; 
      for (int node=0; node<nodeCount; node++) {
        dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(new File(out_stub+"_"+node+".lsi"))));
        dos.writeInt(0); // will be number of local patches
        dos.writeInt(0); // will be number of remote patches
        int[] results = assignPatches(node,dos);
        dos.writeInt(Integer.reverseBytes(-1));
        dos.flush();
        dos.close();
        RandomAccessFile r = new RandomAccessFile(out_stub+"_"+node+".lsi","rw");
        r.seek(0);
        r.writeInt(Integer.reverseBytes(results[0]));
        r.seek(4);
        r.writeInt(Integer.reverseBytes(results[1]));
        r.close();
      } 
    } catch (Exception ex) { ex.printStackTrace(); }
  }
  
  public void loadConfig(String filename) {
    try {
      DataInputStream dis = new DataInputStream(new FileInputStream(new File(filename)));
      int nodeCount = dis.readInt();
      for (int i=0; i<nodeCount; i++) {
        ArrayList<Integer> nodeConfig = new ArrayList<Integer>();
        final int recordSize = dis.readInt();
        while (nodeConfig.size()<recordSize) {
          final int titleLength = dis.readInt();
          for (int k=0; k<titleLength; k++) dis.readChar();   // Skip the title - not concerned here.
          dis.readLong();  // Also skip population.
          final int points = dis.readInt();
          nodeConfig.add(new Integer(points));
          for (int k=0; k<points; k++) {
            nodeConfig.add(new Integer(dis.readInt()));
            nodeConfig.add(new Integer(dis.readInt()));            
          }
        }
        nodeConfigs.add(nodeConfig);
      }
      dis.close();
    } catch (Exception e) {e.printStackTrace(); }
  }

  
  public void init(String lsStub) {
    lsd=new LandScanData(lsStub);
  }
    


  public static void printUsage() {
    System.out.println("Usage: java -Xmx1g LSICreate \\\\path\\to\\landscan.flt \\\\path\\to\\nodes.lsc \\\\path\\to\\outputfilestub");
    System.out.println("------------------------------------------------------------------------------------------------");
    System.out.println(" (1) Path to a Landscan-style .FLT file. Assumes the accompanying .HDR is in the same folder.");
    System.out.println("     Then as many as you like of:-");
    System.out.println(" (2) Path to an LSC file - this is the node boundary file created by the LSC-Creator. (not public yet).");
    System.out.println(" (3) Output path and filename stub - files will have _n.lsi appended for each node n.");
    System.exit(1);
  }
  
  public static void main(String[] args) {
    if ((args.length<3) || (args.length%2==0)) printUsage();
    else {
      int argno=0;
      boolean init=false;
      LSICreate lc = new LSICreate();
      String landscan_stub = args[argno++];
      String lsc_file = args[argno++];
      if (!landscan_stub.toUpperCase().endsWith(".FLT")) {
        System.out.println("Error - first argument should be path to landscan file ending in .flt\n\n");
        printUsage();
      }
      else {
        landscan_stub=landscan_stub.substring(0,landscan_stub.length()-4);
        if (!new File(landscan_stub+".flt").exists()) {
          System.out.println("Error - couldn't find pop file:"+landscan_stub+".flt\n\n");
          printUsage();
        }
        else if (!new File(landscan_stub+".hdr").exists()) { System.out.println("Error - couldn't find pop header: "+landscan_stub+".hdr\n\n"); argno++; }
        else if (!new File(lsc_file).exists()) { System.out.println("Error - couldn't find LSC file: "+lsc_file); argno++; }
        else {
          String out_file = args[argno++];
          if (!init) {
            lc.init(landscan_stub);
            init=true;
          }
          lc.loadConfig(lsc_file);
          lc.createLSIs(out_file);
        }
      }
    }
  }
   
  
  class LandScanData {
    LandScanHeader lsh;
    int[][] index_int;
    int[][] data_int;
    
    public int getData(int x, int y) {
      try {
        int i=0;
        while ((i<index_int[y].length) && (index_int[y][i]<x)) i++;
        if ((i<index_int[y].length) && (index_int[y][i]==x)) return data_int[y][i];
        else return 0;
      } catch (Exception e) {System.out.println("x="+x+", y="+y); }
      return -1;
    }
    
    public void getRowData(int[] rowData, int y) {
      Arrays.fill(rowData,0);
      int index=0;
      for (int x=0; x<lsh.ncols; x++)
        if (index_int[y].length>index) 
          if (index_int[y][index]==x) rowData[x]=data_int[y][index++];
          
    }
    
    public void getRowData(int[] rowData, int y, int x1, int x2) {
      // Get partial row...
      // Find first mention of an x between x1 and x2, in index_int[y][]
      
      Arrays.fill(rowData,0);
      int x=0;
      if (index_int[y].length>0) {
        while ((x<index_int[y].length) && (index_int[y][x]<x1)) x++;
        while ((x<index_int[y].length) && (index_int[y][x]<=x2)) {
          rowData[index_int[y][x]-x1]=data_int[y][x];
          x++;
        }
      }
    }
      
    
    public LandScanData(String s) {
      loadData(s);
    }
    
    public void loadData(String stub) {
      lsh = new LandScanHeader(stub+".hdr");
      index_int = new int[lsh.nrows][];
      data_int = new int[lsh.nrows][];
      loadFullData(stub);
    }
    
    public void loadFullData(String stub) {
      ArrayList<Integer> rowInt = new ArrayList<Integer>();
      ArrayList<Integer> indexInt = new ArrayList<Integer>();
      try {
        DataInputStream dis = new DataInputStream(new BufferedInputStream(new FileInputStream(stub+".flt")));
        for (int j=0; j<lsh.nrows; j++) {
          for (int i=0; i<lsh.ncols; i++) {
            final int data = Math.round(Float.intBitsToFloat(Integer.reverseBytes(dis.readInt())));
            if (data>0) {
              rowInt.add(new Integer(data));
              indexInt.add(new Integer(i));
            }
          }
          
          final int sizeInt = rowInt.size();
          index_int[j]=new int[sizeInt];
          data_int[j]=new int[sizeInt];
          for (int i=0; i<sizeInt; i++) {
            index_int[j][i]=indexInt.get(i).intValue();
            data_int[j][i]=rowInt.get(i).intValue();
          }
          rowInt.clear();
          indexInt.clear();

        }
        dis.close();
      } catch (Exception e) { e.printStackTrace(); }
    }
  }

  class LandScanHeader {
    int ncols;
    int nrows;
    double xllcorner;
    double yllcorner;
    double xllcenter;
    double yllcenter;
    double cellsize;
    double NODATA_value;
    byte byteorder;
    String filename;
    
    public LandScanHeader(String filename) {
      xllcenter=Double.MAX_VALUE;
      yllcenter=Double.MAX_VALUE;
      xllcorner=Double.MAX_VALUE;
      yllcorner=Double.MAX_VALUE;
      try {
        BufferedReader br = new BufferedReader(new FileReader(filename));
        String line = br.readLine();
        while (line!=null) {
          parseLine(line);
          line = br.readLine();
        }
        br.close();
      } catch (Exception e) { e.printStackTrace(); }
      if (xllcenter==Double.MAX_VALUE) xllcenter = xllcorner+(cellsize/2.0);
      if (yllcenter==Double.MAX_VALUE) yllcenter = yllcorner+((cellsize * (nrows-1.0))+cellsize/2.0);
      if (xllcorner==Double.MAX_VALUE) xllcorner = xllcenter-(cellsize/2.0);
      if (yllcorner==Double.MAX_VALUE) yllcorner = yllcenter-((cellsize * (nrows-1.0))+cellsize/2.0);
    }
    
    public void parseLine(String s) {
      if (s.startsWith("ncols")) ncols = Integer.parseInt(s.substring(5).trim());
      else if (s.startsWith("nrows")) nrows = Integer.parseInt(s.substring(5).trim());      
      else if (s.startsWith("xllcorner")) xllcorner = Double.parseDouble(s.substring(10).trim());      
      else if (s.startsWith("yllcorner")) yllcorner = Double.parseDouble(s.substring(10).trim());
      else if (s.startsWith("xllcenter")) xllcenter = Double.parseDouble(s.substring(10).trim());      
      else if (s.startsWith("yllcenter")) yllcenter = Double.parseDouble(s.substring(10).trim());  
      else if (s.startsWith("cellsize")) cellsize = Double.parseDouble(s.substring(9).trim());
      else if (s.startsWith("NODATA_value")) NODATA_value = Double.parseDouble(s.substring(13).trim());
    }

}

  
}
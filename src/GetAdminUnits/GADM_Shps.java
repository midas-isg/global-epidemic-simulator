/* GADM_Shps.java, part of the Global Epidemic Simulation v1.0 BETA
/* Download GADM SHP files, reduce, combines, and create raster version
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
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.RandomAccessFile;
import java.net.URL;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// This tool converts SHP.zip files from GADM into simpler .bin files for the Global Sim Kit, packaging
// them into a single ZIP file of ISO_adm[level].bin
// It scales the polygons down to landscan resolution as sometimes they are prohibitively large.
// The tool also creates the raster country file, in the ESRI format.
//////////////////////////////////////////////////////////////////////////////////////////////////////////

public class GADM_Shps {
  
  // This is a list of 231 ISO/country pairs. The index (0..231) matches with the index provided to
  // the scripts for creating the synthetic population, provided that the raster country file also
  // uses those ids. (Which this initialisation tool also does)
  
  static final String[][] iso_country = new String[][] {
    new String[]{"ABW","Aruba"},new String[]{"AFG","Afghanistan"},new String[]{"AGO","Angola"},new String[]{"AIA","Anguilla"},new String[]{"ALB","Albania"},new String[]{"AND","Andorra"},
    new String[]{"ANT","Netherlands Antilles"},new String[]{"ARE","United Arab Emirates"},new String[]{"ARG","Argentina"},new String[]{"ARM","Armenia"},new String[]{"ASM","American Samoa"},
    new String[]{"ATG","Antigua and Barbuda"},new String[]{"AUS","Australia"},new String[]{"AUT","Austria"},new String[]{"AZE","Azerbaijan"},new String[]{"BDI","Burundi"},
    new String[]{"BEL","Belgium"},new String[]{"BEN","Benin"},new String[]{"BFA","Burkina Faso"},new String[]{"BGD","Bangladesh"},new String[]{"BGR","Bulgaria"},new String[]{"BHR","Bahrain"},
    new String[]{"BHS","Bahamas"},new String[]{"BIH","Bosnia and Herzegovina"},new String[]{"BLR","Belarus"},new String[]{"BLZ","Belize"},new String[]{"BMU","Bermuda"},
    new String[]{"BOL","Bolivia"},new String[]{"BRA","Brazil"},new String[]{"BRB","Barbados"},new String[]{"BRN","Brunei Darussalam"},new String[]{"BTN","Bhutan"},new String[]{"BWA","Botswana"},
    new String[]{"CAF","Central African Republic"},new String[]{"CAN","Canada"},new String[]{"CHE","Switzerland"},new String[]{"CHL","Chile"},new String[]{"CHN","China"},
    new String[]{"CIV","Cote d'Ivoire Cote d'Ivoire"},new String[]{"CMR","Cameroon"},new String[]{"COD","Congo. Democratic Republic of the"},new String[]{"COG","Congo"},
    new String[]{"COK","Cook Islands"},new String[]{"COL","Colombia"},new String[]{"COM","Comoros"},new String[]{"CPV","Cape Verde"},new String[]{"CRI","Costa Rica"},new String[]{"CUB","Cuba"},
    new String[]{"CYM","Cayman Islands"},new String[]{"CYP","Cyprus"},new String[]{"CZE","Czech Republic"},new String[]{"DEU","Germany"},new String[]{"DJI","Djibouti"},
    new String[]{"DMA","Dominica"},new String[]{"DNK","Denmark"},new String[]{"DOM","Dominican Republic"},new String[]{"DZA","Algeria"},new String[]{"ECU","Ecuador"},new String[]{"EGY","Egypt"},
    new String[]{"ERI","Eritrea"},new String[]{"ESP","Spain"},new String[]{"EST","Estonia"},new String[]{"ETH","Ethiopia"},new String[]{"FIN","Finland"},new String[]{"FJI","Fiji"},
    new String[]{"FLK","Falkland Islands (Malvinas)"},new String[]{"FRA","France"},new String[]{"FRO","Faroe Islands"},new String[]{"FSM","Micronesia. Federated States of"},
    new String[]{"GAB","Gabon"},new String[]{"GBR","United Kingdom"},new String[]{"GEO","Georgia"},new String[]{"GGY","Guernsey"},new String[]{"GHA","Ghana"},new String[]{"GIB","Gibraltar"},
    new String[]{"GIN","Guinea"},new String[]{"GLP","Guadeloupe"},new String[]{"GMB","Gambia"},new String[]{"GNB","Guinea-Bissau"},new String[]{"GNQ","Equatorial Guinea"},
    new String[]{"GRC","Greece"},new String[]{"GRD","Grenada"},new String[]{"GRL","Greenland"},new String[]{"GTM","Guatemala"},new String[]{"GUF","French Guiana"},new String[]{"GUM","Guam"},
    new String[]{"GUY","Guyana"},new String[]{"HKG","Hong Kong"},new String[]{"HND","Honduras"},new String[]{"HRV","Croatia"},new String[]{"HTI","Haiti"},new String[]{"HUN","Hungary"},
    new String[]{"IDN","Indonesia"},new String[]{"IMN","Isle of Man"},new String[]{"IND","India"},new String[]{"IRL","Ireland"},new String[]{"IRN","Iran. Islamic Republic of"},
    new String[]{"IRQ","Iraq"},new String[]{"ISL","Iceland"},new String[]{"ISR","Israel"},new String[]{"ITA","Italy"},new String[]{"JAM","Jamaica"},new String[]{"JEY","Jersey"},
    new String[]{"JOR","Jordan"},new String[]{"JPN","Japan"},new String[]{"KAZ","Kazakhstan"},new String[]{"KEN","Kenya"},new String[]{"KGZ","Kyrgyzstan"},new String[]{"KHM","Cambodia"},
    new String[]{"KIR","Kiribati"},new String[]{"KNA","Saint Kitts and Nevis"},new String[]{"KOR","Korea. Republic of"},new String[]{"KWT","Kuwait"},
    new String[]{"LAO","Lao People's Democratic Republic"},new String[]{"LBN","Lebanon"},new String[]{"LBR","Liberia"},new String[]{"LBY","Libyan Arab Jamahiriya"},
    new String[]{"LCA","Saint Lucia"},new String[]{"LIE","Liechtenstein"},new String[]{"LKA","Sri Lanka"},new String[]{"LSO","Lesotho"},new String[]{"LTU","Lithuania"},
    new String[]{"LUX","Luxembourg"},new String[]{"LVA","Latvia"},new String[]{"MAC","Macao"},new String[]{"MAR","Morocco"},new String[]{"MCO","Monaco"},new String[]{"MDA","Moldova. Republic of"},
    new String[]{"MDG","Madagascar"},new String[]{"MDV","Maldives"},new String[]{"MEX","Mexico"},new String[]{"MHL","Marshall Islands"},
    new String[]{"MKD","Macedonia. the former Yugoslav Republic of"},new String[]{"MLI","Mali"},new String[]{"MLT","Malta"},new String[]{"MMR","Myanmar"},new String[]{"MNG","Mongolia"},
    new String[]{"MNP","Northern Mariana Islands"},new String[]{"MOZ","Mozambique"},new String[]{"MRT","Mauritania"},new String[]{"MSR","Montserrat"},new String[]{"MTQ","Martinique"},
    new String[]{"MUS","Mauritius"},new String[]{"MWI","Malawi"},new String[]{"MYS","Malaysia"},new String[]{"MYT","Mayotte"},new String[]{"NAM","Namibia"},new String[]{"NCL","New Caledonia"},
    new String[]{"NER","Niger"},new String[]{"NFK","Norfolk Island"},new String[]{"NGA","Nigeria"},new String[]{"NIC","Nicaragua"},new String[]{"NIU","Niue"},new String[]{"NLD","Netherlands"},
    new String[]{"NOR","Norway"},new String[]{"NPL","Nepal"},new String[]{"NRU","Nauru"},new String[]{"NZL","New Zealand"},new String[]{"OMN","Oman"},new String[]{"PAK","Pakistan"},
    new String[]{"PAN","Panama"},new String[]{"PCN","Pitcairn"},new String[]{"PER","Peru"},new String[]{"PHL","Philippines"},new String[]{"PLW","Palau"},new String[]{"PNG","Papua New Guinea"},
    new String[]{"POL","Poland"},new String[]{"PRI","Puerto Rico"},new String[]{"PRK","Korea. Democratic People's Republic of"},new String[]{"PRT","Portugal"},new String[]{"PRY","Paraguay"},
    new String[]{"PSE","Palestinian Territory. Occupied"},new String[]{"PYF","French Polynesia"},new String[]{"QAT","Qatar"},new String[]{"REU","Reunion Reunion"},new String[]{"ROU","Romania"},
    new String[]{"RUS","Russian Federation"},new String[]{"RWA","Rwanda"},new String[]{"SAU","Saudi Arabia"},new String[]{"SCG","Serbia"},new String[]{"SDN","Sudan"},new String[]{"SEN","Senegal"},
    new String[]{"SGP","Singapore"},new String[]{"SHN","Saint Helena"},new String[]{"SJM","Svalbard and Jan Mayen"},new String[]{"SLB","Solomon Islands"},new String[]{"SLE","Sierra Leone"},
    new String[]{"SLV","El Salvador"},new String[]{"SMR","San Marino"},new String[]{"SOM","Somalia"},new String[]{"SPM","Saint Pierre and Miquelon"},new String[]{"STP","Sao Tome and Principe"},
    new String[]{"SUR","Suriname"},new String[]{"SVK","Slovakia"},new String[]{"SVN","Slovenia"},new String[]{"SWE","Sweden"},new String[]{"SWZ","Swaziland"},new String[]{"SYC","Seychelles"},
    new String[]{"SYR","Syrian Arab Republic"},new String[]{"TCA","Turks and Caicos Islands"},new String[]{"TCD","Chad"},new String[]{"TGO","Togo"},new String[]{"THA","Thailand"},
    new String[]{"TJK","Tajikistan"},new String[]{"TKL","Tokelau"},new String[]{"TKM","Turkmenistan"},new String[]{"TLS","Timor-Leste"},new String[]{"TON","Tonga"},
    new String[]{"TTO","Trinidad and Tobago"},new String[]{"TUN","Tunisia"},new String[]{"TUR","Turkey"},new String[]{"TUV","Tuvalu"},new String[]{"TWN","Taiwan. Province of China"},
    new String[]{"TZA","Tanzania. United Republic of"},new String[]{"UGA","Uganda"},new String[]{"UKR","Ukraine"},new String[]{"URY","Uruguay"},new String[]{"USA","United States"},
    new String[]{"UZB","Uzbekistan"},new String[]{"VCT","Saint Vincent and the Grenadines"},new String[]{"VEN","Venezuela. Bolivarian Republic of"},new String[]{"VGB","Virgin Islands. British"},
    new String[]{"VIR","Virgin Islands. U.S."},new String[]{"VNM","Viet Nam"},new String[]{"VUT","Vanuatu"},new String[]{"WLF","Wallis and Futuna"},new String[]{"WSM","Samoa"},
    new String[]{"YEM","Yemen"},new String[]{"ZAF","South Africa"},new String[]{"ZMB","Zambia"},new String[]{"ZWE","Zimbabwe"}};

  
  // Some switches
  boolean verbose = false;   // Turn this on for more details analysis output.
  
  // State
  
  
  ArrayList<String> f_names = new ArrayList<String>();
  ArrayList<Character> f_types = new ArrayList<Character>();
  ArrayList<Integer> f_lengths = new ArrayList<Integer>();
  ArrayList<String> dbfEntry = new ArrayList<String>();
  int no_recs;
  
  // DB indexes
  
    
  
  // Output text files
  
  private static final byte ISO=0;
  private static final byte N0=1;
  private static final byte I0=2;
  private static final byte N1=3;
  private static final byte I1=4;
  private static final byte N2=5;
  private static final byte I2=6;
  private static final byte N3=7;
  private static final byte I3=8;
  private static final byte N4=9;
  private static final byte I4=10;  
  private static final byte N5=11;
  private static final byte I5=12;  
  
  
  public void croak(String s) {
    try { throw new Exception(s); }
    catch (Exception e) { e.printStackTrace(); }
    System.exit(0);
  }
  
  public void writeString(DataOutputStream dos, String s) throws Exception{
    dos.writeInt(s.length());
    for (int i=0; i<s.length(); i++) dos.writeChar(s.charAt(i));
  }
  
  public void parseShapeFile(DataInputStream dbf,String iso,String in_file,String out_file,byte level,PrintWriter[] pw,byte[] C_ID, byte[] C_N) throws Exception {
    Polygon p = new Polygon();
    DataInputStream shp = new DataInputStream(new BufferedInputStream(new FileInputStream(in_file)));
    DataOutputStream bin = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(out_file)));
    writeString(bin,iso);
    bin.writeByte(level);
    bin.writeInt(no_recs);
    
    int file_code = shp.readInt();          // File code - hex value 0x0000270a = 9994 decimal
    if (file_code!=9994) croak("File code is not 9994 (hex 270A)");
    for (int i=0; i<5; i++) shp.readInt();  // 5 unused bytes
    int file_size = 2*shp.readInt();          // File size obviously - but measured in 16-bit words. I double it for ytes.
    int version = Integer.reverseBytes(shp.readInt());    // Version. Who knows why little-endian takes over here...
    int shape_type = Integer.reverseBytes(shp.readInt()); // Shape type;
    if (verbose) System.out.println("SHP File "+in_file+": file_size = "+file_size+", version = "+version+", shape_type = "+shape_type);
    double min_x = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double min_y = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double max_x = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double max_y = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double min_z = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double max_z = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double min_m = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    double max_m = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
    
    if (verbose) System.out.println("X Bounds = "+min_x+"..."+max_x+",  Y Bounds = "+min_y+"..."+max_y);
    if (verbose) System.out.println("Z Bounds = "+min_z+"..."+max_z+",  M Bounds = "+min_m+"..."+max_m);
    
    int file_pointer = 100; // Header is exactly 100 bytes. 9 ints, 6 doubles = 9*4 + 8*8 = 36 + 64 = 100
    
    while (file_pointer<file_size) {
      // Read record header
      nextDBF(dbf);
      addDBRow(iso,pw,C_ID,C_N);
      writeString(bin,dbfEntry.get(C_N[level]).trim());
      for (int i=1; i<=level; i++) bin.writeInt(Integer.parseInt(dbfEntry.get(C_ID[i]).trim()));
      
      int rec_no = shp.readInt();
      int rec_length = shp.readInt()*2;   // Again, I just prefer bytes to 16-bit words.
      file_pointer+=8;
      if (verbose) System.out.println("Rec No="+rec_no+", No. bytes = "+rec_length);
      
      
      // Write the BIN file stuff
      
      
      
      // Actual record
      
      int rec_pointer=0;   // Keep track of progress through variable length record
      while (rec_pointer<rec_length) {
        int rec_shape_type = Integer.reverseBytes(shp.readInt());
        rec_pointer+=4;
        file_pointer+=4;
      
        if (rec_shape_type==5) {   // Polygon. MBR, Number of parts, Number of points, Parts, Points
          /*double rec_min_x = */Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));    // Bounding box comes next
          /*double rec_min_y = */Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
          /*double rec_max_x = */Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
          /*double rec_max_y = */Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
          rec_pointer+=32;
          file_pointer+=32;

          if (verbose) System.out.println("REC "+rec_no+": X Bounds = "+min_x+"..."+max_x+",  Y Bounds = "+min_y+"..."+max_y);
          if (verbose) System.out.println("REC "+rec_no+": Z Bounds = "+min_z+"..."+max_z+",  M Bounds = "+min_m+"..."+max_m);
        
          int no_parts = Integer.reverseBytes(shp.readInt());     // Number of separate "polygons"
          bin.writeInt(no_parts);
          int no_points = Integer.reverseBytes(shp.readInt());    // Number of points
          if (verbose) System.out.println("REC "+rec_no+": no_parts = "+no_parts+", no_points = "+no_points);
          
          rec_pointer+=8;
          file_pointer+=8;

          int[] part_start = new int[no_parts]; 
          for (int i=0; i<no_parts; i++) part_start[i]=Integer.reverseBytes(shp.readInt()); // Indexes - first point.
          
          if (verbose) {
            System.out.print("REC "+rec_no+": part indexes = ");
            for (int i=0; i<no_parts; i++) System.out.print(part_start[i]+",");
            System.out.println("");
          }
          
          rec_pointer+=(4*no_parts);
          file_pointer+=(4*no_parts);
          
        
          for (int i=0; i<no_parts; i++) {
            int no_points_in_part=0;
            if (i<no_parts-1) no_points_in_part=part_start[i+1]-part_start[i];
            else no_points_in_part=no_points-part_start[i];
            p.reset();
            if (verbose) System.out.println("REC "+rec_no+": part "+i+", points in part = "+no_points_in_part);
            for (int j=0; j<no_points_in_part; j++) {
              double p_x = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));    
              double p_y = Double.longBitsToDouble(Long.reverseBytes(shp.readLong()));
              int p_x_int = (int) Math.round(120.0*p_x);
              int p_y_int = (int) Math.round(120.0*p_y);
              if (p.npoints==0) p.addPoint(p_x_int,p_y_int);
              else if ((p_x_int!=p.xpoints[p.npoints-1]) || (p_y_int!=p.ypoints[p.npoints-1])) p.addPoint(p_x_int,p_y_int);
              //double p_m = Double.longBitsToDouble(Long.reverseBytes(dis.readLong()));
              if (verbose) System.out.println("REC "+rec_no+": part "+i+", point "+j+": ("+p_x+","+p_y+")");
              rec_pointer+=16;
              file_pointer+=16;
            }
            bin.writeInt(p.npoints);
            for (int j=0; j<p.npoints; j++) {
              bin.writeInt(p.xpoints[j]);
              bin.writeInt(p.ypoints[j]);
            }
          }
        } else croak("Shape type "+rec_shape_type+" not implemented");
      }
    }
    bin.flush();
    bin.close();
    shp.close();
  }
  
  public void processDBFile(String data_path,String iso, byte level,PrintWriter[] pw) throws Exception {
    
    String file = data_path+iso+"_adm"+level+".dbf";
    f_names.clear();
    f_types.clear();
    f_lengths.clear();
    DataInputStream dbf = new DataInputStream(new BufferedInputStream(new FileInputStream(file)));
    byte file_type = dbf.readByte();
    if (verbose) System.out.println("File type: "+file_type);
    byte mod_yy = dbf.readByte();
    byte mod_mm = dbf.readByte();
    byte mod_dd = dbf.readByte();
    if (verbose) System.out.println("Last modified: "+(1900+mod_yy)+"-"+mod_mm+"-"+mod_dd);
    no_recs = Integer.reverseBytes(dbf.readInt());
    short first_rec = Short.reverseBytes(dbf.readShort());
    int rec_length = Short.reverseBytes(dbf.readShort());
    for (int i=0; i<16; i++) dbf.readByte();
    byte flags = dbf.readByte();
    byte codepage = dbf.readByte();
    if (verbose) System.out.println("No. records = "+no_recs+", first rec starts at offset "+first_rec+", record length = "+rec_length+", Table flags = "+flags+", Codepage = "+codepage);
    for (int i=0; i<2; i++) dbf.readByte();
    char starter = (char) dbf.readByte();
    while (starter!=0x0D) {
      char[] title = new char[11];
      title[0]=(starter==0)?' ':(char)starter;
      for (int i=1; i<=10; i++) {
        char ch = (char) dbf.readByte();
        title[i]=(ch==0)?' ':ch;
      }
      char type = (char) dbf.readByte();
      int displ = Integer.reverseBytes(dbf.readInt());
      int field_length = dbf.readByte();
      if (field_length<0) field_length=256+field_length;
      byte dec_places = dbf.readByte();
      byte field_flags = dbf.readByte();
      int auto_incr_next = Integer.reverseBytes(dbf.readInt());
      int auto_incr_step = dbf.readByte();
      dbf.readDouble(); // Skip 8 bytes
      if (verbose) System.out.println("Field: "+new String(title)+", Format "+type+", displ="+displ+", field_length="+field_length+", dec places = "+dec_places+", field_flags="+field_flags+", incr_next = "+auto_incr_next+", auto step = "+auto_incr_step);
      f_names.add(new String(title));
      f_types.add(new Character(type));
      f_lengths.add(new Integer(field_length));
      starter = (char) dbf.readByte();
    }
    byte[] C_ID = new byte[6];
    byte[] C_N = new byte[6];
    for (int i=0; i<6; i++) { C_ID[i]=-1; C_N[i]=-1; }
    for (byte i=0; i<f_names.size(); i++) {
      for (int j=0; j<6; j++) {
        if (f_names.get(i).trim().equals("ID_"+j)) C_ID[j]=i;
        else if (f_names.get(i).trim().equals("NAME_"+j)) C_N[j]=i;
      }
      if (f_names.get(i).trim().equals("NAME_ENGLI")) C_N[0]=i;
      else if (f_names.get(i).trim().equals("GADMID")) C_ID[0]=i;
    }
    
    parseShapeFile(dbf,iso,data_path+iso+"_adm"+level+".shp",data_path+iso+"_adm"+level+".bin",level,pw,C_ID,C_N);
    dbf.close();
  }
  
  public void nextDBF(DataInputStream dbf) throws Exception {
    dbfEntry.clear();
    /*byte del = */dbf.readByte();
    for (int j=0; j<f_names.size(); j++) {
      char type = f_types.get(j).charValue();
      int length = f_lengths.get(j).intValue();
      if ((type=='C') || (type=='F') || (type=='N')) {
        char[] string = new char[length];
        for (int k=0; k<length; k++) {
          char ch = (char) dbf.readByte();
          string[k]=(ch==0)?' ':ch;
        }
        dbfEntry.add(new String(string));
        if (verbose) System.out.println("  "+f_names.get(j)+" = "+new String(string));
      } else System.out.println("Error in NextDBF");
    }
    
  }
  
  public void addDBRow(String iso,PrintWriter[] pw,byte[] C_ID, byte[] C_N) {
    String country="";
    pw[ISO].println(iso);
    pw[I0].println(dbfEntry.get(C_ID[0]).trim());
    for (int i=0; i<iso_country.length; i++) {
      if (iso_country[i][0].equals(iso)) {
        pw[N0].println(iso_country[i][1]);
        i=iso_country.length+1;
      }
    }
    pw[N0].println(country);
    if (C_ID[1]==-1) {
      pw[N1].println();
      pw[I1].println(-1);
    } else {
      pw[N1].println(dbfEntry.get(C_N[1]).trim());
      pw[I1].println(dbfEntry.get(C_ID[1]).trim());
    }
    if (C_ID[2]==-1) {
      pw[N2].println();
      pw[I2].println(-1);
    } else {
      pw[N2].println(dbfEntry.get(C_N[2]).trim());
      pw[I2].println(dbfEntry.get(C_ID[2]).trim());
    }
    if (C_ID[3]==-1) {
      pw[N3].println();
      pw[I3].println(-1);
    } else {
      pw[N3].println(dbfEntry.get(C_N[3]).trim());
      pw[I3].println(dbfEntry.get(C_ID[3]).trim());
    }
    if (C_ID[4]==-1) {
      pw[N4].println();
      pw[I4].println(-1);
    } else {
      pw[N4].println(dbfEntry.get(C_N[4]).trim());
      pw[I4].println(dbfEntry.get(C_ID[4]).trim());
    }
    if (C_ID[5]==-1) {
      pw[N5].println();
      pw[I5].println(-1);
    } else {
      pw[N5].println(dbfEntry.get(C_N[5]).trim());
      pw[I5].println(dbfEntry.get(C_ID[5]).trim());
    }
  }
  
  
  
  public void download(String url, String file) {
    try {
      java.io.BufferedInputStream in = new java.io.BufferedInputStream(new URL(url).openStream());
      java.io.FileOutputStream fos = new java.io.FileOutputStream(file);
      java.io.BufferedOutputStream bout = new BufferedOutputStream(fos,1024);
      byte[] data = new byte[1024];
      int x=0;
      while((x=in.read(data,0,1024))>=0) {
        bout.write(data,0,x);
      }
      bout.close();
      in.close();
    } catch (Exception e) {}
  }
  
  public void downloadAll(String http_path,String data_path) {
    try {
      for (int i=0; i<iso_country.length; i++) {
        System.out.println("Downloading "+http_path+iso_country[i][0]+"_adm.zip"+" -> "+data_path+iso_country[i][0]+"_adm.zip");
        download(http_path+iso_country[i][0]+"_adm.zip",data_path+iso_country[i][0]+"_adm.zip");
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  
  
  static final int BUFFER = 2048;
  final byte data[] = new byte[BUFFER];     
  
  public byte getMaxLevel(String iso, String zipfile) throws Exception {
    byte result=0;
    ZipFile zf = new ZipFile(zipfile);
    if (zf.getEntry(iso+"_adm5.shp")!=null) result=5;
    else if (zf.getEntry(iso+"_adm4.shp")!=null) result=4;
    else if (zf.getEntry(iso+"_adm3.shp")!=null) result=3;
    else if (zf.getEntry(iso+"_adm2.shp")!=null) result=2;
    else if (zf.getEntry(iso+"_adm1.shp")!=null) result=1;
    zf.close();
    return result;
  }
  
  public void extract(String zipfile, String file, String out_file) {
    try {
      BufferedInputStream is = null;
      ZipFile zfile = new ZipFile(zipfile);
      ZipEntry entry = zfile.getEntry(file);
      is = new BufferedInputStream(zfile.getInputStream(entry));
      int count;
      FileOutputStream fos = new FileOutputStream(out_file);
      boolean done=false;
      while (!done) {
        count = is.read(data,0,BUFFER);
        if (count==-1) done=true;
        else {
          fos.write(data,0,count);
        }
      }
      fos.flush();
      fos.close();
      is.close();
      zfile.close();
    } catch(Exception e) { e.printStackTrace(); }
  }
  
  public void extract(String data_path,String iso,byte level) {
    extract(data_path+iso+"_adm.zip",iso+"_adm"+level+".shp",data_path+iso+"_adm"+level+".shp");
    extract(data_path+iso+"_adm.zip",iso+"_adm"+level+".dbf",data_path+iso+"_adm"+level+".dbf");
  }
  
  public void convert(String data_path,String iso,byte level,PrintWriter[] pw) throws Exception {
    extract(data_path,iso,level);
    processDBFile(data_path,iso,level,pw);
    new File(data_path+iso+"_adm"+level+".dbf").delete();
    new File(data_path+iso+"_adm"+level+".shp").delete();    
  }
  
  
  public void zipBins(String data_path) {
    try {
      byte[] buf = new byte[4096];
      File[] list = new File(data_path).listFiles();
      ZipOutputStream out = new ZipOutputStream(new FileOutputStream(data_path+File.separator+"bins.zip"));
      for (int i=0; i<list.length; i++) {
        String file = list[i].getPath();
        String just_file = new File(file).getName();
        if (file.endsWith(".bin")) {
          FileInputStream in = new FileInputStream(file);
          out.putNextEntry(new ZipEntry(just_file));
          int len;
          while ((len = in.read(buf)) > 0) {
            out.write(buf, 0, len);
          }
          out.closeEntry();
          in.close();
          new File(file).delete();
        }
      }
      out.close();
    } catch (IOException e) { e.printStackTrace(); }
  }
  
  public void convertAll(String data_path) {
    try {
      PrintWriter[] pw = new PrintWriter[13];
      pw[ISO]=new PrintWriter(new File(data_path+File.separator+"no_shapes_iso.txt"));
      pw[I0]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id0.txt"));
      pw[I1]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id1.txt"));      
      pw[I2]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id2.txt"));
      pw[I3]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id3.txt"));
      pw[I4]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id4.txt"));
      pw[I5]=new PrintWriter(new File(data_path+File.separator+"no_shapes_id5.txt"));
      pw[N0]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name0.txt"));
      pw[N1]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name1.txt"));
      pw[N2]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name2.txt"));
      pw[N3]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name3.txt"));
      pw[N4]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name4.txt"));
      pw[N5]=new PrintWriter(new File(data_path+File.separator+"no_shapes_name5.txt"));      
      
      File[] list = new File(data_path).listFiles();
      for (int i=0; i<list.length; i++) {
        String file = list[i].getPath();
        if (file.endsWith("_adm.zip")) {
          String country = file.substring(file.length()-11,file.length()-8);
          byte max_level = getMaxLevel(country,file);
          for (byte level=0; level<=max_level; level++) {
            System.out.println("Converting "+file+", level "+level);
            convert(data_path,country,(byte)level,pw);
          }
          new File(file).delete();
        }
      }
      for (int i=0; i<13; i++) {
        pw[i].flush();
        pw[i].close();
      }
    } catch (Exception e) {e.printStackTrace(); }
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////
  // Code for creating the MAP file
  
  private static final byte WRITE = 1;
  private static final byte REMEMBER = 2;  
  private static final byte RESTORE = 3;
  
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
  
  public void polygonToAUMap(Polygon p, String file, int code,ArrayList<Integer> memory, byte function) {
    // Convert int code to LSBFirst float
    float f_code = (float) code;
    int r_code = Integer.reverseBytes(Float.floatToRawIntBits(f_code));
    
    int mem_counter=0;
    try {
      RandomAccessFile raf = new RandomAccessFile(file,"rw");
      final Rectangle bounds = p.getBounds();
      final int booleanWidth = (int) Math.ceil((1+bounds.width)/8.0);
      final char[][] booleanGrid = new char[booleanWidth][bounds.height+1];
      int px,py;

      ArrayList<Integer> pixels;    
      for (int i=0; i<p.npoints; i++) {    // For each line segment in polygon
        if (i<p.npoints-1) pixels = getPixels(p.xpoints[i],p.ypoints[i],p.xpoints[i+1],p.ypoints[i+1]);
        else pixels = getPixels(p.xpoints[i],p.ypoints[i],p.xpoints[0],p.ypoints[0]);
        for (int j=0; j<pixels.size(); j+=2) { // For each pixel in line segment
          px = ((Integer) pixels.get(j)).intValue();
          py = ((Integer) pixels.get(j+1)).intValue();      

          final int px_b = px-bounds.x;
          final int py_b = py-bounds.y;
          final int xdiv8 = px_b/8;
          final int xmod8 = px_b%8;

          if (((booleanGrid[xdiv8][py_b]) & (1<<xmod8))==0) {
            booleanGrid[xdiv8][py_b] = (char) ((booleanGrid[xdiv8][py_b]) | (1<< (xmod8))); 
            long ls_x = px+21600;
            long ls_y = 21600-(py+10800);
            raf.seek(((ls_y*43200)+ls_x)*4);
            if (function==WRITE) {
              raf.writeInt(r_code);
            } else if (function==REMEMBER) {
              memory.add(raf.readInt());
            } else if (function==RESTORE) {
              raf.writeInt(memory.get(mem_counter++));
            }
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
          for (i=start_i; i<=end_i; i++) {
            final int i_actual = i+bounds.x;
            if (((booleanGrid[i/8][j]) & (1<< (i%8)))==0) {
              if (unknown) {
                inside=p.contains(i_actual,j_actual);
                unknown=false;
              }
              if (inside) {
                long ls_x = i_actual+21600;
                long ls_y = 21600-(j_actual+10800);
                raf.seek(((ls_y*43200)+ls_x)*4);
                if (function==WRITE) {
                  raf.writeInt(r_code);
                } else if (function==REMEMBER) {
                  memory.add(raf.readInt());
                } else if (function==RESTORE) {
                  raf.writeInt(memory.get(mem_counter++));
                }
              }
            } else unknown=true;
          }
        }
      }
      raf.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void createEmptyAUMap(String file) {
    byte[] minus_one = new byte[5120000];
    for (int i=0; i<5120000; i+=4) {
      minus_one[i]=0x00;
      minus_one[i+1]=0x00;
      minus_one[i+2]=(byte) 0x80;
      minus_one[i+3]=(byte) 0xBF;    // 00,00,80,bf is the IEEE code for (float) -1  - with byte order reversed to LSBFIRST.
    }
    try {
      DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(new File(file))));
      for (int i=0; i<729; i++) dos.write(minus_one); // Write 43200x21600 of reverseBytes(-1.0f) 
      dos.flush();
      dos.close();
      
    } catch (Exception e) { e.printStackTrace(); }
    minus_one=null;
  }
  
  public String readString(DataInputStream dis) throws Exception {
    int len = dis.readInt();
    StringBuffer sb = new StringBuffer();
    for (int i=0; i<len; i++) sb.append(dis.readChar());
    return new String(sb.toString());
  }
  
  public void readPolys(DataInputStream dis, boolean useThis, ArrayList<Polygon> list,byte level) {
    try {
      if (useThis)  {
        int polys = dis.readInt();
        for (int j=0; j<polys; j++) {
          Polygon gpoly = new Polygon();
          int points = dis.readInt();
          int p_no=0;
          
          for (int k=0; k<points; k++) {
            int new_x=dis.readInt();
            int new_y=dis.readInt();
            if (p_no==0) {
              gpoly.addPoint(new_x,new_y);
              p_no++;
            } else {
              boolean ok=true;
              if ((new_x==gpoly.xpoints[p_no-1]) && (new_y==gpoly.ypoints[p_no-1])) ok=false;
              if ((ok) && (p_no>=2)) {
                if ((new_x==gpoly.xpoints[p_no-2]) && (new_y==gpoly.ypoints[p_no-2])) {
                  gpoly.xpoints[p_no-1]=gpoly.xpoints[p_no-2];
                  gpoly.ypoints[p_no-1]=gpoly.ypoints[p_no-2];
                  ok=false;
                }
              }
              if (ok) {
                gpoly.addPoint(new_x,new_y);
                p_no++;
              }
            }
            gpoly.npoints=p_no;
          }
          list.add(gpoly);
        }
      } else {
        int polys = dis.readInt();
        for (int j=0; j<polys; j++) {
          int points = dis.readInt();
          for (int k=0; k<points*2; k++) dis.readInt();
        }
      }
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public void loadPolygons(String gadm_path, String iso, ArrayList<Polygon> list) {   
    try {
      ZipFile zf = new ZipFile(gadm_path+"bins.zip");
      String file = iso+"_adm0.bin";
      ZipEntry ze = zf.getEntry(file);
      DataInputStream dis = new DataInputStream(new BufferedInputStream(zf.getInputStream(ze)));
      readString(dis); // Country
      dis.readByte();  // Level
      int placemarks = dis.readInt();
      for (int i=0; i<placemarks; i++) {
        readString(dis);
        readPolys(dis,true,list,(byte)0);
        i=placemarks;
      } 
      dis.close();
      zf.close();
    } catch (Exception e) { e.printStackTrace(); }
  }
  
  public boolean isAntiClockwise(Polygon p) {
    int sum=((360+p.xpoints[0])-(360+p.xpoints[p.npoints-1]))*(180+p.ypoints[0]+180+p.ypoints[p.npoints-1]);
    for (int i=1; i<p.npoints; i++) {
      sum+=((360+p.xpoints[i])-(360+p.xpoints[i-1]))*(180+p.ypoints[i]+180+p.ypoints[i-1]);
    }
    return (sum<0);
  }
  
  public void makeCBR(String gadm_path, String map_path) {
    try {
      PrintWriter PW = new PrintWriter(new File(map_path+"cbr.hdr"));
      PW.println("ncols         43200");
      PW.println("nrows         21600");
      PW.println("xllcorner     -180");
      PW.println("yllcorner     -90");
      PW.println("cellsize      0.00833333333");
      PW.println("NODATA_value  -9999");
      PW.println("byteorder     LSBFIRST");
      PW.flush();
      PW.close();
    } catch (Exception e) { e.printStackTrace(); }
    map_path+="cbr.flt";
    System.out.println("Creating empty map file...");
    createEmptyAUMap(map_path);

    int done=0;
    for (int i=0; i<iso_country.length; i++) {
      ArrayList<Polygon> polys = new ArrayList<Polygon>();
      loadPolygons(gadm_path,iso_country[i][0],polys);
      done++;
              
      // Check for any anti-clockwise units, and copy data...
      ArrayList<ArrayList<Integer>> anti_memory = new ArrayList<ArrayList<Integer>>();
      for (int j=0; j<polys.size(); j++) {
        Polygon one_poly = polys.get(j);
        if (isAntiClockwise(one_poly)) {
          ArrayList<Integer> remember = new ArrayList<Integer>();
          polygonToAUMap(one_poly,map_path,i,remember,REMEMBER);
          anti_memory.add(remember);
        }
      }
      
      if (anti_memory.size()>0) System.out.println("Found "+anti_memory.size()+" exclusion polygons");
              
      for (int j=0; j<polys.size(); j++) {
        Polygon one_poly = polys.get(j);
        if (!isAntiClockwise(one_poly)) polygonToAUMap(one_poly,map_path,i,null,WRITE);
      }
      
      int mem_counter=0;
      for (int j=0; j<polys.size(); j++) {
        Polygon one_poly = polys.get(j);
        if (isAntiClockwise(one_poly)) {
          ArrayList<Integer> remember = anti_memory.get(mem_counter);
          polygonToAUMap(one_poly,map_path,i,remember,RESTORE);
          remember.clear();
          mem_counter++;
        }
      }
      anti_memory.clear();
      polys.clear();
      System.out.println("Rasterised "+iso_country[i][0]+". ("+(done)+"/"+iso_country.length+")");
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////
  
  
  
  public GADM_Shps() {}
  
  public static void main(String[] args) throws Exception {
       
    GADM_Shps g = new GADM_Shps();
    if (args.length!=3) {
      System.out.println("GADM_Shps - please specify:-");
      System.out.println("            [1] path to output for admin units and support files");
      System.out.println("            [2] path to output for country border (cbr.hdr|flt)");
      System.out.println("            [3] URL for GADM zip files (eg http://biogeo.ucdavis.edu/data/diva/adm/");
    }
    else {
      String data_path=args[0];
      if (!data_path.endsWith(File.separator)) data_path+=File.separator;
      String map_path=args[1];
      if (!map_path.endsWith(File.separator)) map_path+=File.separator;      
      String http_path = args[2];
      if (!http_path.endsWith("/")) http_path+="/";
      g.downloadAll(http_path,data_path);
      g.convertAll(data_path);
      g.zipBins(data_path);
      g.makeCBR(data_path,map_path);
    }
  }
}

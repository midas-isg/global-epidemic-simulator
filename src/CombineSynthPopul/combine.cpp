/* combine.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Standalone tool for creating flatfile from new synthetic population
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


#include "stdio.h"
#include <vector>
#include <iostream>
#include <fstream>

// This squashes all the household files back into a landscan equivalent file. 
// Then give this landscan file back to the PatchMaker utility, to make
// sure all the right patches exist, with the right population in each.

//  Compile with g++ -o combine combine.cpp
//

#ifdef _WIN32
  #include "windows.h"
  #include "io.h"
  #define MM_INT64 __int64
  #ifdef __GNUC__
    #define SEEK fseeko64
    #define TELL ftello64
  #else
    #define SEEK _fseeki64
    #define TELL _ftelli64
  #endif

#else
  #include <stdint.h>
  #define SEEK fseeko64
  #define TELL ftello64
  #define MM_INT64 int64_t
#endif



#define ROWS 20880
#define COLS 43200
#define CHUNK 1000000000

static float** ls_grid;
static std::vector<unsigned char> buf;
MM_INT64 buf_size;
MM_INT64 buf_capacity;

char* readLine(FILE* file) {
  std::vector<char> s;
  char* result;
  int done=0;
  while (done==0) {
    char ch = fgetc(file);
    if (ch==-1) {
      result=NULL;
      done=1;
    } else if (ch=='\n') {
      result = new char[s.size()+1];
      for (unsigned int i=0; i<s.size(); i++) {
        result[i]=s.at(i);
      }
      result[s.size()]='\0';
      done=1;
    } else s.push_back(ch);
  } 
  return result;
}
  
void loadBuf(char* in_file) {

  FILE* f_in = fopen(&in_file[0],"rb");
  SEEK(f_in,0L,SEEK_END);
  MM_INT64 size = TELL(f_in);
  if (buf_capacity<size) {
    buf.clear();
    buf.resize(size);
    buf_capacity=size;
  }
  SEEK(f_in,0L,SEEK_SET);

  // Read file
  
  buf_size=size;
  MM_INT64 fp=0;
  MM_INT64 size_read=0;
  while (size>0) {
    if (size>CHUNK) {
      size_read=fread(&buf[fp],1,CHUNK,f_in);
      size-=size_read;
      fp+=size_read;
    } else {
      size_read=fread(&buf[fp],1,size,f_in);
      size-=size_read;
    }
  }
  fclose(f_in);   
}

short getShort(MM_INT64 offset) {
  short s=*((short*) &buf[offset]);
  return s;
}

int getInt(MM_INT64 offset) {
  int i=*((int*) &buf[offset]);
  return i;
}

double getDouble(MM_INT64 offset) {
  double d=*((double*) &buf[offset]);
  return d;
}

MM_INT64 processHouse(MM_INT64 offset) {
  double lat=getDouble(offset);
  offset+=8;
  double lon=getDouble(offset);
  offset+=8;
  short hosts_in_hh=getShort(offset);
  offset+=6;
  int x = (int) ((lon+180.0)*120.0);
  int y = (int) ((84.0-lat)*120.0);
  ls_grid[x][y]+=hosts_in_hh;
  
  offset+=13*hosts_in_hh;
  return offset;
}

void processHH(char* in_file) {
  loadBuf(in_file);
  printf("L ");
  fflush(stdout);
  short no_age_groups=getShort(0);
  int no_households=getInt(2+(4*no_age_groups));
  MM_INT64 offset=10+(no_age_groups*4);

  for (int house_no=0; house_no<no_households; house_no++) {
    if (house_no % (no_households/5)==0) {
      printf(".");
      fflush(stdout);
    }
    offset=processHouse(offset);
  }
}

void writeLS(char* overlay_file) {
  FILE* f_out = fopen(&overlay_file[0],"wb");
  for (int j=0; j<ROWS; j++) {
    for (int i=0; i<COLS; i++) {
      fwrite(&ls_grid[i][j],4,1,f_out);
    }
  }  
  fclose(f_out);
}

void writeHDR(char* hdr_file) {
  FILE* f_out = fopen(&hdr_file[0],"wb");
  fprintf(f_out,"ncols         43200");
  fprintf(f_out,"nrows         21600");
  fprintf(f_out,"xllcorner     -180");
  fprintf(f_out,"yllcorner     -90");
  fprintf(f_out,"cellsize      0.00833333333");
  fprintf(f_out,"NODATA_value  -9999");
  fprintf(f_out,"byteorder     LSBFIRST");
  fflush(f_out);
  fclose(f_out);
}

int main(int argc, char* argv[])  {
  
  if (argc!=2) {
    printf("COMBINE v1. An internal utility for the Global Epidemic Simulator.\n");
    printf("           Usually the script will be system-generated.\n  Usage: combine script.txt\n");
    printf("     Where script contains:-\n");
    printf("           output FLT file (eg, population.flt)\n");
    printf("           output HDR file (eg, population.hdr)\n");
    printf("           household file 1\n");
    printf("           household file 2...\n");
    printf("     where household files are the *unprocessed* output from the synthetic\n");
    printf("     population creator. This script therefore needs to be run *once* after\n");
    printf("     generating the synthetic population, followed by the patch file maker,\n");
    printf("     and then the Job Editor GUI will work");
    fflush(stdout);
  } else {
    buf_size=0;
    buf_capacity=0;
    printf("COMBINE v1.");
    fflush(stdout);
    
    FILE* script = fopen(argv[1],"r");
    char* flt_file = readLine(script);
    char* hdr_file = readLine(script);
    printf("Preparing to write %s\n",flt_file);
    fflush(stdout);
    
    ls_grid = new float*[COLS];
    for (int i=0; i<COLS; i++) {
      ls_grid[i]=new float[ROWS];
      for (int j=0; j<ROWS; j++) {
        ls_grid[i][j]=0.0f;
      }
    }
    
    while (!feof(script)) {
      char* hh_file_in = readLine(script);
      if (hh_file_in!=NULL) {
        printf("%s ",hh_file_in);
        fflush(stdout);
        processHH(hh_file_in);
        printf("DONE\n");
        fflush(stdout);
      }
    }
    fclose(script);
    writeLS(flt_file);
    printf("Wrote %s\n",flt_file);
    writeHDR(hdr_file);
    printf("Wrote %s\n",hdr_file);

    fflush(stdout);
  }
  return 0;
}
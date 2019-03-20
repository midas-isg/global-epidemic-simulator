/* output.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Functions for updating stats, writing flatfile, images and DB.
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

#include "output.h"

void setInfected(double lon, double lat, int delta,int thread_no, world *wo) {
  const int x = lonToLsIndex(lon)/(43200/PNG_WIDTH);
  const int y = latToLsIndex(lat)/(21600/PNG_HEIGHT);
  if ((x<0) || (x>PNG_WIDTH) || (y<0) || (y>PNG_HEIGHT)) {
    printf("Range error\n");
  } else wo->infected_grid[x][y][thread_no]+=delta;
 
}

void setImmune(double lon, double lat, int delta, int thread_no, world *wo) {
  const int x = lonToLsIndex(lon)/(43200/PNG_WIDTH);
  const int y = latToLsIndex(lat)/(21600/PNG_HEIGHT);
  if ((x<0) || (x>PNG_WIDTH) || (y<0) || (y>PNG_HEIGHT)) {
    printf("Range error\n");
  } else wo->immune_grid[x][y][thread_no]+=delta;
}

void logFlatfile(world *wo) {
  int i;
  unsigned int j;
  for (i=0; i<wo->no_units; i++) {
    unit* u = &wo->a_units[i];
    if (u->log) { // Only log to flatfile if one of the fields is non-zero.
      bool ok=false;
      if ((u->current_symptomatic_inf>0) || (u->current_nonsymptomatic_inf>0)) ok=true;
      else if (u->new_comm_cases>0) ok=true;
      else if (u->new_hh_cases>0) ok=true;
      else if (u->new_comm_infs>0) ok=true;
      else if (u->new_hh_infs>0) ok=true;
      else {
        j=0;
        while ((j<wo->P->no_place_types) && (!ok)) {
          if (u->new_place_cases[j]>0) ok=true;
          else if (u->new_place_infs[j]>0) ok=true;
          j++;
        }
      }

      if (ok) {
        fprintf(wo->ff,"%d\t%d\t%d\t%d\t%d\t",wo->T,i,u->contact_makers[0],u->new_comm_cases,u->new_hh_cases);
        for (j=0; j<wo->P->no_place_types; j++) fprintf(wo->ff,"%d\t",u->new_place_cases[j]);
        fprintf(wo->ff,"%d\t%d\t",u->new_comm_infs,u->new_hh_infs);
        for (j=0; j<wo->P->no_place_types; j++) fprintf(wo->ff,"%d\t",u->new_place_infs[j]);
        fprintf(wo->ff,"%d\t%d\n",u->current_symptomatic_inf,u->current_nonsymptomatic_inf);
       
      }
    }
  }
  fflush(wo->ff);
}

void logDB(world *wo) {
  int* dbdata;
  unsigned int j;
  int i;
  
  for (i=0; i<wo->no_units; i++) {
    if (wo->a_units[i].log) {
      
      bool ok=false;
      if (wo->a_units[i].new_comm_cases>0) ok=true;
      else if (wo->a_units[i].new_hh_cases>0) ok=true;
      else if (wo->a_units[i].new_comm_infs>0) ok=true;
      else if (wo->a_units[i].new_hh_infs>0) ok=true;
      else {
        j=0;
        while ((j<wo->P->no_place_types) && (!ok)) {
          if (wo->a_units[i].new_place_cases[j]>0) ok=true;
          else if (wo->a_units[i].new_place_infs[j]>0) ok=true;
          j++;
        }
      }

      if (ok) {
        j=0;

        dbdata = new int[9+(2*wo->P->no_place_types)];
        dbdata[j++]=(int) wo->T;
        dbdata[j++]=i;
        dbdata[j++]=wo->a_units[i].contact_makers[0];
        dbdata[j++]=wo->a_units[i].new_comm_cases;
        dbdata[j++]=wo->a_units[i].new_comm_infs;
        dbdata[j++]=wo->a_units[i].new_hh_cases;
        dbdata[j++]=wo->a_units[i].new_hh_infs;
        dbdata[j++]=wo->a_units[i].current_symptomatic_inf;
        dbdata[j++]=wo->a_units[i].current_nonsymptomatic_inf;
        for (unsigned int k=0; k<wo->P->no_place_types; k++) {
          dbdata[j++]=wo->a_units[i].new_place_cases[k];
          dbdata[j++]=wo->a_units[i].new_place_infs[k];
        }
        wo->db->InsertRecord(dbdata); // Don't put all-ZERO data into database.
        delete dbdata;
      }
    }
  }
  wo->db->CommitInsChanges();
}

void resetAllUnitStats(world *wo) {
  for (int i=0; i<wo->no_units; i++) {
    unit* u = &wo->a_units[i];
    u->new_comm_cases=0;
    for (unsigned int k=0; k<wo->P->no_place_types; k++) {
      u->new_place_cases[k]=0;
      u->new_place_infs[k]=0;
    }
    u->new_hh_cases=0;
    u->new_comm_infs=0;
    u->new_hh_infs=0;
    u->current_nonsymptomatic_inf=0;
    u->current_symptomatic_inf=0;
    
    for (int j=0; j<10*wo->P->timesteps_per_day; j++) {
      u->hist_comm_cases[j]=0;
      for (unsigned int k=0; k<wo->P->no_place_types; k++) u->hist_place_cases[k][j]=0;
      u->hist_hh_cases[j]=0;
    }
  }
  wo->log_10day_slot=0;
}


void resetUnitStats(world *wo) {         // Reset timestep-based counters.
  for (int i=0; i<wo->no_units; i++) {
    unit* u = &wo->a_units[i];
    for (int j=0; j<wo->thread_count; j++) u->contact_makers[j]=0;
    u->new_comm_cases=0;
    for (unsigned int j=0; j<wo->P->no_place_types; j++) {
      u->new_place_cases[j]=0;
      u->new_place_infs[j]=0;
    }
    u->new_hh_cases=0;
    u->new_comm_infs=0;
    u->new_hh_infs=0;
    
    // NB - don't reset current_nonsymptomatic_inf / current_symptomatic_inf - these are meant to accumulate.
  }
  wo->log_10day_slot++;
  if (wo->log_10day_slot>=10*wo->P->timesteps_per_day) wo->log_10day_slot=0;
}

void statsTimestep(world *wo) {                     
  for (int i=0; i<wo->no_units; i++) {
    for (int j=0; j<wo->thread_count; j++) {
      if (j>0) wo->a_units[i].contact_makers[0]+=wo->a_units[i].contact_makers[j];
      wo->a_units[i].new_comm_cases+=wo->delta_comm_cases[i][j];
      wo->a_units[i].new_comm_infs+=wo->delta_comm_infs[i][j];
      wo->a_units[i].new_hh_cases+=wo->delta_hh_cases[i][j];
      wo->a_units[i].new_hh_infs+=wo->delta_hh_infs[i][j];

      // Don't update wo->a_units[i].current_[non]symptomatic_inf - done in messages.cpp too.

      wo->delta_comm_cases[i][j]=0;
      wo->delta_comm_infs[i][j]=0;
      wo->delta_hh_cases[i][j]=0;
      wo->delta_hh_infs[i][j]=0;

      // Don't set wo->delta_[non]symptomatic_inf either - done in messages.cpp too.
      
      for (unsigned int k=0; k<wo->P->no_place_types; k++) {
        wo->a_units[i].new_place_cases[k]+=wo->delta_place_cases[i][j][k];
        wo->a_units[i].new_place_infs[k]+=wo->delta_place_infs[i][j][k];
        wo->delta_place_cases[i][j][k]=0;
        wo->delta_place_infs[i][j][k]=0;
      }
    }
    
    for (int i=0; i<wo->no_units; i++) {  
      wo->a_units[i].comm_10day_accumulator-=wo->a_units[i].hist_comm_cases[wo->log_10day_slot];
      for (unsigned int k=0; k<wo->P->no_place_types; k++)
  	    wo->a_units[i].place_10day_accumulator[k]-=wo->a_units[i].hist_place_cases[k][wo->log_10day_slot];
	    wo->a_units[i].hh_10day_accumulator-=wo->a_units[i].hist_hh_cases[wo->log_10day_slot];
    
      wo->a_units[i].comm_10day_accumulator+=wo->a_units[i].new_comm_cases;
      for (unsigned int k=0; k<wo->P->no_place_types; k++)
        wo->a_units[i].place_10day_accumulator[k]+=wo->a_units[i].new_place_cases[k];
      wo->a_units[i].hh_10day_accumulator+=wo->a_units[i].new_hh_cases;

      wo->a_units[i].hist_comm_cases[wo->log_10day_slot]=wo->a_units[i].new_comm_cases;
      for (unsigned int k=0; k<wo->P->no_place_types; k++)
        wo->a_units[i].hist_place_cases[k][wo->log_10day_slot]=wo->a_units[i].new_place_cases[k];
      wo->a_units[i].hist_hh_cases[wo->log_10day_slot]=wo->a_units[i].new_hh_cases;
    }
  }
}


void updateImage(world *wo) {
  int thread_no,total_inf, total_imm,x,y,z;

  #pragma omp parallel for private (thread_no,total_inf,total_imm,x,y,z)
  for (thread_no=0; thread_no<wo->thread_count; thread_no++) {
    for (x=thread_no; x<PNG_WIDTH; x+=wo->thread_count) {
      for (y=0; y<PNG_HEIGHT; y++) {
        total_inf=0;
        total_imm=0;
        for (z=0; z<wo->thread_count; z++) {
          total_inf+=wo->infected_grid[x][y][z];
          total_imm+=wo->immune_grid[x][y][z];
        }
          
        if (total_inf+total_imm==0) {
          wo->image[(PNG_WIDTH*y)+x]=(unsigned char) 0;
        } else {
          unsigned char val = (unsigned char) (1+(254.0*((float)total_imm/(total_imm+total_inf))));
          wo->image[(PNG_WIDTH*y)+x]=val;
        }
      }
    }
  }
}

void saveImage(world *wo) {
  string filename;
  filename.append(wo->mv_path);
  filename.append("/");
  filename.append(wo->mv_file);
  std::stringstream noConverter;
  int no = (int) ((float)wo->T/wo->P->timestep_hours);
  if (no<1000) filename.append("0");
  if (no<100) filename.append("0");
  if (no<10) filename.append("0");
  noConverter << (no);
  filename.append(noConverter.str());
  filename.append(".png");
  char* fpointer = &filename[0];

  //create encoder and set settings and info (optional)
  
  LodePNG::Encoder encoder;
  encoder.addPalette((unsigned char)0, (unsigned char)0, (unsigned char)0, (unsigned char)0);
  
  for (int i=0; i<127; i++) encoder.addPalette((unsigned char)255,(unsigned char)i*2,(unsigned char) 0,(unsigned char)255);
  for (int i=0; i<127; i++) encoder.addPalette((unsigned char)(255-(2*i)),(unsigned char)255,(unsigned char)0,(unsigned char)255);

  encoder.addPalette((unsigned char)0,(unsigned char)255,(unsigned char)0,(unsigned char)255);
  
  //both the raw image and the encoded image must get colorType 3 (palette)
  encoder.getInfoPng().color.colorType = 3; //if you comment this line, and store the palette in InfoRaw instead (use getInfoRaw() in the previous lines), then you get the same image in a RGBA PNG.
  encoder.getInfoRaw().color.colorType = 3;

  /* end */
  encoder.getSettings().zlibsettings.windowSize = 2048;

  //encode and save
 
  vector<unsigned char> buffer;
  encoder.encode(buffer, &(wo->image[0]), PNG_WIDTH, PNG_HEIGHT);
  LodePNG::saveFile(buffer, fpointer);
  buffer.clear();
}

void initDB(world* w) {
  if ((w->log_db) && (w->mpi_rank==w->mpi_size-1)) {
    w->db = new DBOps(w->db_server, (char*)"user", (char*)"password"); // Usually not needed
    w->db->Connect();
    w->db->prepareCommands(w->P->no_place_types,w->db_table);

    try {
      w->db->DropTable();
  		w->db->CreateTable();
  	}
  	catch( int Error ){
		  if ( Error != OBJ_EXISTS_ERROR )
        cerr << "Error initialising database\n";
    }
    MPI_Barrier(MPI_COMM_WORLD);  // Make sure DB is created before binding parameters etc.
    w->db->PrepareSQLInsertStmt();
  }
}

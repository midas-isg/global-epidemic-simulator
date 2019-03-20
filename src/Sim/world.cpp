/* world.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Encapsulates globals and main data structures for the world
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


#include "world.h"

world::world(int argc, char *argv[]) {
  // Handle comand-line parameters
  thread_max=0;
  in_path="";
  infectionMod=0;
  con_toggle=0;
  for (int i=1; i<argc; i++) {
    if (strnicmp("/in:",argv[i],4)==0) {               // Specify where params.bin is found.
      in_path=argv[i];
      in_path=in_path.substr(4)+"/";
    } else if (strnicmp("/ompmax:",argv[i],8)==0) {    // Force number of OMP threads
      sscanf(argv[i]+8,"%d", &thread_max);
    }
  }

  P = new params();
  
// Initialise MPI
   #ifdef _USEMPI
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);
    char name[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name(name, &len);
    
    printf("MPI Running. Node %d out of %d is %s\n", mpi_rank, mpi_size,name);
    fflush(stdout); 


//    printf("MPI Init at time %f\n",MPI_Wtime());
  #endif

  // Initialise OpenMP
  
  #ifdef _USE_OPENMP
    thread_count=omp_get_max_threads();
    if (thread_max!=0) thread_count=thread_max;
    omp_set_num_threads(thread_count);
    printf("%d: Threads[%d]",thread_count,mpi_rank);
    
    #pragma omp parallel default(shared) 
    {
      printf("%i ",omp_get_thread_num());
    }
    printf("\n");
   #else
    printf("OpenMP Disabled\n");
    thread_count=1;
    omp_set_num_threads(1);
  #endif
  
  
      
  #ifndef _USEMPI
    mpi_size = 1;  // Default for one-node run. 
    mpi_rank = 1;  // Default for one-node runs is node 0, N/S America. NB, with MPI running in debug model this is what MPI will set the values to anyway.
  #endif
  
  char* param_file;
  param_file = new char[in_path.length()+12];
  strcpy(param_file,&in_path[0]);
  strcat(param_file,"/params.bin");
  for (int i=0; i<2160; i++) {
    for (int j=0; j<1080; j++) {
      localPatchLookup[i][j]=-1;
      allPatchLookup[i][j]=-1;
    }
  }
  loadBinaryInitFile(this,param_file);
  buffer = new unsigned char*[thread_count];
  req_bytes_from = new unsigned int[mpi_size];
  rep_bytes_from = new unsigned int[mpi_size];
  est_bytes_from = new unsigned int[mpi_size];
  node_mpi_use = new char*[thread_count];
  req_base = new int*[thread_count];
  reply_base = new int*[thread_count];
  //printf("Before vector initialisation\n"); fflush(stdout);
  confirmQueue = new lwv::vector<infectedPerson*>*[thread_count];
  for (int j=0; j<thread_count; j++) confirmQueue[j] = new lwv::vector<infectedPerson*>[2];
  recoveryQueue = new lwv::vector<infectedPerson*>*[thread_count];
  for (int j=0; j<thread_count; j++) recoveryQueue[j]=new lwv::vector<infectedPerson*>[P->infectionWindow];
  contactQueue = new lwv::vector<infectedPerson*>*[thread_count];
  for (int j=0; j<thread_count; j++) contactQueue[j] = new lwv::vector<infectedPerson*>[P->infectionWindow];
  symptomQueue = new lwv::vector<infectedPerson*>*[thread_count];
  for (int j=0; j<thread_count; j++) symptomQueue[j] = new lwv::vector<infectedPerson*>[P->infectionWindow];
  
  for (int i=0; i<thread_count; i++) {
    buffer[i]=new unsigned char[8];
    node_mpi_use[i]=new char[mpi_size];
    for (int j=0; j<mpi_size; j++) node_mpi_use[i][j]=(char)0;
    req_base[i] = new int[mpi_size];
    reply_base[i] = new int[mpi_size];
    for (int j=0; j<mpi_size; j++) {
      reply_base[i][j]=0;
      req_base[i][j]=-1;
    }
  }

  delta_place_cases = new int**[no_units];
  delta_place_infs = new int**[no_units];
  delta_comm_cases = new int*[no_units];
  delta_comm_infs = new int*[no_units];
  delta_hh_cases = new int*[no_units];
  delta_hh_infs = new int*[no_units];
  delta_total_nonsympt_inf = new int*[no_units];
  delta_total_sympt_inf = new int*[no_units];

  for (int i=0; i<no_units; i++) {
    delta_place_cases[i]=new int*[thread_count];
    delta_place_infs[i]=new int*[thread_count];
    delta_comm_cases[i]=new int[thread_count];
    delta_comm_infs[i]=new int[thread_count];
    delta_hh_cases[i]=new int[thread_count];
    delta_hh_infs[i]=new int[thread_count];
    delta_total_nonsympt_inf[i] = new int[thread_count];
    delta_total_sympt_inf[i] = new int[thread_count];

    for (int j=0; j<thread_count; j++) {
      delta_comm_cases[i][j]=0;
      delta_comm_infs[i][j]=0;
      delta_hh_cases[i][j]=0;
      delta_hh_infs[i][j]=0;
      delta_total_nonsympt_inf[i][j]=0;
      delta_total_sympt_inf[i][j]=0;
      delta_place_cases[i][j]=new int[P->no_place_types];
      delta_place_infs[i][j]=new int[P->no_place_types];
      for (unsigned int k=0; k<P->no_place_types; k++) {
        delta_place_cases[i][j][k]=0;
        delta_place_infs[i][j][k]=0;
      }
    }
  }

  printf("After vector initialisation\n"); fflush(stdout);

  infected_grid = new int**[PNG_WIDTH];
  immune_grid = new int**[PNG_WIDTH];
  for (int i=0; i<PNG_WIDTH; i++) {
    infected_grid[i]=new int*[PNG_HEIGHT];
    immune_grid[i]=new int*[PNG_HEIGHT];
    for (int j=0; j<PNG_HEIGHT; j++) {
      infected_grid[i][j] = new int[thread_count];
      immune_grid[i][j] = new int[thread_count];
      for (int k=0; k<thread_count; k++) {
        infected_grid[i][j][k]=0;
        immune_grid[i][j][k]=0;
      }
    }
  }

  // Initialise remote request counters
  
  placeInfMsg = new lwv::vector<unsigned char>*[thread_count];
  for (int i=0; i<thread_count; i++) placeInfMsg[i] = new lwv::vector<unsigned char>[mpi_size];
  placeProphMsg = new lwv::vector<unsigned char>*[thread_count];
  for (int i=0; i<thread_count; i++) placeProphMsg[i] = new lwv::vector<unsigned char>[mpi_size];
  placeClosureMsg = new lwv::vector<unsigned char>*[thread_count];
  for (int i=0; i<thread_count; i++) placeClosureMsg[i] = new lwv::vector<unsigned char>[mpi_size];

  remoteRequests = new lwv::vector<unsigned char>*[thread_count];
  for (int i=0; i<thread_count; i++) remoteRequests[i] = new lwv::vector<unsigned char>[mpi_size];
  remoteReplies = new lwv::vector<unsigned char>*[thread_count];
  for (int i=0; i<thread_count; i++) remoteReplies[i] = new lwv::vector<unsigned char>[mpi_size];
  message_in = new unsigned char[1]; // Just a primer so that it can be deleted in the message handling section.

  reqHostAddresses = new lwv::vector<SIM_I64>*[thread_count];
  reqOrders = new lwv::vector<lwv::vector<unsigned short> >*[thread_count];
  reqContactAddresses = new lwv::vector<lwv::vector<SIM_I64> >*[thread_count];
  for (int i=0; i<thread_count; i++) {
    reqHostAddresses[i] = new lwv::vector<SIM_I64>[2];
    reqOrders[i] = new lwv::vector<lwv::vector<unsigned short> >[2];
    reqContactAddresses[i] = new lwv::vector<lwv::vector<SIM_I64> >[2];
  }

  
  // Initialise parameters

  T=0;
}

world::~world() {
#ifdef _USEDB
  db->DeleteSQLInsertStmt();
  delete db;
#endif
  delete [] allPatchList;
  delete [] localPatchList;
  
  for (int i=0; i<thread_count; i++) {
    for (int j=0; j<P->infectionWindow; j++) {
      while (contactQueue[i][j].size()>0) contactQueue[i][j].pop_back();
      contactQueue[i][j].clear();
      if (j<2) {
        while (confirmQueue[i][j].size()>0) confirmQueue[i][j].pop_back();
        confirmQueue[i][j].clear();
      }
      while (recoveryQueue[i][j].size()>0) recoveryQueue[i][j].pop_back();
      recoveryQueue[i][j].clear();
    }
    delete [] contactQueue[i];
    delete [] confirmQueue[i];
    delete [] recoveryQueue[i];
    delete [] symptomQueue[i];
    delete [] node_mpi_use[i];
    delete [] req_base[i];
    delete [] reply_base[i];
  }

  delete [] contactQueue;
  delete [] confirmQueue;
  delete [] recoveryQueue;
  delete [] symptomQueue;

  for (int i=0; i<PNG_WIDTH; i++) {
    for (int j=0; j<PNG_HEIGHT; j++) {
      delete [] infected_grid[i][j];
      delete [] immune_grid[i][j];
    }
    delete [] infected_grid[i];
    delete [] immune_grid[i];
  }
  delete [] infected_grid;
  delete [] immune_grid;

  for (int i=0; i<thread_count; i++) {
    for (int j=0; j<mpi_size; j++) {
      remoteRequests[i][j].clear();
      remoteReplies[i][j].clear();
      placeInfMsg[i][j].clear();
      placeClosureMsg[i][j].clear();
      placeProphMsg[i][j].clear();
    }
    delete[] remoteRequests[i];
    delete[] remoteReplies[i];
    delete[] placeInfMsg[i];
    delete[] placeClosureMsg[i];
    delete[] placeProphMsg[i];
  }


  for (int i=0; i<no_units; i++) {
    for (int j=0; j<thread_count; j++) {
      delete[] delta_place_cases[i][j];
      delete[] delta_place_infs[i][j];
    }
    delete[] delta_place_cases[i];
    delete[] delta_place_infs[i];
    delete[] delta_comm_cases[i];
    delete[] delta_comm_infs[i];
    delete[] delta_hh_cases[i];
    delete[] delta_hh_infs[i];
  }
  delete[] delta_place_cases;
  delete[] delta_place_infs;
  delete[] delta_comm_cases;
  delete[] delta_comm_infs;
  delete[] delta_hh_cases;
  delete[] delta_hh_infs;
  delete[] delta_total_sympt_inf;
  delete[] delta_total_nonsympt_inf;


  delete[] remoteRequests;
  delete[] remoteReplies;
  delete[] placeInfMsg;
  delete[] placeClosureMsg;
  delete[] placeProphMsg;
  for (unsigned int i=0; i<P->no_place_types; i++) delete no_places[i];
  delete[] no_places;

  for (int i=0; i<thread_count; i++) delete[] buffer[i];
  delete[] buffer;
  delete[] req_bytes_from;
  delete[] rep_bytes_from;
  delete[] node_mpi_use;
  delete[] req_base;
  delete[] reply_base;

  delete[] prob_travel;
  delete[] prob_visit;
  for (int i=0; i<no_countries; i++) {
    delete[] prob_dest[i];
    delete[] prob_orig[i];
    delete[] prob_dest_country[i];
    delete[] prob_orig_country[i];
  }
  delete[] destinations_count;
  delete[] origins_count;
  for (int i=0; i<no_countries; i++) {
    patches_in_country[i].clear();
    country_patch_pop[i].clear();
    delete[] people_per_country_per_node[i];
  }
  delete[] patches_in_country;
  delete[] country_patch_pop;
  delete[] people_per_country_per_node;
  
  delete[] interventions;
  for (int i=0; i<no_countries; i++) {
    for (unsigned int j=0; j<P->no_place_types; j++) {
      places[i][j].clear();
    }
    delete[] places[i];
  }

 delete[] places;

}

/* initialise.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Read initial data and setup initial matrices
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

#include "initialise.h"

void clearBuffer(world* w) {
  w->buffer_pointer=0;
  w->buffer_content=0;
}

void readBuffer(FILE* f, world* w) {
  w->buffer_content = fread(w->read_buffer,1,BUFFER_SIZE,f);
  w->buffer_pointer = 0;  
}

void read(FILE* f, int size, char* address, world* w) {
  int i;
  if (w->buffer_pointer+(SIM_I64)size<=w->buffer_content) {    // Common case - we'll be able to read the required amount without a buffer reload.
    for (i=0; i<size; i++) {
      address[i]=w->read_buffer[w->buffer_pointer++];
    }
  } else {
    i=0;      // This bit looks overkill, but for various reasons, I wanted to ensure
              // the code handles fread in readBuffer returning read 0 bytes. I suspect this might
              // rarely happen under very abnormally high disk/network IO stress.

    while (i<size) {                                                // While we want more bytes:
      int diff = (int) (w->buffer_content-w->buffer_pointer);       // What's left in the buffer?
      while ((diff>0) && (i<size)) {                                // While there are bytes left and we want them
        address[i++]=w->read_buffer[w->buffer_pointer++];           // Pick them from the buffer
        diff--;                                                     // One less byte left..
      }
      if (i<size) readBuffer(f,w);                                  // Buffer reload if we still want bytes (ie diff==0)
    }
  }
}

void loadOverlay(world *w, char *file) {
  errline=1136;
  FILE* f = fopen(&file[0],"rb");
  localPatch* p;
  int households,people;
  clearBuffer(w);
  for (int i=0; i<2160; i++) {
    for (int j=0; j<1080; j++) {
      read(f,4,(char*)&households,w);
      read(f,4,(char*)&people,w);
      if (w->localPatchLookup[i][j]>=0) {
        p=w->localPatchList[w->localPatchLookup[i][j]];
        p->no_people+=people;
        p->no_households+=households;
        p->rem_no_households+=households;
      }
    }
  }
  fclose(f);
  for (int i=0; i<2160; i++) {
    for (int j=0; j<1080; j++) {
      if (w->localPatchLookup[i][j]>=0) {
        p=w->localPatchList[w->localPatchLookup[i][j]];
        p->households = new household[w->localPatchList[w->localPatchLookup[i][j]]->no_households];
        p->people = new person[w->localPatchList[w->localPatchLookup[i][j]]->no_people];
        p->no_people=0;
        p->no_households=0;
      }
    }
  }
  errline=1165;
}

void loadHouseholdFile(world *w,  char* file, unsigned char country, int**** tpgn) {
  errline=1172;
  unsigned int total_households=0;
  unsigned int total_hosts=0;
  unsigned short hosts_in_household=0;
  unsigned int i=0,j=0;
  double lat=0;
  double lon=0;
  int admin_unit;
  int ls_x=0;
  int ls_y=0;
  float age=0;
  unsigned int place=0;
  unsigned short place_type=0;
  unsigned short group=0;
  int the_patch;
  int cache_patch=-1;
  int cache_cp_index=-1;
  unsigned char found_patch=0;
  unsigned short no_age_groups;
  float lower_bound;
  
 
  FILE* f = fopen(&file[0],"rb");
  clearBuffer(w);
  read(f,2,(char*)&no_age_groups,w);
  for (i=0; i<no_age_groups; i++) {
    read(f,4,(char*)&lower_bound,w);
  }
  read(f,4,(char*)&total_households,w);
  read(f,4,(char*)&total_hosts,w);

  // Calculate traveller/visitor probability, and normalise travel matrix for this country
  
  float total_travellers = 0;
  for (int i=0; i<w->destinations_count[country]; i++) total_travellers+=w->prob_dest[country][i];
  
  w->prob_travel[country]=(1.0f/365.0f) * (total_travellers/(float)total_hosts);
  for (int i=0; i<w->destinations_count[country]; i++) w->prob_dest[country][i]/=total_travellers;
  

  total_travellers=0;
  for (int i=0; i<w->origins_count[country]; i++) total_travellers+=w->prob_orig[country][i];
  w->prob_visit[country] = (1.0f/365.0f) * (total_travellers/(float)total_hosts);
  for (int i=0; i<w->origins_count[country]; i++) w->prob_orig[country][i]/=total_travellers;
  
  // Now convert w->prob_orig and w->prob_dest to cumulative.

  if (w->origins_count[country]>0) {
    for (int i=1; i<w->origins_count[country]; i++) {
      w->prob_orig[country][i]+=w->prob_orig[country][i-1];
      if (w->prob_orig[country][i]>1.0f) w->prob_orig[country][i]=1.0f;
    }
    w->prob_orig[country][w->origins_count[country]-1]=2.0f;    // Ensure final element in cumulative distribution is succeessful
  
  }

  if (w->destinations_count[country]>0) {
    for (int i=1; i<w->destinations_count[country]; i++) {
      w->prob_dest[country][i]+=w->prob_dest[country][i-1];
      if (w->prob_dest[country][i]>1.0f) w->prob_dest[country][i]=1.0f;
    }
      w->prob_dest[country][w->destinations_count[country]-1]=2.0f;    // Ensure final element in cumulative distribution is succeessful
  }

  int* unemp=new int[4];
  int* tot=new int[4];
  unemp[0]=0;
  unemp[1]=0;
  unemp[2]=0;
  unemp[3]=0;
  tot[0]=0;
  tot[1]=0;
  tot[2]=0;
  tot[3]=0;
  for (i=0; i<total_households; i++) {
    read(f,8,(char*)&lat,w);
    read(f,8,(char*)&lon,w);
    read(f,2,(char*)&hosts_in_household,w);
    read(f,4,(char*)&admin_unit,w);
    w->a_units[admin_unit].no_hosts+=hosts_in_household;
    
    // Locate patch for (lat,lon) - is it local?

    ls_x=lonToLsIndex(lon);
    ls_y=latToLsIndex(lat);
    int ls_xd20=ls_x/20;
    int ls_yd20=ls_y/20;
    the_patch=w->localPatchLookup[ls_xd20][ls_yd20];
    if (the_patch>=0) {
      w->people_per_country_per_node[country][w->mpi_rank]+=hosts_in_household;
      household *h = &w->localPatchList[the_patch]->households[w->localPatchList[the_patch]->no_households];
      h->lon=(float)lon;
      h->lat=(float)lat;
      h->unit=admin_unit;
      w->a_units[admin_unit].no_nodes=1;  // Mark unit as "in use" on this node. MPI reduce later will work out if it's used on other nodes too.
      h->no_people=(unsigned char) hosts_in_household;
      h->susc_people=h->no_people;
      h->first_person=w->localPatchList[the_patch]->no_people;
      h->country=country;
      h->patch=the_patch;

      for (j=0; j<hosts_in_household; j++) {
        person *p = &w->localPatchList[the_patch]->people[w->localPatchList[the_patch]->no_people];
        p->house=h;
        read(f,1,(char*)&p->age,w); // DUMMY - age index group
        read(f,4,(char*)&p->age,w); // Actual age (Float)
        int k=0;
        while ((k<w->no_age_bands-1) && (w->max_age_band[k]<p->age)) k++;
        p->susceptibility=(float)w->init_susceptibility[k];
        read(f,2,(char*)&place_type,w);
        p->place_type = (unsigned char) place_type;
        read(f,4,(char*)&p->place,w);
        read(f,2,(char*)&p->group,w);

        if (p->place_type<w->P->no_place_types) {
          tot[p->place_type]++;
          if (p->place<w->no_places[country][p->place_type]) {
            if (p->group==65535) p->group=w->places[country][p->place_type].at(p->place)->no_groups-1;
            tpgn[p->place_type][p->place][p->group][w->mpi_rank]++;
            
          } else 
            unemp[p->place_type]++;
        } else { printf("%d: ERROR - p->place_type=%d\n",w->mpi_rank, p->place_type); fflush(stdout); } // Should never happen!
        
        p->status=STATUS_SUSCEPTIBLE;
        w->localPatchList[the_patch]->no_people++;        
      }
      
      w->localPatchList[the_patch]->no_households++;
      if (w->localPatchList[the_patch]->no_households>w->localPatchList[the_patch]->rem_no_households) {
        printf("%d: ERROR! No. of households in file disagrees with overlay, rem_hh=%d, hh=%d, the_patch=%d, country=%d, ls_x=%d, ls_y=%d, lon=%e, lat=%e\n",w->mpi_rank,w->localPatchList[the_patch]->rem_no_households,
          w->localPatchList[the_patch]->no_households,the_patch,country,ls_x,ls_y,lon,lat);
        w->localPatchList[the_patch]->no_households=w->localPatchList[the_patch]->rem_no_households; // Just to keep it safe.
      }
      found_patch=0;
      if (cache_patch==the_patch) w->country_patch_pop[country][cache_cp_index]+=hosts_in_household;
      else {
        for (j=0; j<w->patches_in_country[country].size(); j++) {
          if (w->patches_in_country[country][j]==the_patch) {
            found_patch=1;
            w->country_patch_pop[country][j]+=hosts_in_household;
            cache_patch=the_patch;
            cache_cp_index=j;
            j=(int)w->patches_in_country[country].size();
          }
        }
        if (found_patch==0) {

          w->patches_in_country[country].push_back(the_patch);
          w->country_patch_pop[country].push_back(hosts_in_household);
          cache_patch=the_patch;
          cache_cp_index=(int) (w->patches_in_country[country].size()-1);
        }
      }

    } else {   // Household is not on node - need to parse, and possibly track establishments
               // Remember countries may be entirely on another node. Hence, fix this with MPI SYNC.

      for (j=0; j<hosts_in_household; j++) {
        read(f,1,(char*)&age,w);         // Dummy ageband index
        read(f,4,(char*)&age,w);         // Actual age float
        read(f,2,(char*)&place_type,w);  // Place type 0..3
        read(f,4,(char*)&place,w);       // Index of place
        read(f,2,(char*)&group,w);       // Group no. within palce
	    
        if (w->allPatchLookup[ls_xd20][ls_yd20]==-1) { // This can happen if population FLT file is out of sync with synthetic population
          printf("%d: MINUS ONE PATCH! ls_x=%d, ls_y=%d, ls_x/20=%d,ls_y/20=%d, lon=%E,lat=%E\n",w->mpi_rank,ls_x,ls_y,ls_xd20,ls_yd20,lon,lat); fflush(stdout);
        } else {
          if (place_type<w->P->no_place_types) {
            if (place<w->no_places[country][place_type]) {
              if (group==65535) group= w->places[country][place_type].at(place)->no_groups-1;
              tpgn[place_type][place][group][w->allPatchList[w->allPatchLookup[ls_xd20][ls_yd20]]->node]++;
            } 
          }
        }
      }
    }
    int test = w->allPatchLookup[ls_x/20][ls_y/20];
    if (test>=0) {
      w->patch_populations[test]+=hosts_in_household;
    } else { printf("Invalid APL, lon=%e, lat=%e, test=%d\n",lon,lat,test); fflush(stdout); }
  }
  fclose(f);
  errline=11275;
}

void linkPeopleToEstablishments(world* w, int country, int**** tpgn) {
  errline=11279;
  for (unsigned int i=0; i<w->P->no_place_types; i++) {
    for (unsigned int j=0; j<w->no_places[country][i]; j++) {
      place* p = w->places[country][i].at(j);
      p->group_member_count=new unsigned int[p->no_groups];
      for (unsigned int m=0; m<p->no_groups; m++) p->group_member_count[m]=0;

      unsigned int total=0;
      for (int m=0; m<w->mpi_size; m++) {
        for (unsigned int k=0; k<p->no_groups; k++) {
          total+=tpgn[i][j][k][m];
        }
      }
      if (total!=p->total_hosts) {
        printf("%d: Error - country %d, place type %d, place no %d, est says hosts=%d, hh says hosts=%d\n",
        w->mpi_rank,country,i,j,p->total_hosts,total);
        p->total_hosts=total;
      }

      
      p->no_nodes=0;
      for (int m=0; m<w->mpi_size; m++) {
        for (unsigned int k=0; k<p->no_groups; k++) {
          if (tpgn[i][j][k][m]>0) {
            p->no_nodes++;
            k=p->no_groups;
          }
        }
      }

      p->group_member_node_count = new unsigned int*[p->no_groups];
      if (p->no_nodes>1) p->no_nodes=w->mpi_size;
      for (unsigned int m=0; m<p->no_groups; m++) {
        p->group_member_node_count[m]=new unsigned int[p->no_nodes];
      }

      for (unsigned int k=0; k<p->no_groups; k++) {
        p->local_members[k]=new person*[tpgn[i][j][k][w->mpi_rank]];
        if (p->no_nodes==1) {
          p->group_member_node_count[k][0]=0;   // Use as counter
          p->group_member_count[k]+=tpgn[i][j][k][w->mpi_rank];
        } else {
          for (int m=0; m<w->mpi_size; m++) {
            p->group_member_count[k]+=tpgn[i][j][k][m];
            if (m==w->mpi_rank) p->group_member_node_count[k][m]=0;    // Use as counter
            else p->group_member_node_count[k][m]=tpgn[i][j][k][m];    // Use the value - nothing to count.
          }
        }
      }
    }
  }
  
  
  
  for (unsigned int i=0; i<w->noLocalPatches; i++) {
    localPatch* lp = w->localPatchList[i];
    for (int j=0; j<lp->no_households; j++) {
          
      for (unsigned char k=0; k<lp->households[j].no_people; k++) {
        person* p = &lp->people[lp->households[j].first_person+k];
        if (p->house->country==country) {
          if (p->place_type<w->P->no_place_types) {
            if (p->place<w->no_places[country][p->place_type]) {
              place* e = w->places[country][p->place_type].at(p->place);
              if (e->no_nodes==1) {
                e->local_members[p->group][e->group_member_node_count[p->group][0]++]=p;
     
              } else if (e->no_nodes>1) {
                
                e->local_members[p->group][e->group_member_node_count[p->group][w->mpi_rank]++]=p;

              }
            }
          } 
        }
      }
    }
  }
  errline=11357;
}


void loadPlaces(world *w,char* file,unsigned char country,unsigned char place_type) {

  // Load given establishment file of given establishment type, for given country.
  
  errline=11362;
  unsigned short dummy2;
  FILE* f = fopen(&file[0],"rb");
  read(f,2,(char*)&dummy2,w);  // ID of file. (should == place_type)
  read(f,4,(char*)&w->no_places[country][place_type],w);
  for (unsigned int i=0; i<w->no_places[country][place_type]; i++) {
    place* e = new place();
    e->country=country;
    read(f,8,(char*)&e->lat,w);
    read(f,8,(char*)&e->lon,w);
    read(f,4,(char*)&e->total_hosts,w);
    read(f,4,(char*)&e->no_groups,w);   // Actually, this is "largest group number" so... groups=0,1,2,3 will be "3" 
    e->no_groups+=2;                    // So no. groups = +1, then +1 for extra "65535" meaning staff/no group.
    read(f,4,(char*)&e->unit,w);
    errline=11381;
    e->local_members = new person**[e->no_groups];
    w->places[country][place_type].push_back(e);
    errline=11384;  
  }      
  fclose(f);
}

void loadTravelMatrix(world *w) {
  errline=11390;

  string t_file = w->in_path+"travel_matrix.bin";
  FILE *iniFile = fopen(&t_file[0],"rb");
  clearBuffer(w);
  read(iniFile,4,(char*)&w->no_countries,w);

  w->prob_visit = new float[w->no_countries+1];     // The +1 is to allow for "no country" - the -9999 in the initialisation file.
  w->prob_travel = new float[w->no_countries+1];
  w->prob_visit[w->no_countries]=-1;
  w->prob_travel[w->no_countries]=-1;

  w->origins_count = new int[w->no_countries];
  w->destinations_count = new int[w->no_countries];
  w->prob_dest_country = new unsigned char*[w->no_countries];
  w->prob_dest = new float*[w->no_countries];
  w->prob_orig_country = new unsigned char*[w->no_countries];
  w->prob_orig = new float*[w->no_countries];
  
  w->patches_in_country = new lwv::vector<int>[w->no_countries];
 

  w->country_patch_pop = new lwv::vector<int>[w->no_countries];
  int country_temp;

  for (int i=0; i<w->no_countries; i++) {
    read(iniFile,4,(char*)&w->destinations_count[i],w);
    w->prob_dest_country[i] = new unsigned char[w->destinations_count[i]];
    w->prob_dest[i] = new float[w->destinations_count[i]];
    for (int j=0; j<w->destinations_count[i]; j++) {
      read(iniFile,4,(char*)&country_temp,w);
      w->prob_dest_country[i][j]=(unsigned char) country_temp;
      read(iniFile,4,(char*)&country_temp,w);
      w->prob_dest[i][j] = (float) country_temp;
    }
  }
  for (int i=0; i<w->no_countries; i++) {
    read(iniFile,4,(char*)&w->origins_count[i],w);
    w->prob_orig_country[i] = new unsigned char[w->origins_count[i]];
    w->prob_orig[i] = new float[w->origins_count[i]];
    for (int j=0; j<w->origins_count[i]; j++) {
      read(iniFile,4,(char*)&country_temp,w);
      w->prob_orig_country[i][j]=(unsigned char) country_temp;
      read(iniFile,4,(char*)&country_temp,w);
      w->prob_orig[i][j]=(float) country_temp;
    }
  }

  w->people_per_country_per_node = new unsigned int*[w->no_countries];
  for (int i=0; i<w->no_countries; i++) {
    w->people_per_country_per_node[i] = new unsigned int[w->mpi_size];
    for (int j=0; j<w->mpi_size; j++) {
      w->people_per_country_per_node[i][j]=0;
    }
  }
  
 fclose(iniFile);
 errline=11447;
 
}


void loadPatches(world *w) {
  errline=11453;
  int x,y,size,node;
  int local=0;
  int p_i,p_j=0;
  int pi_d20=0;
  int pj_d20=0;
  
  
  string datafile="";
  datafile.append(w->in_path);
  datafile.append("config_");
  std::stringstream rankConverter;
  rankConverter << w->mpi_rank;
  datafile.append(rankConverter.str());
  datafile.append(".lsi");
  
  clearBuffer(w);
  FILE* iniFile = fopen(&datafile[0],"rb");
  read(iniFile,4,(char*)&w->noLocalPatches,w);
  read(iniFile,4,(char*)&w->noRemotePatches,w);
                              
  // Read patch data - comes in square bundles of landscan cells.
  w->totalPatches = w->noLocalPatches + w->noRemotePatches;
  w->allPatchList = new patch*[w->totalPatches];
  w->localPatchList = new localPatch*[w->noLocalPatches];
  w->patch_populations = new int[w->totalPatches];

  for (int i=0; i<(int)w->totalPatches; i++) {
    read(iniFile,4,(char*)&x,w);        // Landscan X co-ordinate of top-left corner
    read(iniFile,4,(char*)&y,w);        // Landscan Y co-ordinate for top-left corner
    y+=720;                             // Not sure about this. Landscan 0 = 84degrees Latitude
    read(iniFile,4,(char*)&size,w);     // Size in landscan cells
    read(iniFile,4,(char*)&node,w);     // Node that has "local" info for this cell

    w->patch_populations[i]=0;
    
    if (node==w->mpi_rank) {     // This patch is local for this node.
      if (local>=w->noLocalPatches) {
        printf("Error - node %d, more local patches in file than reported in header\n",w->mpi_rank);
        fflush(stdout);
        local=w->noLocalPatches-1;
      }
      localPatch *p = new localPatch();
      p->x=(unsigned short) x;
      p->y=(unsigned short) y;
      p->size=(unsigned short) size;
      p->node=(short) node;
      p->no_people=0;
      p->no_households=0;
      p->rem_no_households=0;
      p->no_qpatches=0;
      p->p_traveller=0;
      p->p_visitor=0;
      w->localPatchList[local]=p;
      w->allPatchList[i]=p;
      pi_d20=p->x/20;
      pj_d20=p->y/20;
      if ((pi_d20>=0) && (pi_d20<2160) && (pj_d20>=0) && (pj_d20<1080)) {
        w->localPatchLookup[pi_d20][pj_d20]=local;    // Store in a lookup table.
        w->allPatchLookup[pi_d20][pj_d20]=i;    // Store in a lookup table.
      } else {
        printf("Error - node %d, invalid x,y, %d,%d\n",w->mpi_rank,pi_d20,pj_d20);
        fflush(stdout);
      }
      local++;
    } else {
      patch *p = new patch();
      p->x=(unsigned short) x;
      p->y=(unsigned short) y;
      p->size=(unsigned short) size;
      p->node=(unsigned short) node;
      w->allPatchList[i]=p;
      for (p_i=p->x; p_i<p->x+size; p_i+=20) {
        for (p_j=p->y; p_j<p->y+size; p_j+=20) {
          pi_d20=p_i/20;
          pj_d20=p_j/20;
          if ((pi_d20>=0) && (pi_d20<2160) && (pj_d20>=0) && (pj_d20<1080))  {
            w->allPatchLookup[pi_d20][pj_d20]=i;
          } else {
            printf("Error - node %d, remote patch invalid x,y, %d,%d\n",w->mpi_rank,pi_d20,pj_d20);
            fflush(stdout);
          }
        }
      }
    }
  }
  read(iniFile,4,(char*)&x,w);    // This should be a -1
  fclose(iniFile);
  errline=11544;
}

void calculateQ(world* w) {
  errline=11548;
  int thread_no;

  // This loop parallelises embarrassingly, since qk1,k' is independent from qk2,k'

  #pragma omp parallel for private (thread_no) schedule(static,1)
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    double cumulative, q_temp,Z_k;
    patch *p_k;
    localPatch *p_klocal;
    patch* p_kprime;
    unsigned int k,k_prime;
  
    for (k=thread_no; k<w->noLocalPatches; k+=w->thread_count) {
      p_klocal=w->localPatchList[k];
      if (p_klocal->no_households>0) {
        p_k = static_cast<patch*>(p_klocal);
    
        // First calculate normalisation term = 1 / sum for all k', [ F(Dk,k') * Nk ]
      
        Z_k=0;
        q_temp=0;
      
        for (k_prime=0; k_prime<w->totalPatches; k_prime++) {
    	  p_kprime=w->allPatchList[k_prime];
          q_temp=(unit::kernel_F(&w->a_units[p_klocal->households[0].unit],patch::distance(p_k,p_kprime))*w->patch_populations[k_prime]);
          if (q_temp>0) {
            Z_k+=q_temp;
            p_klocal->no_qpatches++;
          }
        }
        p_klocal->q_prob = new float[p_klocal->no_qpatches];
        p_klocal->q_patch = new int[p_klocal->no_qpatches];
        p_klocal->no_qpatches=0;

        if (Z_k>0) {
          Z_k=1.0/Z_k;
        // Now generate cumulative distribution 
          cumulative=0;
          for (k_prime=0; k_prime<w->totalPatches; k_prime++) {
            p_kprime = w->allPatchList[k_prime];
            q_temp = (unit::kernel_F(&w->a_units[p_klocal->households[0].unit],patch::distance(p_k,p_kprime))*w->patch_populations[k_prime]*Z_k);
            if (q_temp>0) {
              cumulative+=q_temp;
              p_klocal->q_patch[p_klocal->no_qpatches]=k_prime;
              p_klocal->q_prob[p_klocal->no_qpatches++]=(float)cumulative;
            }
          }
        }
      }
    }
  }
  errline=11600;
}

void loadBinaryInitFile(world* w, string file) {
  errline=11604;
   w->read_buffer = new char[BUFFER_SIZE];
  loadTravelMatrix(w);
  FILE* f = fopen(&file[0],"rb");

  // Load basic settings
  int band_start;

  fread(&w->P->no_place_types,4,1,f);
  
  w->places = new lwv::vector<place*>*[w->no_countries];
  w->no_places = new unsigned int*[w->no_countries];
  for (int i=0; i<w->no_countries; i++) {
    w->places[i] = new lwv::vector<place*>[w->P->no_place_types];
    w->no_places[i] = new unsigned int[w->P->no_place_types];
  }


  int fixed_flag=0;
  w->P->symptom_delay=0;

  /*******************************************
  /* READ LATENT PERIOD
  /*******************************************/
  
  fread(&fixed_flag,4,1,f);                            // 1 = fixed latent period, 0 = variable
  w->P->latent_period_fixed=(fixed_flag==1);
  if (w->P->latent_period_fixed) {
    fread(&w->P->latent_period,8,1,f);                 // if fixed, just read one value
    w->P->latent_period_cutoff=w->P->latent_period;
  } else {
    fread(&w->P->latent_period_icdf_res,4,1,f);    // Else, read resolution of distrib (usually 21)
    w->P->latent_period_icdf = new double[w->P->latent_period_icdf_res];
    w->P->latent_period_icdf_res--;                
    fread(&w->P->latent_period_mean,8,1,f);
    for (int i=0; i<=w->P->latent_period_icdf_res; i++) {
      fread(&w->P->latent_period_icdf[i],8,1,f);  
      w->P->latent_period_icdf[i]=exp(-w->P->latent_period_icdf[i]);
    }
    fread(&w->P->latent_period_cutoff,8,1,f);                                // And cut off.    
  }

  /*******************************************
  /* READ INFECTIOUS-NESS OVER TIME
  /*******************************************/
  
  
  fread(&fixed_flag,4,1,f);                                // Fixed infectiousness = 1, 0 = variable
  w->P->infectiousness_fixed=(fixed_flag==1);
  if (w->P->infectiousness_fixed) {
    fread(&w->P->infectiousness,8,1,f);                    // Again, read one value if fixed
  } else {
    fread(&w->P->infectiousness_profile_res,4,1,f);               // Else resolution of distrib(usually 21)
    w->P->infectiousness_profile=new double[w->P->infectiousness_profile_res];
    for (int i=0; i<w->P->infectiousness_profile_res; i++) {
      fread(&w->P->infectiousness_profile[i],8,1,f);
    }
  }

  /*******************************************
  /* READ INFECTIOUS PERIOD
  /*******************************************/
    
  fread(&fixed_flag,4,1,f);
  w->P->infectious_period_fixed=(fixed_flag==1);
  if (w->P->infectious_period_fixed) {
    fread(&w->P->infectious_period,8,1,f);
    w->P->infectious_period_cutoff=w->P->infectious_period;
  } else {
    fread(&w->P->infectious_period_mean,8,1,f);
    fread(&w->P->infectious_period_icdf_res,4,1,f);
    w->P->infectious_period_icdf=new double[w->P->infectious_period_icdf_res];
    w->P->infectious_period_icdf_res--;
    for (int i=0; i<=w->P->infectious_period_icdf_res; i++) {
      fread(&w->P->infectious_period_icdf[i],8,1,f);
      w->P->infectious_period_icdf[i]=exp(-w->P->infectious_period_icdf[i]);
    }
    fread(&w->P->infectious_period_cutoff,8,1,f);
  }
  
  w->P->timesteps_per_day=4;
  w->P->timestep_hours=24.0/w->P->timesteps_per_day;
  
  w->P->infectionWindow=(int) (2+w->P->latent_period_cutoff+w->P->infectious_period_cutoff)*w->P->timesteps_per_day;
  

  // Load Interventions

  fread(&w->no_interventions,4,1,f);
  w->interventions = new intervention[w->no_interventions];

  for (int i=0; i<w->no_interventions; i++) {
    fread(&w->interventions[i].type,4,1,f);
    fread(&w->interventions[i].trig_on_type,4,1,f);
    if (w->interventions[i].trig_on_type==1) {
      fread(&w->interventions[i].start_day,8,1,f);
    } else if (w->interventions[i].trig_on_type==0) {
      fread(&w->interventions[i].on_case_period,4,1,f);
      fread(&w->interventions[i].on_case_op,4,1,f);
      fread(&w->interventions[i].on_threshold,8,1,f);
      fread(&w->interventions[i].on_thr_unit,4,1,f);
      fread(&w->interventions[i].on_thr_delay,8,1,f);

    } else printf("%d; ERROR - Trigger on type not recognised\n",w->mpi_rank);

    fread(&w->interventions[i].trig_off_type,4,1,f);

    if (w->interventions[i].trig_off_type==1) {
      fread(&w->interventions[i].duration_days,8,1,f);
    } else if (w->interventions[i].trig_off_type==0) {
      fread(&w->interventions[i].off_case_period,4,1,f);
      fread(&w->interventions[i].off_case_op,4,1,f);
      fread(&w->interventions[i].off_threshold,8,1,f);
      fread(&w->interventions[i].off_thr_unit,4,1,f);
      fread(&w->interventions[i].off_thr_delay,8,1,f);

    } else printf("%d: ERROR - Trigger off type not recognised\n",w->mpi_rank);

    if (w->interventions[i].type==BORDER_CONTROL_ID) {
      w->interventions[i].sub_type = new BorderControlInt();
      fread(&((BorderControlInt*)w->interventions[i].sub_type)->p_deny_exit,8,1,f);
      fread(&((BorderControlInt*)w->interventions[i].sub_type)->p_deny_entry,8,1,f);

    } else if (w->interventions[i].type==TREATMENT_ID) {
      w->interventions[i].sub_type = new TreatmentInt();
      fread(&((TreatmentInt*)w->interventions[i].sub_type)->m_inf_of_clinical,8,1,f);
      fread(&((TreatmentInt*)w->interventions[i].sub_type)->treat_delay,8,1,f);
      fread(&((TreatmentInt*)w->interventions[i].sub_type)->treat_duration,8,1,f);

    } else if (w->interventions[i].type==PROPHYLAXIS_ID) {
      w->interventions[i].sub_type = new ProphylaxisInt();
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->m_susc_of_susceptible,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->m_inf_of_proph,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->m_clin_of_proph,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->proph_delay,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->proph_duration,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->proph_coverage,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->proph_household,8,1,f);
      fread(&((ProphylaxisInt*)w->interventions[i].sub_type)->proph_social,8,1,f);
      
    } else if (w->interventions[i].type==VACC_ID) {
      w->interventions[i].sub_type = new VaccinationInt();
      fread(&((VaccinationInt*)w->interventions[i].sub_type)->m_vacc_susc,8,1,f);
      fread(&((VaccinationInt*)w->interventions[i].sub_type)->m_vacc_inf,8,1,f);
      fread(&((VaccinationInt*)w->interventions[i].sub_type)->m_vacc_clin,8,1,f);
      fread(&((VaccinationInt*)w->interventions[i].sub_type)->vacc_delay,8,1,f);
      fread(&((VaccinationInt*)w->interventions[i].sub_type)->vacc_coverage,8,1,f);

    } else if (w->interventions[i].type==QUARANTINE_ID) {
      w->interventions[i].sub_type = new QuarantineInt();
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->m_s_wp_rate,8,1,f);
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->m_hh_rate,8,1,f);
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->q_period,8,1,f);
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->q_delay,8,1,f);
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->q_compliance,8,1,f);
      fread(&((QuarantineInt*)w->interventions[i].sub_type)->q_community,8,1,f);

    } else if (w->interventions[i].type==PLACE_CLOSE_ID) {
      w->interventions[i].sub_type = new PlaceClosureInt();
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->threshold,8,1,f);
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->thr_unit,4,1,f);
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->delay,8,1,f);
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->period,8,1,f);
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->m_hh_rate,8,1,f);
      fread(&((PlaceClosureInt*)w->interventions[i].sub_type)->m_comm_rate,8,1,f);

    } else if (w->interventions[i].type==BLANKET_ID) {
      w->interventions[i].sub_type = new BlanketTravelInt();
      fread(&((BlanketTravelInt*)w->interventions[i].sub_type)->distance,8,1,f);
      fread(&((BlanketTravelInt*)w->interventions[i].sub_type)->p_deny_travel,8,1,f);

    } else if (w->interventions[i].type==AREA_QUARANTINE_ID) {
      w->interventions[i].sub_type = new AreaQuarantineInt();
      fread(&((AreaQuarantineInt*)w->interventions[i].sub_type)->ring_radius,8,1,f);
      fread(&((AreaQuarantineInt*)w->interventions[i].sub_type)->p_deny_travel,8,1,f);
      fread(&((AreaQuarantineInt*)w->interventions[i].sub_type)->period,8,1,f);
      fread(&((AreaQuarantineInt*)w->interventions[i].sub_type)->delay,8,1,f);

    } else printf("%d: ERROR - Unknown intervention type\n",w->mpi_rank);

  }

  fread(&w->no_units,4,1,f);
  
  w->a_units = new unit[w->no_units];
  for (int i=0; i<w->no_units; i++) {
    w->a_units[i].log=false;
    w->a_units[i].no_nodes=0;
    w->a_units[i].contact_makers=new int[w->thread_count];
    w->a_units[i].B_place = new double[w->P->no_place_types];
    w->a_units[i].P_group = new double[w->P->no_place_types];
    
    w->a_units[i].new_place_cases = new int[w->P->no_place_types];
    w->a_units[i].new_place_infs = new int[w->P->no_place_types];
    w->a_units[i].hist_place_cases = new int*[w->P->no_place_types];
    w->a_units[i].place_10day_accumulator = new int[w->P->no_place_types];
    for (unsigned int j=0; j<w->P->no_place_types; j++) {
      w->a_units[i].new_place_cases[j]=0;
      w->a_units[i].new_place_infs[j]=0;
      w->a_units[i].hist_place_cases[j] = new int[10*w->P->timesteps_per_day];
      for (int k=0; k<10*w->P->timesteps_per_day; k++) {
        w->a_units[i].hist_place_cases[j][k]=0;
      }
      w->a_units[i].place_10day_accumulator[j]=0;
    }
    
    w->a_units[i].new_hh_cases=0;
    w->a_units[i].new_comm_cases=0;
    w->a_units[i].new_hh_infs=0;
    w->a_units[i].new_comm_infs=0;
    w->a_units[i].current_nonsymptomatic_inf=0;
    w->a_units[i].current_symptomatic_inf=0;
    
    w->a_units[i].hist_comm_cases=new int[10*w->P->timesteps_per_day];
    w->a_units[i].hist_hh_cases=new int[10*w->P->timesteps_per_day];
    for (int j=0; j<10*w->P->timesteps_per_day; j++) {
      w->a_units[i].hist_comm_cases[j]=0;
      w->a_units[i].hist_hh_cases[j]=0;
    }
    w->a_units[i].comm_10day_accumulator=0;
    w->a_units[i].hh_10day_accumulator=0;
    w->a_units[i].no_hosts=0;

    w->a_units[i].p_symptomatic=0;
    w->a_units[i].p_severe=0;
    w->a_units[i].p_detect_sympt=0;
    w->a_units[i].p_detect_severe=0;
    w->a_units[i].abs_place_sympt=new double[w->P->no_place_types];
    w->a_units[i].abs_place_sev=new double[w->P->no_place_types];
    w->a_units[i].abs_place_sympt_cc_mul=new double[w->P->no_place_types];
    w->a_units[i].abs_place_sev_cc_mul=new double[w->P->no_place_types];
    w->a_units[i].mul_sympt_inf=0;
    w->a_units[i].mul_severe_inf=0;


    int level,grump_index;
    fread(&level,4,1,f);
    w->a_units[i].level=(unsigned char)level;
    fread(&grump_index,4,1,f);
    w->a_units[i].country=(unsigned char) grump_index;
    if (level<=0) w->a_units[i].parent_id=level; // So parent of level -1 unit = -1 (no parent - this is the global unit)
                                                 // parent of level 0 = index 0 (this is country - parent is global)
    else fread(&w->a_units[i].parent_id,4,1,f);  // otherwise read index of parent

    // Unit parameters

    fread(&w->a_units[i].B_spat,8,1,f);
    fread(&w->a_units[i].k_a,8,1,f);
    fread(&w->a_units[i].k_b,8,1,f);
    fread(&w->a_units[i].k_cut,8,1,f);
    fread(&w->a_units[i].B_hh,8,1,f);

    
    for (unsigned int j=0; j<w->P->no_place_types; j++) {
      fread(&w->a_units[i].B_place[j],8,1,f);
      fread(&w->a_units[i].P_group[j],8,1,f);
      fread(&w->a_units[i].abs_place_sympt[j],8,1,f);
      fread(&w->a_units[i].abs_place_sympt_cc_mul[j],8,1,f);
      fread(&w->a_units[i].abs_place_sev[j],8,1,f);
      fread(&w->a_units[i].abs_place_sev_cc_mul[j],8,1,f);
    }

    fread(&w->a_units[i].p_symptomatic,8,1,f);
    fread(&w->a_units[i].p_detect_sympt,8,1,f);
    fread(&w->a_units[i].mul_sympt_inf,8,1,f);
    fread(&w->a_units[i].p_severe,8,1,f);
    fread(&w->a_units[i].p_detect_severe,8,1,f);
    fread(&w->a_units[i].mul_severe_inf,8,1,f);
    fread(&w->a_units[i].seasonal_max,8,1,f);
    fread(&w->a_units[i].seasonal_min,8,1,f);
    fread(&w->a_units[i].seasonal_temporal_offset,8,1,f);

    // Interventions selected for this unit

    fread(&w->a_units[i].no_interventions,4,1,f);
    w->a_units[i].interventions=new LiveIntervention[w->a_units[i].no_interventions];
    for (int j=0; j<w->a_units[i].no_interventions; j++) {
      fread(&w->a_units[i].interventions[j].int_no,4,1,f);
      w->a_units[i].interventions[j].switch_time=-1;
      w->a_units[i].interventions[j].unit=i;
      w->a_units[i].interventions[j].active=false;
    }

    int log;
    fread(&log,4,1,f);
    w->a_units[i].log=(log==1);

  }
  errline=11893;
  resetUnitStats(w);

  // Seeding initialisation

  fread(&w->P->seed1,4,1,f);
  fread(&w->P->seed2,4,1,f);
  initSeeds(w->P->seed1,w->P->seed2);
  fread(&w->P->no_seeds,4,1,f);
  
  w->P->seed_lat = new double[w->P->no_seeds];
  w->P->seed_long = new double[w->P->no_seeds];
  w->P->seed_no = new int[w->P->no_seeds];
  w->P->seed_ts = new int[w->P->no_seeds];
  w->P->next_seed = 0;

  for (int i=0; i<w->P->no_seeds; i++) {
    fread(&w->P->seed_long[i],8,1,f);
    fread(&w->P->seed_lat[i],8,1,f);
    double day;
    fread(&day,8,1,f);
    w->P->seed_ts[i]=(int) (day*(double)24.0);
    fread(&w->P->seed_no[i],4,1,f);
  }

  // Params for Initialising population
  fread(&w->no_age_bands,4,1,f);
  w->max_age_band=new float[w->no_age_bands];
  w->init_susceptibility = new double[w->no_age_bands];
  for (int i=0; i<w->no_age_bands; i++) {
    fread(&band_start,4,1,f);
    fread(&w->max_age_band[i],4,1,f);
    fread(&w->init_susceptibility[i],8,1,f);
  }
  
  
  // Output options

  int dummy;
  fread(&dummy,4,1,f);
  if (dummy==1) {
    w->log_db=true;
    fread(&dummy,4,1,f);
    w->db_server=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->db_server[i],1,1,f);
    w->db_server[dummy]='\0';
    fread(&dummy,4,1,f);
    w->db_table=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->db_table[i],1,1,f);
    w->db_table[dummy]='\0';
    initDB(w);
  } else w->log_db=false;

  fread(&dummy,4,1,f);
  if (dummy==1) {
    w->log_flat=true;
    fread(&dummy,4,1,f);
    w->ff_path=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->ff_path[i],1,1,f);
    w->ff_path[dummy]='\0';
    fread(&dummy,4,1,f);
    w->ff_file=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->ff_file[i],1,1,f);
    w->ff_file[dummy]='\0';

    char* ff_file = new char[(strlen(w->ff_path)+strlen(w->ff_file))+10];
    strcpy(ff_file,w->ff_path);
    strcat(ff_file,"/");
    strcat(ff_file,w->ff_file);
    strcat(ff_file,".txt");
    if (w->mpi_rank==0) w->ff=fopen(&ff_file[0],"w");
  } else w->log_flat=false;

  fread(&dummy,4,1,f);
  if (dummy==1) {
    w->log_movie=true;
    fread(&dummy,4,1,f);
    w->mv_path=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->mv_path[i],1,1,f);
    w->mv_path[dummy]='\0';
    fread(&dummy,4,1,f);
    w->mv_file=new char[dummy+1];
    for (int i=0; i<dummy; i++) fread(&w->mv_file[i],1,1,f);
    w->mv_file[dummy]='\0';
  } else w->log_movie=false;
  
  printf("%d:  Loading patches\n",w->mpi_rank);
  fflush(stdout);
  loadPatches(w);
  

  errline=11985;
  int noCountryFiles;
  fread(&noCountryFiles,4,1,f);

  char* ov_file;
  char* hh_file;
  char** place_files;
  
  fread(&dummy,4,1,f);
  ov_file = new char[dummy+1];
  for (int j=0; j<dummy; j++) fread(&ov_file[j],1,1,f);
  ov_file[dummy]='\0';
  
  printf("%d:  Loading %s\n",w->mpi_rank,ov_file);
  fflush(stdout);
  loadOverlay(w,ov_file);
    
  for (int i=0; i<noCountryFiles; i++) {
    errline=111005;

    int grump;
    fread(&grump,4,1,f);
    int code;
    fread(&code,4,1,f);
    
    int no_nodes,node;
    fread(&no_nodes,4,1,f);
    
    bool needed=false;
    for (char j=0; j<no_nodes; j++) {
      fread(&node,4,1,f);
      if (node==w->mpi_rank) needed=true;
    }
    fread(&dummy,4,1,f);
    hh_file= new char[dummy+1];
    for (int j=0; j<dummy; j++) fread(&hh_file[j],1,1,f);
    hh_file[dummy]='\0';
 
   place_files = new char*[w->P->no_place_types];
   for (unsigned int k=0; k<w->P->no_place_types; k++) {
     fread(&dummy,4,1,f);
     place_files[k] = new char[dummy+1];
     for (int j=0; j<dummy; j++) fread(&place_files[k][j],1,1,f);
     place_files[k][dummy]='\0';
   }
  
    if (needed) {
      printf("%d:  Loading %s\n",w->mpi_rank,hh_file);
      fflush(stdout);
      clearBuffer(w);
      for (unsigned char p=0; p<w->P->no_place_types; p++) {
        clearBuffer(w);
        loadPlaces(w,place_files[p],code,p);
      }

      // This structure is so that when reading the households, we count 
      // who attends which place_type/place no./group and which node they live on.
      // This is then used for linking up people to places in static arrays
      // rather than vectors.

      int**** tpgn;   // type, place, group, node
      tpgn = new int***[w->P->no_place_types];
      for (unsigned int _i=0; _i<w->P->no_place_types; _i++) {
        tpgn[_i]= new int**[w->no_places[code][_i]];
        for (unsigned int _j=0; _j<w->no_places[code][_i]; _j++) {
          place* p = w->places[code][_i].at(_j);
          tpgn[_i][_j] = new int*[p->no_groups];
          for (unsigned int _k=0; _k<p->no_groups; _k++) {   // Extra group at the end for "-1" = no group
            tpgn[_i][_j][_k]=new int[w->mpi_size];
            for (int _m=0; _m<w->mpi_size; _m++) {
              tpgn[_i][_j][_k][_m]=0;
            }
          }
        }
      }
      errline=111065;

      loadHouseholdFile(w,hh_file,(unsigned char) code,tpgn);
      errline=111072;
      linkPeopleToEstablishments(w,(unsigned char) code,tpgn);
      
      for (unsigned int _i=0; _i<w->P->no_place_types; _i++) {
        for (unsigned int _j=0; _j<w->no_places[code][_i]; _j++) {
          place* p = w->places[code][_i].at(_j);
          for (unsigned int _k=0; _k<p->no_groups; _k++) delete[] tpgn[_i][_j][_k];
          delete[] tpgn[_i][_j];
        }
        delete[] tpgn[_i];
      }
      delete[] tpgn;
      
    } 
  }
  errline=111089;
  fclose(f);
  printf("%d:  Calculating q matrix\n",w->mpi_rank);
  fflush(stdout);
  calculateQ(w);
  
  delete w->read_buffer;

  syncAdminUnitUse(w);    // Agree on which admin units are relevant to multiple nodes.
  syncPPCPN(w);           // Agree on how many people per country on each node.
  errline=111001;
  
}

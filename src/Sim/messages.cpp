/* messages.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Functions for building and sending MPI messages
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

#include "messages.h"

unsigned char* image_message;    // This is a buffer for assembling the data for movie PNG files.

int* starter_msg_out;    // The first message sorts out all the sizes... it is annoying that MPI needs this separately.
int* starter_msg_in;     // MSG_size_out is an array of 3* (mpi_size*mpi_size) - node-to-node grids for (1) requests, (2) replies
int* msg_counts_out;  // and (3) school/place messages. MSG_size_in is the same for the incoming "result".
int* msg_displs_out;  // The All-to-all messages work by storing, for each node, the list of sizes (counts) for each destination node
int* msg_counts_in;   // And the displacements of each in one big buffer. These int*s do this for the outgoing and incoming messages.
int* msg_displs_in;

int starter_msg_size=0;

void initialiseMessages(world* w) {
#ifdef _USEMPI
  if (w->mpi_rank==0) {
    image_message = new unsigned char[PNG_WIDTH*PNG_HEIGHT];   // Image size specified here. Use node 0 to compile the image.
  } else {
    image_message = new unsigned char[1];
  }

  starter_msg_size = (w->mpi_size*w->mpi_size*3);                 // BLOCKS 1,2,3
  starter_msg_size+= (w->no_units*(6+(2*w->P->no_place_types)));  // BLOCK 4 
  starter_msg_size++;                                             // BLOCK 5        -  see comments in doMessage()

  starter_msg_in = new int[starter_msg_size];
  starter_msg_out = new int[starter_msg_size];
  msg_counts_out = new int[w->mpi_size];
  msg_displs_out = new int[w->mpi_size];
  msg_counts_in = new int[w->mpi_size];
  msg_displs_in = new int[w->mpi_size];


#else
  image_message = new unsigned char[1];
#endif
}


void addFirstRemoteRequest(world* w, unsigned short thread_no,float lon, float lat, infectedPerson* infected,unsigned short node, unsigned short n_remotes) {
  // Only called from addRemoteRequest
  //
  // Add new request fragment stub. If an individual requests one remote contact, it is likely to actually request more than one. Reduce
  // message size therefore by only including the details of the infector (lon,lat,address,total contacts needed, etc) once, below.
  // Since subsequent requests for that infector will be contiguous in the [thread_no][node] array, this is thread-safe.


  SIM_I64 address;
  unsigned short n = (unsigned short) infected->n_contacts;
  w->req_base[thread_no][node]=(int) w->remoteRequests[thread_no][node].size();            // Store base for this message.
  for (int i=0; i<4; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(lon)))[i]);           // Lon of infected person (float)
  for (int i=0; i<4; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(lat)))[i]);           // Lat of infected person (float)
  address = (SIM_I64) infected;
  for (int i=0; i<8; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&address))[i]);         // Mem address of infected person (SIM_I64)
  for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&n))[i]);               // Total contact needed (n) (ushort)
  for (unsigned short i=0; i<n; i++)                                                                                                  // Provide space for ordering local contacts.
    for (int j=0; j<2; j++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&i))[j]);             //   (Default is 0,1,2,3,4,5... overwrite later)
  for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&n_remotes))[i]);       // Number of remote requests - overwritten. (req_base+18+2*n_contacts)
}

void addRemoteRequest(world* w, unsigned short thread_no,patch* location_susceptible,float lon, float lat, infectedPerson* infected,float new_contact_time,unsigned short contact_no,unsigned short node) {
#ifdef _USEMPI
  if (w->req_base[thread_no][node]==-1) {                                         // Is this the first request added?
    addFirstRemoteRequest(w, thread_no,lon,lat,infected,node,(unsigned short)1);        // If so, add the infector details, before adding the target info
    unsigned short lsize = (unsigned short) location_susceptible->size/20;              // Size of location patch. (Reduced to short)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(contact_no)))[i]);               // The "index" of requests in the contacts order.
    for (int i=0; i<4; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&new_contact_time))[i]);           // Time of contact (float)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(location_susceptible->x)))[i]);  // X of local patch (ushort)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(location_susceptible->y)))[i]);  // Y of local patch (ushort)
    w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(lsize)))[0]);                                            // Size of local patch (uchar)
    w->node_mpi_use[thread_no][node]=(char)1;
  
  } else {
    int req_base = w->req_base[thread_no][node]+18+(2*infected->n_contacts);                               // Otherwise, the base will have been set
    unsigned short no_requests = *((unsigned short *) &(w->remoteRequests[thread_no][node][req_base]));    // Get no. of requests as unsigned short
    
    no_requests++;              
    for (int k=0; k<2; k++) w->remoteRequests[thread_no][node][req_base+k]=((unsigned char*)(&(no_requests)))[k];   // Update no. of requests in message

    unsigned short lx = (unsigned short) location_susceptible->x;
    unsigned short ly = (unsigned short) location_susceptible->y;
    unsigned char  lsize = location_susceptible->size/20;

    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(contact_no)))[i]);    // The "index" of requests in the contacts order.
    for (int i=0; i<4; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&new_contact_time))[i]); // Time of contact (float)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(lx)))[i]);             // X of local patch (ushort)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(ly)))[i]);             // Y of local patch (ushort)
    w->remoteRequests[thread_no][node].push_back(((unsigned char*)(&(lsize)))[0]);                                  // Size of local patch (uchar)
  }
#endif
}

void addTravelRequest(world* w, unsigned short thread_no,infectedPerson* infected,float new_contact_time,unsigned short contact_no, unsigned char first_flag, unsigned short travel_type) {
#ifdef _USEMPI
  if (w->req_base[thread_no][infected->travel_plan->travel_node]==-1) {                                                                   // Is this the first request added?
    addFirstRemoteRequest(w, thread_no,infected->personPointer->house->lon, infected->personPointer->house->lat, infected,infected->travel_plan->travel_node,(unsigned short)1);  // If so, call special initialiser
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(contact_no)))[i]);    // The "index" of requests in the contacts order.
    for (int i=0; i<4; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&new_contact_time))[i]); // Time of contact (float)
    unsigned short s = (unsigned short) infected->travel_plan->country;
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(s)))[i]);             // X of local patch (ushort)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(travel_type)))[i]);          // TRAVEL MODE
    w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(first_flag);                                  // 0 = first travel request - choose location
    w->node_mpi_use[thread_no][infected->travel_plan->travel_node]=(char)1;
  
  } else {
    int req_base = w->req_base[thread_no][infected->travel_plan->travel_node]+18+(2*infected->n_contacts);                               // Otherwise, the base will have been set
    unsigned short no_requests = *((unsigned short *) &(w->remoteRequests[thread_no][infected->travel_plan->travel_node][req_base]));    // Get no. of requests as unsigned short
    no_requests++;              
    for (int k=0; k<2; k++) w->remoteRequests[thread_no][infected->travel_plan->travel_node][req_base+k]=((unsigned char*)(&(no_requests)))[k];   // Update no. of requests in message
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(contact_no)))[i]);    // The "index" of requests in the contacts order.
    for (int i=0; i<4; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&new_contact_time))[i]); // Time of contact (float)
    unsigned short s = (unsigned short) infected->travel_plan->country;
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(s)))[i]);             // Country to pick (ushort)
    for (int i=0; i<2; i++) w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(((unsigned char*)(&(travel_type)))[i]);   // TRAVEL MODE
    w->remoteRequests[thread_no][infected->travel_plan->travel_node].push_back(first_flag);                                                      // 1 = NOT the first travel req. Keep position.
  }
#endif
}

void finaliseRemoteRequest(world* w, unsigned short thread_no, infectedPerson* infected) { // An individual has made all their remote requests - tidy the message up.
#ifdef _USEMPI
  unsigned short count_nodes=0;
  for (unsigned short i=0; i<w->mpi_size; i++) {
    if (w->node_mpi_use[thread_no][i]==(char)1) count_nodes++;    // Count number of remote nodes that took part in the contact finding algorithm
  }
  
  // So, this infected host has "open" messages reader for a number of nodes.

  for (int i=0; i<w->mpi_size; i++) {
    if (w->node_mpi_use[thread_no][i]==(char)1) {                                                                        // If there is an "open" message for this node,
      for (int j=0; j<2; j++) w->remoteRequests[thread_no][i].push_back(((unsigned char*)(&(count_nodes)))[j]);          // Then add how many nodes are in use for this message,
      for (int j=0; j<infected->n_contacts; j++) {                                                                       // And tell all of them the list of local contact nos.
        for (int k=0; k<2; k++) 
          w->remoteRequests[thread_no][i][w->req_base[thread_no][i]+18+(2*j)+k] = ((unsigned char*)(&(infected->contact_order[j])))[k];    // Overwrite the default values for local contacts
      }
    }
  }

  for (unsigned short i=0; i<w->mpi_size; i++) {
    if (w->node_mpi_use[thread_no][i]==(char)1) {
      for (unsigned short j=0; j<w->mpi_size; j++) {
        if (w->node_mpi_use[thread_no][j]==(char)1) {
          for (unsigned short k=0; k<2; k++) {
            w->remoteRequests[thread_no][i].push_back(((unsigned char*)(&(j)))[k]);             // Then tell them the list of nodes that are involved in this transaction.
          }
        }
      }
    }
  }
  for (int i=0; i<w->mpi_size; i++) {
    w->node_mpi_use[thread_no][i]=(char)0;
    w->req_base[thread_no][i]=-1;                           // And reset the counters and flags.
  }
#endif
}

void addRemoteReply(world* w, unsigned short thread_no, infectedPerson* pointer, unsigned short home_node, unsigned short dest_node, unsigned short contact_no) {
#ifdef _USEMPI
  if (w->reply_base[thread_no][dest_node]==0) addFirstRemoteReply(w,thread_no,pointer,home_node,dest_node,contact_no);
  else {
    unsigned short successes = *((unsigned short *) &(w->remoteReplies[thread_no][dest_node])[w->reply_base[thread_no][dest_node]]);
    successes++;
    for (int i=0; i<2; i++) w->remoteReplies[thread_no][dest_node][w->reply_base[thread_no][dest_node]+i]=((unsigned char*)(&successes))[i];
    for (int i=0; i<2; i++) w->remoteReplies[thread_no][dest_node].push_back( ((unsigned char*)(&contact_no))[i]);
  }
#endif
}

void addFirstRemoteReply(world* w, unsigned short thread_no, infectedPerson* pointer, unsigned short home_node, unsigned short dest_node, unsigned short contact_no) {
#ifdef _USEMPI
  SIM_I64 address = (SIM_I64) pointer;
  w->remoteReplies[thread_no][dest_node].push_back((unsigned char)0);                                            // Control byte. See request handling section.
  for (int i=0; i<8; i++) w->remoteReplies[thread_no][dest_node].push_back(((unsigned char*)(&address))[i]);     // Mem address of infected person
  for (int i=0; i<2; i++) w->remoteReplies[thread_no][dest_node].push_back(((unsigned char*)(&home_node))[i]);   // Which node is this person local on
  unsigned short one = 1;
  w->reply_base[thread_no][dest_node]=(int) w->remoteReplies[thread_no][dest_node].size();                       // Remember base address
  for (int i=0; i<2; i++) w->remoteReplies[thread_no][dest_node].push_back(((unsigned char*)(&one))[i]);         // So far, count of replies for this infected = 1
  for (int i=0; i<2; i++) w->remoteReplies[thread_no][dest_node].push_back(((unsigned char*)(&contact_no))[i]);  // And the index of this contact.
#endif
}

void finaliseRemoteReply(world* w, unsigned short thread_no) {
#ifdef _USEMPI
  for (int i=0; i<w->mpi_size; i++) w->reply_base[thread_no][i]=0;
#endif
}

void addPlaceInfectionMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country, unsigned char place_type,
    int place, int host_no,double new_contact_time, double infectiousness, double t_inf) {
  w->placeInfMsg[thread_no][node].push_back(country);
  w->placeInfMsg[thread_no][node].push_back(place_type);
  for (int i=0; i<4; i++) w->placeInfMsg[thread_no][node].push_back(((unsigned char*)(&place))[i]);
  for (int i=0; i<4; i++) w->placeInfMsg[thread_no][node].push_back(((unsigned char*)(&host_no))[i]);
  for (int i=0; i<8; i++) w->placeInfMsg[thread_no][node].push_back(((unsigned char*)(&new_contact_time))[i]);
  for (int i=0; i<8; i++) w->placeInfMsg[thread_no][node].push_back(((unsigned char*)(&infectiousness))[i]);
  for (int i=0; i<8; i++) w->placeInfMsg[thread_no][node].push_back(((unsigned char*)(&t_inf))[i]);
}

void addPlaceClosureMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country,
    unsigned char place_type, int place,float start, float end) {
  w->placeClosureMsg[thread_no][node].push_back(country);
  w->placeClosureMsg[thread_no][node].push_back(place_type);
  for (int i=0; i<4; i++) w->placeClosureMsg[thread_no][node].push_back(((unsigned char*)(&place))[i]);
  for (int i=0; i<4; i++) w->placeClosureMsg[thread_no][node].push_back(((unsigned char*)(&start))[i]);
  for (int i=0; i<4; i++) w->placeClosureMsg[thread_no][node].push_back(((unsigned char*)(&end))[i]);
}

void addPlaceProphylaxMsg(world* w, unsigned short thread_no,unsigned short node, unsigned char country,
    unsigned char place_type, int place,float start, float end) {
  w->placeProphMsg[thread_no][node].push_back(country);
  w->placeProphMsg[thread_no][node].push_back(place_type);
  for (int i=0; i<4; i++) w->placeProphMsg[thread_no][node].push_back(((unsigned char*)(&place))[i]);
  for (int i=0; i<4; i++) w->placeProphMsg[thread_no][node].push_back(((unsigned char*)(&start))[i]);
  for (int i=0; i<4; i++) w->placeProphMsg[thread_no][node].push_back(((unsigned char*)(&end))[i]);
}

void syncAdminUnitUse(world* w) {
  // Called once at the beginning to agree on which nodes have "some" responsibility for each admin unit.
  // (Hence, if events happen in admin unit on one node, the others need to be synchronised
  // Before this function, unit.no_nodes is either 1 or 0, for "present" or "not"
  // After this function, unit.no_nodes is sum of the 1s and 0s for each node. Hence it is
  // No. of nodes that have an interest in this unit.
  if (w->mpi_size>1) {
    unsigned char* admin_nodes_out = new unsigned char[w->no_units];
    unsigned char* admin_nodes_in = new unsigned char[w->no_units];
    for (int i=0; i<w->no_units; i++) admin_nodes_out[i]=w->a_units[i].no_nodes;   // Default, no_nodes is one or zero
    MPI_Allreduce(admin_nodes_out,admin_nodes_in,w->no_units,MPI_UNSIGNED_CHAR,MPI_SUM,MPI_COMM_WORLD);
    for (int i=0; i<w->no_units; i++) w->a_units[i].no_nodes=admin_nodes_in[i];
    delete admin_nodes_out;
    delete admin_nodes_in;
  }
}

void syncPPCPN(world* w) {
  // Called once at the beginning to agree on how many people in each country live on each node.

  if (w->mpi_size>1) {
    int* ppcpn_out = new int[w->no_countries*w->mpi_size];
    int x=0;
    for (int i=0; i<w->no_countries; i++) {
      for (int j=0; j<w->mpi_size; j++) {
        ppcpn_out[x]=w->people_per_country_per_node[i][j];
        x++;
      }
    }
    int* ppcpn_in = new int[w->no_countries*w->mpi_size];
    MPI_Allreduce(ppcpn_out,ppcpn_in,w->no_countries*w->mpi_size,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
    x=0;
    for (int i=0; i<w->no_countries; i++) {
      for (int j=0; j<w->mpi_size; j++) {
        w->people_per_country_per_node[i][j]=ppcpn_in[x];
        x++;
      }
    }
    
    delete ppcpn_out;
    delete ppcpn_in;
  }
}

void processUnitInfo(world* w) {
  int start = (w->mpi_size*w->mpi_size*3);
  
  for (int i=0; i<w->no_units; i++) {
    w->a_units[i].new_comm_cases+=starter_msg_in[start];
    start++;
    w->a_units[i].new_comm_infs+=starter_msg_in[start];
    start++;
    w->a_units[i].new_hh_cases+=starter_msg_in[start];
    start++;
    w->a_units[i].new_hh_infs+=starter_msg_in[start];
    start++;
    w->a_units[i].current_symptomatic_inf+=starter_msg_in[start];
    start++;
    w->a_units[i].current_nonsymptomatic_inf+=starter_msg_in[start];
    start++;
    for (unsigned int j=0; j<w->P->no_place_types; j++) {
      w->a_units[i].new_place_cases[j]+=starter_msg_in[start];
      start++;
      w->a_units[i].new_place_infs[j]+=starter_msg_in[start];
      start++;
    }
  }
}

void prepareUnitInfo(world* w) {

  // This goes in the "starter" message, since its fixed-width.

  // Format:
  //      new community/travel cases : int (ie detected)
  //      new community/travel infections : int
  //      new household cases : int (ie detected)
  //      new household infections : int
  //      For each place_type: 
  //        new place cases : int (ie detected)
  //        new place infections : int
  // 
  //   Hence, length = 6+2*(no_place_type) ints   per unit
  
  int start = (w->mpi_size*w->mpi_size*3);
  int total;
  for (int i=0; i<w->no_units; i++) {
    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_comm_cases[i][j];
      w->delta_comm_cases[i][j]=0;
    }
    starter_msg_out[start++]=total;
    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_comm_infs[i][j];
      w->delta_comm_infs[i][j]=0;
    }
    starter_msg_out[start++]=total;

    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_hh_cases[i][j];
      w->delta_hh_cases[i][j]=0;
    }
    starter_msg_out[start++]=total;

    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_hh_infs[i][j];
      w->delta_hh_infs[i][j]=0;
    }
    starter_msg_out[start++]=total;
    
    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_total_sympt_inf[i][j];
      w->delta_total_sympt_inf[i][j]=0;
    }
    starter_msg_out[start++]=total;

    total=0;
    for (int j=0; j<w->thread_count; j++) {
      total+=w->delta_total_nonsympt_inf[i][j];
      w->delta_total_nonsympt_inf[i][j]=0;
    }
    starter_msg_out[start++]=total;

    
    
    for (unsigned int j=0; j<w->P->no_place_types; j++) {
      total=0;
      for (int k=0; k<w->thread_count; k++) {
        total+=w->delta_place_cases[i][k][j];
        w->delta_place_cases[i][k][j]=0;
      }
      starter_msg_out[start++]=total;
      total=0;
      for (int k=0; k<w->thread_count; k++) {
        total+=w->delta_place_infs[i][k][j];
        w->delta_place_infs[i][k][j]=0;
      }
      starter_msg_out[start++]=total;
    }
  }
}


void addStatusInfo(world* w) {
  // Extra info to be synchronised.
  // Currently, just a flag: 0 = I'm completely finished, 1 = I still have infections to deal with.
  int start = 3*(w->mpi_size*w->mpi_size);
  start+=w->no_units*(6+(2*w->P->no_place_types));
  starter_msg_out[start]=w->continue_status;
}

void processStatusInfo(world* w) {
  int start = 3*(w->mpi_size*w->mpi_size);
  start+=w->no_units*(6+(2*w->P->no_place_types));
  w->continue_status=starter_msg_in[start];

}

void doMessage(world* w) {
  
#ifdef _USEMPI
  int error_code=0;
  for (int i=0; i<starter_msg_size; i++) {      // First empty the message arrays.
    starter_msg_out[i]=0;
    starter_msg_in[i]=0;
  }

  // The "starter" message lets all nodes know how many bytes they are expecting to receive. Format:-
  //    (Array of ints)

  // FIRST BLOCK:   starts at:   0
  //                length:      (mpi_size*mpi_size)
  //                content:     Number of bytes of community/travel REQUESTS sent from node i to node j.
  //
  // SECOND BLOCK:  starts at:   (mpi_size*mpi*size)
  //                length:      (mpi_size*mpi_size)
  //                content:     Number of bytes of community/travel REPLIES sent from node i to node j.
  //
  // THIRD BLOCK:  starts at:    2*(mpi_size*mpi*size)
  //                length:      (mpi_size*mpi_size)
  //                content:     Number of bytes of PLACE related info sent from node i to node j.
  // 
  // FOURTH BLOCK: starts at:    3*(mpi_size*mpi_size)
  //                length:      no_units*(4+(2*no_place_types))
  //                ontent:      See prepareUnitInfo(world)
  //
  // FIFTH BLOCK: leftovers...
  //               starts at:    3*(mpi_size*mpi_size)+(no_units*(6+(2_no_place_types)))
  //               length:       1
  //               format:       flag: 0 = "No more work to do" for each node. else 1. (Fine to reduce to a SUM)
  // 

  // Calculate number of bytes to send to each other node.


  int total_req_bytes_out=0;   // Total number of bytes to send out (ie, sum all destinations), due to community contact requests
  int total_rep_bytes_out=0;   // Total number of bytes to send out, due to replies to other nodes' community contact requests
  int total_place_bytes_out=0;   // Total number of bytes to send out due to place infections, closures and prophylactic events.
    
  msg_displs_out[0]=0;         // MPI wants displacements for each node. First displacement is zero obviously.
  for (int dest=0; dest<w->mpi_size; dest++) {   // Now for each node,
    int node_req_bytes_out=0;                    //   Calculate the number of bytes to send.
    int node_rep_bytes_out=0;
    int place_bytes_out=0;
    for (int thread=0; thread<w->thread_count; thread++) {
      node_req_bytes_out+=(int) w->remoteRequests[thread][dest].size();    // Sum requests for node 'dest' (per thread)
      node_rep_bytes_out+=(int) w->remoteReplies[thread][dest].size();     // Sum replies for node 'dest' (per thread)
      place_bytes_out+=(int) w->placeInfMsg[thread][dest].size();          // Sum place-infection info for node 'dest' (per thread)
      place_bytes_out+=(int) w->placeClosureMsg[thread][dest].size();      // Sum place-closure info for node 'dest' (per thread)
      place_bytes_out+=(int) w->placeProphMsg[thread][dest].size();        // Sum place-prophylaxis info for node 'dest' (per thread)
    }

    total_req_bytes_out+=node_req_bytes_out;                               // Accumulate complete totals (for all nodes)
    total_rep_bytes_out+=node_rep_bytes_out;                          
    total_place_bytes_out+=place_bytes_out;
    


    place_bytes_out+=12;   // Need 3 integers for "number" of each message type.
    total_place_bytes_out+=12; // for places - number of infs (34 bytes), closures (14 bytes) and prophylaxis (14 bytes)
    starter_msg_out[(dest*w->mpi_size)+w->mpi_rank]=node_req_bytes_out;                 // MSG_size_out is n*n grid of nodes.
    starter_msg_out[((dest+w->mpi_size)*w->mpi_size)+w->mpi_rank]=node_rep_bytes_out;   // shorthand for (size*size)+(size*dest)+rank
    starter_msg_out[(w->mpi_size*((2*w->mpi_size)+dest))+w->mpi_rank]=place_bytes_out;  // shorthand for (size*size)*2  + (size*dest)  + rank
 
    // Arrange outgoing message buffer in advance too.

    msg_counts_out[dest]=node_req_bytes_out+node_rep_bytes_out+place_bytes_out;       // Number of bytes to send to node 'dest'
    if (dest>0) msg_displs_out[dest]=msg_displs_out[dest-1]+msg_counts_out[dest-1];   // And need to specify buffer displacements for MPI
  }

  
  // And set up outgoing message itself (and clear some memory)

  unsigned char* message_out = new unsigned char[total_req_bytes_out+total_rep_bytes_out+total_place_bytes_out]; // Buffer for sending message.
  int count=0;

  for (int dest=0; dest<w->mpi_size; dest++) {
    for (int thread=0; thread<w->thread_count; thread++) {
      for (int byte=0; byte<w->remoteRequests[thread][dest].size(); byte++) {
        message_out[count]=w->remoteRequests[thread][dest][byte];
        count++;
      }
      w->remoteRequests[thread][dest].clear();
    }
    for (int thread=0; thread<w->thread_count; thread++) {
      for (int byte=0; byte<w->remoteReplies[thread][dest].size(); byte++) {
        message_out[count]=w->remoteReplies[thread][dest][byte];
        count++;
      }
      w->remoteReplies[thread][dest].clear();
    }
    SIM_I64 size=0;
    for (int thread=0; thread<w->thread_count; thread++) size += w->placeInfMsg[thread][dest].size();
    for (int i=0; i<4; i++) message_out[count++]=((unsigned char*)(&size))[i];
    for (int thread=0; thread<w->thread_count; thread++) {
      for (int byte=0; byte<w->placeInfMsg[thread][dest].size(); byte++) {
        message_out[count]=w->placeInfMsg[thread][dest][byte];
        count++;
      }
      w->placeInfMsg[thread][dest].clear();
    }
    size=0;
    for (int thread=0; thread<w->thread_count; thread++) size += w->placeClosureMsg[thread][dest].size();
    for (int i=0; i<4; i++) message_out[count++]=((unsigned char*)(&size))[i];
    for (int thread=0; thread<w->thread_count; thread++) {
      for (int byte=0; byte<w->placeClosureMsg[thread][dest].size(); byte++) {
        message_out[count]=w->placeClosureMsg[thread][dest][byte];
        count++;
      }
      w->placeClosureMsg[thread][dest].clear();
    }
    size=0;
    for (int thread=0; thread<w->thread_count; thread++) size += w->placeProphMsg[thread][dest].size();
    for (int i=0; i<4; i++) message_out[count++]=((unsigned char*)(&size))[i];
    for (int thread=0; thread<w->thread_count; thread++) {
      for (int byte=0; byte<w->placeProphMsg[thread][dest].size(); byte++) {
        message_out[count]=w->placeProphMsg[thread][dest][byte];
        count++;
      }
      w->placeProphMsg[thread][dest].clear();
    }
  }


  // And receive how many bytes are for us.
  prepareUnitInfo(w);
  addStatusInfo(w);
  error_code = MPI_Allreduce(starter_msg_out,starter_msg_in,starter_msg_size,MPI_INT,MPI_SUM,MPI_COMM_WORLD);                // Everyone ends up with a full copy of what messages are going where.
  processUnitInfo(w);
  processStatusInfo(w);


  if (w->log_movie) error_code = MPI_Reduce(&(w->image[0]),&image_message[0],PNG_WIDTH*PNG_HEIGHT,MPI_UNSIGNED_CHAR,MPI_SUM,0,MPI_COMM_WORLD);      // Do the image bit here too. Merge?

  // Arrange incoming memory space, with counts and displacements for MPI message
  
  w->total_req_bytes_in=0;
  w->total_rep_bytes_in=0;
  w->total_est_bytes_in=0;
  msg_displs_in[0]=0;
  
  for (int src=0; src<w->mpi_size; src++) {
    msg_counts_in[src]=starter_msg_in[(w->mpi_rank*w->mpi_size)+src]+starter_msg_in[((w->mpi_rank+w->mpi_size)*w->mpi_size)+src]+
        starter_msg_in[((w->mpi_rank+(2*w->mpi_size))*w->mpi_size)+src];

    if (src>0) msg_displs_in[src]=msg_displs_in[src-1]+msg_counts_in[src-1]; 
    w->total_req_bytes_in+=starter_msg_in[(w->mpi_rank*w->mpi_size)+src];
    w->req_bytes_from[src]=starter_msg_in[(w->mpi_rank*w->mpi_size)+src];
    w->total_rep_bytes_in+=starter_msg_in[((w->mpi_rank+w->mpi_size)*w->mpi_size)+src];
    w->rep_bytes_from[src]=starter_msg_in[((w->mpi_rank+w->mpi_size)*w->mpi_size)+src];
    w->total_est_bytes_in+=starter_msg_in[((w->mpi_rank+(2*w->mpi_size))*w->mpi_size)+src];
    w->est_bytes_from[src]=starter_msg_in[((w->mpi_rank+(2*w->mpi_size))*w->mpi_size)+src];
  }
    
  w->message_in = new unsigned char[w->total_req_bytes_in+w->total_rep_bytes_in+w->total_est_bytes_in];

  error_code = MPI_Alltoallv(message_out,msg_counts_out,msg_displs_out,MPI_UNSIGNED_CHAR,w->message_in,msg_counts_in,msg_displs_in,MPI_UNSIGNED_CHAR,MPI_COMM_WORLD);

  int total=0;
  for (int i=0; i<12; i++) total+=w->message_in[i];

  MPI_Barrier(MPI_COMM_WORLD);
  

  delete[] message_out;
  if ((w->log_movie) && (w->mpi_rank==0)) {
    for (int i=0; i<PNG_WIDTH*PNG_HEIGHT; i++) w->image[i]=image_message[i];
    saveImage(w);
  }
#else
  if (w->log_movie) saveImage(w);
#endif

}

/* sim.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Major contact algorithms, and MAIN function call.
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

#ifdef WIN32
  #include <windows.h>
  #define MEMORY_CHECK
#endif


#ifdef MEMORY_CHECK
  #include <Psapi.h>
  #pragma comment( lib, "psapi.lib" )
#endif

#include "sim.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



int errline;

extern "C" void handle_aborts(int signal_number) {
  printf("Abort called. Last debug code = %d\n",errline);
  fflush(stdout);
#ifdef MEMORY_CHECK
  PrintMemoryInfo(NULL,GetCurrentProcessId());
#endif

}


/*Do this early in your program's initialization */



void handleIncomingMessage(world *w) {              // An incoming message is stored in w->message_in. It contains requests/replies from multiple modes.
#ifdef _USEMPI

  // I've put it in here rather than messages.cpp, since it relies on all the contact functions.
  errline=1037;

  lwv::vector<SIM_I64> store_remote_address;
  lwv::vector<unsigned int> store_remote_rp;
  int thread_no;  // Has to be signed integer for OpenMP
  if ((w->total_rep_bytes_in>0) || (w->total_req_bytes_in>0) || (w->total_est_bytes_in>0)) {   // If there is any incoming message to deal with...

   // First, we need to visit the replies and set the control bytes, so that replies sent from different nodes (hence at arbitrary points in the compiled incoming message) are linked together, in such a way
   // that is computationally cheap to access them - in a threaded way. (ie, easy to skip over related reply messages that are linked together, as if they were one contiguous message).
   // This section can't be threaded, because the threads may race to a pair of messages that should be linked, but aren't yet.
   // Hence, performance is mildly concerning here, but since it enables full multi-threading of the following section, it is worth it for now.

    unsigned short type = 0;      // Either request (0) or reply (1). Requests come first.
    unsigned int pointer = 0;     // A "local" pointer - it points to a place in the current scope (reply or request, for a given node) of the incoming message.
    unsigned int msg_ptr = 0;     // A "global" pointer - into the whole incoming message.
    unsigned short src=0;         // Currently considering messages originating from this node.

    while (src<w->mpi_size) {                                                                             // Have we dealt with rep/req messages from all nodes?
      if ((type==REQUEST) && (pointer>=w->req_bytes_from[src])) { type=REPLY; pointer=0; }                // If we've run out of requests for this node, switch to replies.
      else if ((type==REPLY) && (pointer>=w->rep_bytes_from[src])) {                                      // If we've run out of replies, 
        msg_ptr+=w->est_bytes_from[src];                                                                  //   Skip ALL establishment-related messages.
        type=REQUEST; src++; pointer=0;                                                                   //   Then go back into "REQUEST" mode.
      } else {
        if (type==REQUEST) {               // If we get here, then a REQUEST message is at msg_ptr[0]. Skip it - only interested in linking replies here.
          unsigned short n_contacts = *(unsigned short*) (&(w->message_in)[msg_ptr+16]);                            // Get total no. of contacts in request fragment
          unsigned short n_remotes = *(unsigned short*) (&(w->message_in)[msg_ptr+18+(2*n_contacts)]);              // Get no. of remote contacts in request fragment
          unsigned short n_nodes = *(unsigned short*) (&(w->message_in)[msg_ptr+20+(2*n_contacts)+(11*n_remotes)]); // The number of nodes that need to be replied to.
          pointer+=22+(2*n_contacts)+(11*n_remotes)+(2*n_nodes);                                                    // Move "local" pointer on past the request fragment
          msg_ptr+=22+(2*n_contacts)+(11*n_remotes)+(2*n_nodes);                                                    // And move "big message" pointer on too

        
        } else if (type==REPLY) {          // If we get here, then a valid REPLY message is at msg_ptr[0]. This is interesting.
                                           // We want to find and link up any other REPLY messages that were to the same requestor as this one.
                                           // So here, msg_ptr will stay where it is, and pointer will go looking for other replies.

          SIM_I64 inf_address = *((SIM_I64*) &(w->message_in)[msg_ptr+1]);                    // Get the address of the infected host (this is the first reply, so it's definitely memory address)
          unsigned short home_node = *(unsigned short*) (&(w->message_in)[msg_ptr+9]);        // Which node is the infected host on?
          unsigned short replies_in_frag = *(unsigned short*) (&(w->message_in)[msg_ptr+11]); // How many replies in this fragment (since replies can be concatenated to save message overhead)
          unsigned int reply_pointer=0;
          infectedPerson* ip;
          int array_pointer=0;
          if (home_node==w->mpi_rank) {           // If the infected person is on this node, 
            ip = (infectedPerson*) inf_address;   // Then the address is obvious.
            reply_pointer=ip->reply_address;      

          } else {                                // Otherwise, infected person is on another node.
            
            int found=0;
            array_pointer=0;
            while ((found==0) && (array_pointer<store_remote_address.size())) {    // Look for them in the search list.
              if (store_remote_address.at(array_pointer)==inf_address) {
                found=1;
                reply_pointer=store_remote_rp[array_pointer];      // Found them. Pick up the reply pointer.
              } else array_pointer++;
            }

            if (found==0) {                                        // Didn't find them. Add them to the list.
              store_remote_address.push_back(inf_address);         // Add the address
              store_remote_rp.push_back(1);                        // Reply pointer is "1" - unset initially.
              reply_pointer=1;
              array_pointer=(unsigned int) (store_remote_address.size()-1);
            }
          }
          
          if (reply_pointer==1) {
            if (home_node==w->mpi_rank) ip->reply_address=msg_ptr;
            else store_remote_rp[array_pointer]=msg_ptr;
            w->message_in[msg_ptr]=CTRL_SINGLE_ADDR;

          } else {
            if (w->message_in[reply_pointer]==(unsigned char) CTRL_SINGLE_ADDR) w->message_in[reply_pointer]=(unsigned char) CTRL_FIRST_LINK;
            else if (w->message_in[reply_pointer]==(unsigned char) CTRL_LAST_ADDR) w->message_in[reply_pointer]=(unsigned char) CTRL_MID_LINK;
            SIM_I64 msg_ptr_i64=(SIM_I64) msg_ptr;
            for (int k=0; k<8; k++) w->message_in[reply_pointer+1+k] = ((unsigned char*)(&msg_ptr_i64))[k]; // Over-write old address with pointer to this reply message.
            w->message_in[msg_ptr]=(unsigned char) CTRL_LAST_ADDR;
          }
          msg_ptr+=13+(2*replies_in_frag);     // And skip 
          pointer+=13+(2*replies_in_frag);
        }
      }
    }

    store_remote_address.clear();
    store_remote_rp.clear();

   // Now the replies are linked together, we can process all the REQ/REP messages in a threadsafe way, treating
   // linked replies (ie, replies to the same requestor) as one linked list handled by one thread.
      
    #pragma omp parallel for private(thread_no) schedule(static,1)
    for (thread_no=0; thread_no<w->thread_count; thread_no++) {
      int i=0;
      int j=0;
      int k=0;
      unsigned short skip = thread_no;          // Each thread skips different number of records at the start, after which they skip thread_count records each time.
      unsigned int pointer = 0;                 // This is a relatively "local" pointer, in the current message block.
      unsigned int msg_ptr = 0;                 // This is the more "global" pointer, into the whole message.
      unsigned short src = 0;                   // This is the node that a message fragment was sent from.
      unsigned short type = REQUEST;            // The current type of message being processed. REQUESTs come first.
      unsigned char control = 0;                // The current control byte - for use in skipping linked messages.
      unsigned short n_replies = 0;             // Number of replies in a reply message
      unsigned short n_contacts = 0;            // Number of contacts required in a request message
      unsigned short n_nodes = 0;               // Number of nodes involved in a request message
      unsigned short n_remotes = 0;             // Number of remote contacts requested in a message. (May be >n_contacts)
      float lon=0;
      float lat=0;
      lwv::vector<unsigned short> orders;
      lwv::vector<SIM_I64> contacts;
      person* visitor_person = NULL;
      localPatch* visitor_patch = NULL;
      
      while (src<w->mpi_size) {                                                                              // Use "invalid src" as the flag to stop doing work.
        if ((type==REQUEST) && (pointer>=w->req_bytes_from[src])) {                                          // If we've run out of requests for this node, try replies.
          type=REPLY; 
          pointer=0;   
          if (pointer<w->rep_bytes_from[src]) {                                                                     // But also need to check we're positioned on a valid reply (ie, control=FIRST/ONLY)
            control = w->message_in[msg_ptr];                                                                       // since loop can exit here. So... get control byte
            while ((pointer<w->rep_bytes_from[src]) && ((control==CTRL_MID_LINK) || (control==CTRL_LAST_ADDR))) {   // While there are still replies to treat, and while control=MID/LAST, 
              n_replies = *(unsigned short*) (&(w->message_in)[msg_ptr+11]);                                        // Ignore the fragment - someone else has dealt with the "parent".
              pointer+=13+(2*n_replies);                                                                            // Move "local" pointer over the reply fragment
              msg_ptr+=13+(2*n_replies);
              if (pointer<w->rep_bytes_from[src]) control=w->message_in[msg_ptr];                                   // If there are still reply bytes, get the next control byte.
            }
          }
        }

        else if ((type==REPLY) && (pointer>=w->rep_bytes_from[src])) {          // If we've run out of replies,
          msg_ptr+=w->est_bytes_from[src];                                      // Skip establishment bytes for now.
          type=REQUEST;                                                         // Expect REQUEST message
          pointer=0;                                                            // from next node
          src++;
          
        // Up to this point, we have done no real work,
        // But after the following 'else', we know we are at the correct start
        // of some kind of message. Next, decide if it has to be processed by this thread.
        // If not, then skip it.

        } else if (skip>0) {
          if (type==REQUEST) {                                                                         // Skip a request fragment
            n_contacts = *(unsigned short*) (&(w->message_in)[msg_ptr+16]);                            // Get total no. of contacts in request fragment
            n_remotes = *(unsigned short*) (&(w->message_in)[msg_ptr+18+(2*n_contacts)]);              // Get no. of remote contacts in request fragment
            n_nodes = *(unsigned short*) (&(w->message_in)[msg_ptr+20+(2*n_contacts)+(11*n_remotes)]); // Get no. of 3rd party nodes involved in the request
            pointer+=22+(2*n_contacts)+(11*n_remotes)+(2*n_nodes);        // Move "local" pointer on past the request fragment
            msg_ptr+=22+(2*n_contacts)+(11*n_remotes)+(2*n_nodes);        // And move "big message" pointer on too
            skip--;
          } else if (type==REPLY) {                                                  // Need to skip a reply fragment.
            n_replies = *(unsigned short*) (&(w->message_in)[msg_ptr+11]);           // We know control=FIRST or ONLY.
            pointer+=13+(2*n_replies);                                               // Move on local and
            msg_ptr+=13+(2*n_replies);                                               // global msg pointers
            skip--;                                                                  // and it counts as a skip. But we might now have
            control = w->message_in[msg_ptr];                                        // jumped onto an "invalid" (MID/LAST) reply fragment.
            while ((pointer<w->rep_bytes_from[src]) && ((control==CTRL_MID_LINK) || (control==CTRL_LAST_ADDR))) {     // So ignore replies until either we run out, or we find one with a valid control byte.
              n_replies = *(unsigned short*) (&(w->message_in)[msg_ptr+11]);         // So we want to skip over these, but it doesn't count as a
              pointer+=13+(2*n_replies);                                             // skip, because some other thread will have dealt with the
              msg_ptr+=13+(2*n_replies);                                             // whole linked set of replies.
              if (pointer<w->rep_bytes_from[src]) control=w->message_in[msg_ptr];    // If there are still reply bytes, get the next control byte.
            }             // By the end of this loop, we're either positioned at the next valid reply, OR the END of the replies,
          }               // which will be caught by the REPLY condition about 30 lines ago.

        // FINALLY!
        // We are all ready, positioned at the start of a message that is for this thread to process.

        } else {                    // So Skip==0 and we're at the start of a valid msg.
          skip = w->thread_count;   // Reset skip counter for next time.
          if (type==REQUEST) {      // If it's a REQUEST message...
            visitor_person = NULL;     // Do the unpacking....
            visitor_patch = NULL;

            lon = *(float*) (&(w->message_in)[msg_ptr]);                        // Longitude of requestor
            lat = *(float*) (&(w->message_in)[msg_ptr+4]);                      // Latitude
            SIM_I64 addr = *(SIM_I64*) (&(w->message_in)[msg_ptr+8]);           // Memory address.
            n_contacts = *(unsigned short*) (&(w->message_in)[msg_ptr+16]);     // Total number of contacts requested
            n_remotes = *(unsigned short*) (&(w->message_in)[msg_ptr+18+(2*n_contacts)]);  // Number of remote contacts requested
            n_nodes = *(unsigned short*) (&(w->message_in)[msg_ptr+20+(2*n_contacts)+(11*n_remotes)]); // Number of nodes interested in how many contacts we make here.
            unsigned short remote_contacts_so_far=0;                               // Count *successful* remote contacts we made. (So we can stop if rcsf>n_contacts)
            unsigned int nodes_ptr = msg_ptr+22+(2*n_contacts)+(11*n_remotes);     // Index of the first 3rd party node that needs contact nos for our replies.
            i=0;                   // i will track how many contacts we've successfully made.
            localPatch* temporary_residence=NULL;

            while (i<n_remotes) {   // While we still want to try and make remote contacts
              unsigned int rc_ptr = msg_ptr+20+(2*n_contacts)+(11*i);                        // Points to location of n_remote +2 + (11*i), where i=0..n_remotes. (contact no. of each remote request)
              unsigned short contact_no = *(unsigned short*) (&(w->message_in)[rc_ptr]);     // Number of this contact in infector's contact list. (Which would include locals+remotes)
              float t_contact = *(float*) (&(w->message_in)[rc_ptr+2]);                      // Time of contact 
              unsigned short lx = *(unsigned short*) (&(w->message_in)[rc_ptr+6]);           // Landscan x of *target* patch
              unsigned short ly = *(unsigned short*) (&(w->message_in)[rc_ptr+8]);           // Landscan y of *target* patch - OR overwritten with eg. MSG_TRAVELLER=65535, MSG_VISITOR=65534, MSG_NULL_VISITOR=65533
              int size = 20*((int) *(unsigned char*) (&(w->message_in)[rc_ptr+10]));         // Size of *target* patch - (landscan cells). (Remote patches can be differend sizes)
              
              localPatch* lp;
              person* susceptible=NULL;

              if (ly==MSG_TRAVELLER) {  // So this is a request for a TRAVELLER.
                unsigned char first_flag= *(unsigned char*) (&(w->message_in)[rc_ptr+10]);
                if (first_flag==0) {    // We're about to choose a patch for our TRAVELLER to live in on holiday.
                                        // but only do this on the first time - they'll stay in one place for their holiday.
                  // Next, we choose a random "friend" out of the destination country. Our traveller will stay in their patch
                  temporary_residence = infectedPerson::getPatchForPerson(w,(int) (ranf_mt(thread_no)*
                      w->people_per_country_per_node[lx][w->mpi_rank]),(unsigned char) lx,thread_no);
                  lx=temporary_residence->x;    // Copy landscan x of friend
                  ly=temporary_residence->y;    // Copy landscan y of friend
                  size=temporary_residence->size;  // Copy patch-size of friend
                  lon = (float) lsIndexToLon(lx+(ranf_mt(thread_no)*size));  // But choose random longitude
                  lat = (float) lsIndexToLat(ly+(ranf_mt(thread_no)*size));  // And latitude within that patch.
                } else { 
                  lx=temporary_residence->x;    // Otherwise, copy  x,y,size (since we change lx and ly)
                  ly=temporary_residence->y;    // but don't re-choose temporary residence and long/lat.
                  size=temporary_residence->size;
                }

                // Now use the patch-to-patch matrices to choose community contact patch.
                // Ideally, we might want a different kernel function for when someone is on holiday,
                // as the gravity models may well be different. But for now, use the same kernel function as normal com.contact model

                lp = static_cast<localPatch*>(patch::getCommunityContactPatch(w,ranf_mt(thread_no),temporary_residence));
                if (lp->node==w->mpi_rank) susceptible=&lp->people[(int) (ranf_mt(thread_no)*lp->no_people)];
                else susceptible=NULL;
                // The above pair of lines are a simplification... If travel has been requested in country on another node,
                // then community contacts must also be on this node. This is a POSSIBLE BUG - but is hard to resolve
                // without doing a 3rd MPI stage, which is rather undesirable. Treating as **KNOWN ISSUE** and testing to
                // see how much of an impact this might have.

              } else {   // ly!=MSG_TRAVELLER - could be VISITOR, NULL_VISITOR, or not a visitor at all.

                if ((ly==MSG_VISITOR) || (ly==MSG_NULL_VISITOR)) {
                  if (visitor_person==NULL) { // So we've decided they are a visitor, but not decided who the "origin" person is yet.
                    unsigned int country = (unsigned int) lx;  // In this case, lx will be the country.
                    //if (lx-country>0.5) country++;   // Simple rounding to check we've not got (x-1).99999
                    int tries=0;  // There may be no susceptibles left.
                    while ((visitor_person==NULL) && (tries<100)) { // Try at most 100 times to find one.
                      tries++;    // Increase try no... and pick a random person in the specified country, on this node.
                      int person_no = (unsigned int) (ranf_mt(thread_no)* w->people_per_country_per_node[country][w->mpi_rank]);
                      for (int patch=0; patch<w->patches_in_country[country].size(); patch++) { // Loop to find which patch they live in.
                         if (person_no<w->localPatchList[w->patches_in_country[country].at(patch)]->no_people) {
                          visitor_person=&w->localPatchList[w->patches_in_country[country].at(patch)]->people[person_no]; // Locate person
                          if ((visitor_person->status & STATUS_SUSCEPTIBLE)>0) {  // Are they susceptible?
                            visitor_patch=w->localPatchList[w->patches_in_country[country].at(patch)]; // If so, we have our visitor. Store patch
                            visitor_person->status-=STATUS_SUSCEPTIBLE;   // So they were infected while visting somewhere.
                            visitor_person->status+=STATUS_CONTACTED;     // They've now been contacted some how.
                            lon = visitor_person->house->lon;             // Remember longitude of their house
                            lat = visitor_person->house->lat;             // Remember latitude of their house
                            
                            infectedPerson* ip = new infectedPerson(w,thread_no,visitor_person); // Create infected person object
                            ip->updateStats(w,thread_no,1,0);
                            ip->travel_plan=NULL;
                            unsigned int timeStepsAway = 8;                                            // Schedule fake recovery time.
                            timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;   // Modulo maths to choose the right list
                            w->recoveryQueue[thread_no][timeStepsAway].push_back(ip);      // And schedule recovery


                          } else if (tries>=100) {  // If you can't find a susceptible... then use the co-ordinates, but don't set the infected status
                            visitor_patch=w->localPatchList[w->patches_in_country[country].at(patch)];
                            visitor_person=&visitor_patch->people[person_no];
                            lon=visitor_person->house->lon;
                            lat=visitor_person->house->lat;

                            // This tries>=100 clause is a bit non-ideal - the travel matrix has said we should have a visitor, from here
                            // but there are no susceptible visitors who could have travelled and caught the infection.

                          } else {
                            visitor_person=NULL;   // Otherwise: reset visitor_person to NULL so that earlier condition will succeed and
                          }                        // we keep trying for a susceptible.
                          patch=(int) w->patches_in_country[country].size();   // Done - force patch loop to terminate.
                          // Or otherwise, skip this patch and all its members - person_no must be in a later patch.
                        } else person_no-=w->localPatchList[w->patches_in_country[country].at(patch)]->no_people;
                      }
                    }
                  }
                }   // Done MSG_VISITOR || MSG_NULL_VISITOR

                if (ly!=MSG_NULL_VISITOR) { // NULL_VISITOR means...
                  if (ly==MSG_VISITOR) {    // "Genuine" VISITOR message:-  find community patch
                    patch* p = patch::getCommunityContactPatch(w,ranf_mt(thread_no),visitor_patch);
                    while (p->node!=w->mpi_rank) p = patch::getCommunityContactPatch(w,ranf_mt(thread_no),visitor_patch);
                    // AGAIN, this is a *KNOWN ISSUE* that once remote node has been chosen, the patch for local contacts from that
                    // node must be found on the same node.
                    // Really need 3-stage MPI algorithm.

                    lx=p->x;
                    ly=p->y;
                    size=p->size;

                  }

                  // Since size was size of a remote patch, but here we're looking at corresponding local patches...
                  // Possible more than one local patch would fit into the region.

                  // Hence... count number of people in the original "remote" patch.


                  int total_pop=0;
                  for (int xx=lx; xx<size+lx; xx+=20)
                    for (int yy=ly; yy<size+ly; yy+=20)
                      if (w->localPatchLookup[xx/20][yy/20]>=0)
                        total_pop+=w->localPatchList[w->localPatchLookup[xx/20][yy/20]]->no_people;

                  total_pop = (int) (total_pop*ranf_mt(thread_no));  // Choose random person,
                  for (int xx=lx; xx<size+lx; xx+=20) {              // and then find them in the ordering.
                    for (int yy=ly; yy<size+ly; yy+=20) {
                      if (w->localPatchLookup[xx/20][yy/20]>=0) {
                        if (total_pop<w->localPatchList[w->localPatchLookup[xx/20][yy/20]]->no_people) {
                          lp=w->localPatchList[w->localPatchLookup[xx/20][yy/20]];
                          susceptible = &lp->people[total_pop];
                          yy=ly+size;
                          xx=lx+size;
                        } else total_pop-=w->localPatchList[w->localPatchLookup[xx/20][yy/20]]->no_people;
                      }
                    }
                  }
                
                  // So susceptible is now set.
                }
              }

              // By this stage, susceptible has been set by one of the above methods, and is the susceptible host.
              //                lx is the landscan x co-ordinate of the infector's patch
              //                ly is the landscan y co-ordinate of the infector's patch
              //                size is the size of that landscan cell. (Which will be local now)
              //                lp is patch where the susceptible is.
              //                lon,lat are absolute co-ordinates of the infector
              
          
              if (susceptible!=NULL) { 
                if ((susceptible->status & STATUS_SUSCEPTIBLE)>0) { // If susceptible, then use rejection algorithm
                  double D_kk2 = patch::distance(lp,lx,ly,size);              // Shortest distance between local patch and supplied patch co-ordinates
                  double r_ij = haversine(lon,lat,susceptible->house->lon,susceptible->house->lat);  // Absolute distance
                  double S_ij = unit::kernel_F(&w->a_units[susceptible->house->unit],r_ij);
                  S_ij /= unit::kernel_F(&w->a_units[susceptible->house->unit],D_kk2);                // Rejection criteria

                  if (ranf_mt(thread_no)<S_ij) {                                        // If accepted...
                    remote_contacts_so_far++;                                           //    New remote contact found
                    infectedPerson* ip = new infectedPerson(w,thread_no,susceptible);   //    ** Create new infected person object
                    ip->t_contact=t_contact;                                            // Contact time step.
                    ip->t_inf = w->P->getInfectiousPeriodLength(thread_no);             // Sample infectious period (HOURS)
                    
                    // NOTE, don't call ip.updateStats() yet - Reason: this contact is not yet
                    // certain to happen - may be other remote contacts from different nodes that
                    // make it unnecessary.

                    // Send reply message to all interested nodes

                    for (unsigned short j=0; j<n_nodes; j++) {
                      unsigned short reply_node = *(unsigned short*) (&(w->message_in)[nodes_ptr+(2*j)]);
                      if (reply_node!=w->mpi_rank) { // Exclude "me" as potential reply node.
                        addRemoteReply(w,thread_no,(infectedPerson*) addr, src, reply_node, (unsigned short)contact_no);
                      }
                    }

                    // And reply to originating node, which won't be one of the "interested nodes"

                    addRemoteReply(w,thread_no,(infectedPerson*) addr,(unsigned short)src, (unsigned short)src, (unsigned short)contact_no);

                    // However, if it's the simple case, with just the requestor node and replier node, then
                    // deal with the infection now. We can schedule it because we know we want this contact -
                    // while (i<n_remotes) loop at the top, means if it's just this node we're interested in,
                    // we must try to fulfil n_remotes requests.

                    if (n_nodes==1) {
                      float t_incub = (float) (ip->t_contact+w->P->getLatentPeriodLength(thread_no));           // Sample latent period
                      ip->personPointer->status-=STATUS_SUSCEPTIBLE;                                            // And now they've been contacted.
                      ip->personPointer->status+=STATUS_CONTACTED;                                              
                      
                      unsigned short timeStepsAway = (unsigned short) ((t_incub-w->T)/w->P->timestep_hours);       // Number of timesteps between now, and contact time
                      timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                        // Modulo maths to choose the right list
                      w->contactQueue[thread_no][timeStepsAway].push_back(ip);                                     // And schedule the individual for the contact-finding algorithm in that timestep.

                      timeStepsAway = (unsigned short) ((w->P->symptom_delay+t_incub-w->T)/w->P->timestep_hours);  // Calculate time of onset of symptoms
                      timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
                      w->symptomQueue[thread_no][timeStepsAway].push_back(ip);                                     // Schedule "detection" event



                    }
                    
                    if (remote_contacts_so_far==1) {                                              // If it's the first remote success
                      w->reqHostAddresses[thread_no][1-w->con_toggle].push_back(addr);               // Remember the address of the infected host
                      lwv::vector<unsigned short> order;                                          // Create a vector to remember the "order" of the local contacts
                      order.push_back(n_contacts);                                                // Remember how many local contacts the infected host has.
                      for (int j=0; j<n_contacts; j++) order.push_back(*(unsigned short*) (&(w->message_in)[msg_ptr+18+(2*j)]));   // Populate the order from the message
                      w->reqOrders[thread_no][1-w->con_toggle].push_back(order);                     // Store that order.
                      lwv::vector<SIM_I64> new_contacts;                                          // Create a vector to remember the remote contacts made by this host.
                      w->reqContactAddresses[thread_no][1-w->con_toggle].push_back(new_contacts);    // Store that vector.
                    } 
                    w->reqOrders[thread_no][1-w->con_toggle].back().push_back(contact_no);           // Remember the order number of this new remote contact
                    w->reqContactAddresses[thread_no][1-w->con_toggle].back().push_back((SIM_I64)ip);   // Remember the address of this new remote contact.

                  } 
                } // End if susceptible
 
              } // End if not null
              j=0;
              if ((remote_contacts_so_far)>=n_contacts) { // Now including successful remote contacts, 
                i=n_remotes;                              // if we achieved n_contacts, stop looking.
              }
              i++;
            } // End while (i<n_remotes)
            finaliseRemoteReply(w,thread_no); // Finish off the message.

            //////////////////////////////////////////////////
            // Requests done. Now deal with a reply message //
          
          } else if (type==REPLY) {

            unsigned short control = (unsigned short) w->message_in[msg_ptr];                  // The control bit - FIRST, ONLY, MID or LAST
            unsigned short home_node = *(unsigned short*) (&(w->message_in)[msg_ptr+9]);       // Which node is the infected host on?
            unsigned short node_success = *(unsigned short*) (&(w->message_in)[msg_ptr+11]);   // How many remote contact successes (from one node)? (Followed by a list of the order numbers)
            unsigned short local_success =0;
            unsigned short all_remote_success = node_success;                                  // Accumulate success from all nodes
            SIM_I64 address = *(SIM_I64*) (&(w->message_in)[msg_ptr+1]);

            if (control==CTRL_FIRST_LINK) {                            // This reply is multi-part - first link.
              unsigned short cmp_control=CTRL_UNSET;                   // cmp_control is our comparator:-
              while (cmp_control!=CTRL_LAST_ADDR) {                    // Look for the last part of the multi-part reply
                cmp_control = w->message_in[address];                  // Control byte for this message (address == message index if control != LAST/ONLY)
                all_remote_success += *(unsigned short*) (&(w->message_in)[address+11]);       // Sum total successes for all nodes
                address = *(SIM_I64*) (&(w->message_in)[address+1]);   // Final time, address will be actual memory address
                                                                       //             and cmp_control will be CTLR_LAST_ADDR.
              }                                                // and all_remote_success will be total number of remote successes!
            }
            // Note, although we've visited all parts of the multi-part reply message,
            // msg_ptr has not changed - we're still considering the first part below.

            unsigned short* sort_list;
            int sort_list_length=0;
            infectedPerson* infected;
            if (home_node==w->mpi_rank) {               // If this reply is to the originating node (which is me)
              infected = (infectedPerson*) address;     // Now got a pointer to the infected person.
              infected->reply_address=1;
              local_success = infected->n_contacts;     // This is actual number of contact wanted. (=number of local contacts speculatively made)
              sort_list_length=all_remote_success+local_success;  // Total potential contacts to consider.
              sort_list = new unsigned short[sort_list_length];   // Create an array for sorting.
              for (i=0; i<local_success; i++) sort_list[i]=infected->contact_order[i]; // Add the local contacts.
              // Then add the remote successes from the first part of the message.
              for (i=0; i<node_success; i++) sort_list[i+infected->n_contacts]=*(unsigned short*) (&(w->message_in)[msg_ptr+13+(i*2)]);

            } else {   // Otherwise, we're not the requesting node, but still need to decide whether the remote contacts we got are
              int thread_lookup=0;   // wanted or not.
              while (thread_lookup<w->thread_count) {    // Need to lookup the address of the requestor, and which "orders" were ours.
                int i_int=0;
                while (i_int<w->reqHostAddresses[thread_lookup][w->con_toggle].size()) {    // Lookup the address of the requestor...
                  SIM_I64 req_address = w->reqHostAddresses[thread_lookup][w->con_toggle].at(i_int);     // Got one:-
                  if (req_address==address) {                                                        // Got a match.
                    orders = w->reqOrders[thread_lookup][w->con_toggle].at(i_int);                      // Get the order numbers for these contacts
                    contacts = w->reqContactAddresses[thread_lookup][w->con_toggle].at(i_int);          // and the matching addresses (susceptibles on this node)
                    local_success = orders.front();
                    sort_list = new unsigned short[all_remote_success+local_success+contacts.size()];   // Create an array for sorting.
                    j=0;                                  //   orders contains "no of local successes", followed by orders of those local successes,
                    for (k=1; k<orders.size(); k++) {     //   followed by orders of "remote" successes established on this node.
                      sort_list[k-1]=orders.at(k);        //   Add all orders to the sort list.
                      j++;
                    }
                    SIM_I64 address = *(SIM_I64*) (&(w->message_in)[msg_ptr+1]); //   Address/link, depending on Control byte.
                    
                    if (control==CTRL_FIRST_LINK) { // if it's first, then address gives index into message for next part of reply msg.
                      int cmp_control=1;
                      while (cmp_control<=CTRL_LAST_ADDR) {         // If control byte is valid...
                        cmp_control = w->message_in[address];       // Control byte for this message
                        node_success = *(unsigned short*) (&(w->message_in)[address+11]);    // Sum total successes for all nodes
                        for (k=0; k<node_success; k++) {
                          sort_list[j]=*(unsigned short*) (&(w->message_in)[address+13+(2*k)]);  // And add to the sort list.
                          j++;
                        }
                        address= *(SIM_I64*) (&(w->message_in)[address+1]);              // Get address - if control=2, then it's the real address, otherwise location in array of next link
                        if (cmp_control==CTRL_LAST_ADDR) cmp_control=1+CTRL_LAST_ADDR;   // Set control to invalid, to end loop.
                      }
                    }
                    i_int=(int) w->reqHostAddresses[thread_lookup][w->con_toggle].size();   // Terminate looking for the address.
                  }
                  i_int++;  // Otherwise, keep looking for the address for this thread...
                }
                thread_lookup++; // And keep going through the thread structure too.
              }
            } // (end of the "are we the requesting node or not" if structure)
            
            if (sort_list_length>0) {                                                          // sort_list_length is unsigned, so check here, rather than the -1 loop...
              for (i=0; i<sort_list_length-1; i++) {
                unsigned short best_index= i;                                                  // This is a simple insertion sort - only quite short lists being dealt with here.
                for (j=i+1; j<sort_list_length; j++)  {                                        // So sort the contact numbers into increasing order.
                  if (sort_list[j]<sort_list[best_index]) best_index = j;
                }
                if (i!=best_index) {
                  unsigned short swap = sort_list[i];
                  sort_list[i]=sort_list[best_index];
                  sort_list[best_index]=swap;
                }
              }
            }

            // So we now have a list of contacts established from both the local, and remote node(s), sorted by
            // order no - WHICHEVER node we actually are. Consensus reached.
            // The first "n_contacts" of this list are to be kept.

            if (home_node==w->mpi_rank) {   // If we are the requesting node
              unsigned short contact_ptr=0;

              // Set infected->n_contacts so it is the actual number of "local" contacts we want - subtracting
              // the valid remote contacts.

              while ((contact_ptr<infected->n_contacts) && (infected->contact_order[contact_ptr]<=sort_list[infected->n_contacts-1]))
                contact_ptr++;                                // Allow all local contacts with order <= the 'n_contacts'th entry in the sort_list.
              infected->n_contacts=contact_ptr;               // This is the actual number of local contacts that we want.
              delete [] sort_list;
              
            } else {      // However, if we are actually a remote node...

              for (i=0; i<local_success; i++) {           // local_success is also total number of contacts required.
                unsigned short compare_mine=orders.at(0)+1; // Skip local parts here - just want to compare successful remote contacts
                while (compare_mine<orders.size()) {      // For each member in orders...
                  if (orders.at(compare_mine)==sort_list[i]) {  // both orders and sort_list are sorted by... order!
                    // So compare_mine might move slower than i, since there will be "orders" from other nodes in sort_list.
                    // But if i<local_success, then if we do find one of the "orders" is hours, then it's good to confirm.

                    infectedPerson* ip = (infectedPerson*) contacts.at(compare_mine-(orders.at(0)+1));
                    float t_incub = ip->t_contact+w->P->getLatentPeriodLength(thread_no); 
                    
                    // NB - don't do updateStats here.
                    // It will be done when the contact queue is processed.

                    ip->personPointer->status-=STATUS_SUSCEPTIBLE;
                    ip->personPointer->status+=STATUS_CONTACTED;                        // And now they've been contacted.
                                        
                    unsigned short timeStepsAway = (unsigned short) floor((t_incub-w->T)/w->P->timestep_hours);        // Number of timesteps between now, and contact time
                    timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;      // Modulo maths to choose the right list
                    w->contactQueue[thread_no][timeStepsAway].push_back(ip);                   // And schedule the individual for the contact-finding algorithm in that timestep.

                    timeStepsAway = (unsigned short) floor((w->P->symptom_delay+t_incub-w->T)/w->P->timestep_hours);
                    timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
                    w->symptomQueue[thread_no][timeStepsAway].push_back(ip);

                    compare_mine=(unsigned short) orders.size();                               // Terminate the loop
                  } else compare_mine++;                                                       // Otherwise keep looking
                }
              } // End local success loop
            } // End home_node!=rank
          } // End type==reply
        } // End inner src<w->mpi_size
      } // End outer src<w->mpi_size
    } // End thread loop OMP
    // Now deal with establishment messages.
    // Single-thread this - not worth the overhead.

    msg_ptr=0;
    for (src=0; src<w->mpi_size; src++) {
      msg_ptr+=w->req_bytes_from[src];
      msg_ptr+=w->rep_bytes_from[src];
      int place_bytes = *(int*) (&(w->message_in)[msg_ptr]);
      msg_ptr+=4;
      for (int i=0; i<place_bytes; i+=34) {
        unsigned char country = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned char place_type = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned int place_no = *(unsigned int*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        unsigned int host_no = *(unsigned int*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        double new_contact_time = *(double*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=8;
        double infectiousness = *(double*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=8;
        double t_inf = *(double*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=8;
        makePlaceContactRemote(w,0,country,place_type,place_no,host_no,t_inf,infectiousness,new_contact_time);
      }
      
      place_bytes = *(int*) (&(w->message_in)[msg_ptr]);
      msg_ptr+=4;
      for (int i=0; i<place_bytes; i+=14) {
        unsigned char country = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned char place_type = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned int place_no = *(unsigned int*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        float start = *(float*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        float end = *(float*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        w->places[country][place_type].at(place_no)->applyClosureRemote(w,start,end);
      }
     
      place_bytes = *(int*) (&(w->message_in)[msg_ptr]);
      msg_ptr+=4;
      for (int i=0; i<place_bytes; i+=14) {
        unsigned char country = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned char place_type = *(unsigned char*) (&(w->message_in)[msg_ptr]);
        msg_ptr++;
        unsigned int place_no = *(unsigned int*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        float start = *(float*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        float end = *(float*) (&(w->message_in)[msg_ptr]);
        msg_ptr+=4;
        w->places[country][place_type].at(place_no)->applyProphylaxisRemote(w,0,start,end);
      }
    }
  } // If num bytes...

  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    w->reqHostAddresses[thread_no][w->con_toggle].clear();
    for (int i=0; i<w->reqOrders[thread_no][w->con_toggle].size(); i++)
      w->reqOrders[thread_no][w->con_toggle].at(i).clear();
    w->reqOrders[thread_no][w->con_toggle].clear();
    for (int i=0; i<w->reqContactAddresses[thread_no][w->con_toggle].size(); i++)
      w->reqContactAddresses[thread_no][w->con_toggle].at(i).clear();
    w->reqContactAddresses[thread_no][w->con_toggle].clear();

  }
  
#endif
  errline=10650;
}

void makeCommunityContact(world *w, int thread_no, localPatch* infector_patch, double lon, double lat, 
    infectedPerson* infected, int& n_local, unsigned short& contact_no, short& n_contacts, float new_contact_time) {
  
  errline=10656;
  patch* location_susceptible = patch::getCommunityContactPatch(w,ranf_mt(thread_no),infector_patch);
  if (location_susceptible->node==w->mpi_rank) {                                                                          // If susceptible's patch is on local node, then pick individual (below)
    localPatch* localpatch_susceptible = static_cast<localPatch*>(location_susceptible);                                  //   It's definitely a local patch, so cast.
    person* susceptible = &localpatch_susceptible->people[(int)(localpatch_susceptible->no_people*ranf_mt(thread_no))];   //   Choose random susceptible
    bool border_restrict = false;
    if (w->a_units[infected->personPointer->house->unit].bc_deny_exit>=0) {
      int country1 = w->a_units[infected->personPointer->house->unit].country;
      int country2 = w->a_units[susceptible->house->unit].country;
      if (country1!=country2) {
        if (ranf_mt(thread_no)<w->a_units[infected->personPointer->house->unit].bc_deny_exit) {
          border_restrict = true;
        }
      }
    }

    if (!border_restrict) {
      double D_kk2 = patch::distance(infector_patch,location_susceptible);                                                  //   Dk,k' is shortest distance between patches
      double r_ij = haversine(lon,lat,susceptible->house->lon,susceptible->house->lat);                                     //   r_ij is absolute distance between people
      double S_ij = unit::kernel_F(&w->a_units[infected->personPointer->house->unit],r_ij);
      S_ij /= unit::kernel_F(&w->a_units[infected->personPointer->house->unit],D_kk2);
 
      if (ranf_mt(thread_no)<S_ij) {                        // Contact is accepted
        if ((susceptible->status&STATUS_SUSCEPTIBLE)>0) {      // If contact is susceptible  ***** THREAD SAFETY *****
          if (ranf_mt(thread_no)<susceptible->getSusceptibility(w,thread_no)*infected->getInfectiousness(w,new_contact_time,thread_no)) {
            infectedPerson* ip = new infectedPerson(w,thread_no,susceptible);  //   Create infected person object  ******** PERFORMANCE *******
            ip->t_contact=new_contact_time;                   //   Time of contact (currently float...)
            infected->contacts[n_local]=ip;                   //   Store pointer for infecter person - confirm later
            ip->travel_plan=NULL;                             //   For now, default is the susceptible has no travel plan. We fix this later.
            // Note - not going to update the stats here - because contact is not confirmed.
          }
     
        } else infected->contacts[n_local]=NULL;            // ELSE - contact was found, but was not susceptible.
        infected->contact_order[n_local]=contact_no;        // Keep track of which contact no. this local contact was.
        n_local++;                                          // Update local contact counter.
        contact_no++;                                       // Update "Global" contact counter.
      }
    } 
  } else {                              // ELSE - Susceptible is on a remote node
    bool border_restrict = false;
    if (w->a_units[infected->personPointer->house->unit].bc_deny_exit>=0) {
      int country1 = w->a_units[infected->personPointer->house->unit].country;
      int country2 = w->a_units[location_susceptible->unit].country;
      if (country1!=country2) border_restrict=true;
    }
    if (!border_restrict) {   
      if (contact_no<n_contacts*10) {     //   At most request 10 times as many remotes as locals. (Reduce MPI burden)
        addRemoteRequest(w,thread_no,location_susceptible,infected->personPointer->house->lon,infected->personPointer->house->lat,infected,new_contact_time,contact_no,location_susceptible->node);
        contact_no++;
      }
    }
  }
  errline=10708;
}

void makeHouseholdContacts(world* w, int thread_no, infectedPerson* infected) {
  errline=10712;
  double new_contact_time,p_contact;
  unsigned int i;
  localPatch* patch;
  person* susceptible;
  bool potential_trigger=false;
  double t_at_home=0.0;

  if (infected->travel_plan==NULL) t_at_home=infected->t_inf;
  else t_at_home=infected->t_inf-infected->travel_plan->duration;

  // Adjustment for household contacts if place closure has occurred.

  float adjust_for_swp_closure=1;
  if ((infected->personPointer->place_type>=0) && (infected->personPointer->place_type<=w->P->no_place_types)) {
    if (infected->personPointer->place<w->no_places[infected->personPointer->house->country][infected->personPointer->place_type]) {
      if (w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_start>=0) {
        if ((w->T>=w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_start)
          && (w->T<=w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_end)) {
           adjust_for_swp_closure=(float) w->a_units[infected->personPointer->house->unit].c_hh_mul;
        }
      }
    }
  }

  patch = w->localPatchList[infected->personPointer->house->patch];
  for (i=0; i<infected->personPointer->house->no_people; i++) {
  
    susceptible=&patch->people[infected->personPointer->house->first_person+i];
    if (susceptible!=infected->personPointer) {
      double seas=w->a_units[susceptible->house->unit].getSeasonality(w,susceptible->house->lat);
      p_contact=1-(exp((-w->a_units[susceptible->house->unit].B_hh*seas*(t_at_home/24.0))/(susceptible->house->no_people-1)));
      
      // Multiply p_contact if this household is quarantined
      if (susceptible->house->q_start>=0) {
        if ((w->T>=susceptible->house->q_start) && (w->T<=susceptible->house->q_end)) {
          p_contact*=w->a_units[susceptible->house->unit].q_hh_rate;
        }
      }
        
      p_contact*=adjust_for_swp_closure;
        
      if ((susceptible->status & STATUS_SUSCEPTIBLE)>0) {
        new_contact_time=infected->getNextContactWhileAtHomeOrWorking(w,thread_no);
        if (ranf_mt(thread_no)<p_contact) {
 
          if (ranf_mt(thread_no)<susceptible->getSusceptibility(w,thread_no)*infected->getInfectiousness(w,new_contact_time,thread_no)) {

            susceptible->status-=STATUS_SUSCEPTIBLE;
            susceptible->status+=STATUS_CONTACTED;

            potential_trigger=true;
            infectedPerson* ip = new infectedPerson(w,thread_no,susceptible);
            w->a_units[susceptible->house->unit].add_hh_case(w,susceptible->house->unit,thread_no,
                (ip->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED);
            ip->t_contact=(float)new_contact_time;
            ip->travel_plan=NULL;
            susceptible->house->susc_people--;
            ip->t_inf = w->P->getInfectiousPeriodLength(thread_no);          // Sample infectious period - HOURS
            float latent_period = w->P->getLatentPeriodLength(thread_no);    // Sample latent period - HOURS
            float latent_end = ip->t_contact+latent_period;                  // Calculate end of latent period
            ip->createTravelPlan(w, ip, thread_no, latent_end);
            int timeStepsAway = (int) ((latent_end-w->T)/w->P->timestep_hours);                    // Number of timesteps between now, and contact time

            timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                    // Modulo maths to choose the right list
            w->contactQueue[thread_no][timeStepsAway].push_back(ip);  // And schedule the individual for the contact-finding algorithm in that timestep.
            timeStepsAway = (int) ((w->P->symptom_delay+latent_end-w->T)/w->P->timestep_hours);
            timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
            w->symptomQueue[thread_no][timeStepsAway].push_back(ip);

            if ((ip->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED) potential_trigger=true;

          }
        } 
      }
    }
  }

  if (potential_trigger) {
    if (w->a_units[susceptible->house->unit].pph_susc>=0) susceptible->house->applyProphylaxis(w,thread_no);
    if (w->a_units[susceptible->house->unit].q_compliance>0) susceptible->house->applyQuarantine(w,thread_no);
  }
  errline=10794;
}

void makePlaceContactRemote(world* w, int thread_no, unsigned char country, unsigned char place_type, unsigned int place_no,
    unsigned int host_no, double t_inf, double infectiousness,double new_contact_time) {
    
  errline=10800;
  person* susceptible;
  place* e = w->places[country][place_type].at(place_no);
  
  int node_no=0;
  int group_no=0;
  unsigned int accumulator=0;

  while (accumulator+e->group_member_node_count[group_no][node_no]<=host_no) {
    accumulator+=e->group_member_node_count[group_no][node_no];
    node_no++;
    if (node_no>=w->mpi_size) {
      node_no=0;
      group_no++;
    }
  }

  host_no-=accumulator;  // On remote node, host_no will have had an offset. Remove it here.
  susceptible = e->local_members[group_no][host_no];
  if ((susceptible->status & STATUS_SUSCEPTIBLE)>0) {
    new_contact_time = (w->T+w->P->timestep_hours+(ranf_mt(thread_no)*t_inf)); // Pick random (uniform) time (hours) for a contact to be scheduled
    if (ranf_mt(thread_no)<susceptible->getSusceptibility(w,thread_no)*infectiousness) {
      infectedPerson* ip = new infectedPerson(w,thread_no,susceptible);
      w->a_units[susceptible->house->unit].add_place_case(w,susceptible->house->unit,place_type,thread_no,
          (ip->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED);
      ip->t_contact=(float)new_contact_time;
      ip->travel_plan=NULL;
      susceptible->status-=STATUS_SUSCEPTIBLE;
      susceptible->status+=STATUS_CONTACTED;
      ip->t_inf = w->P->getInfectiousPeriodLength(thread_no);              // Infectious period (hours)
      float latent_period = w->P->getLatentPeriodLength(thread_no);        // Latent period (hours)
      float latent_end = ip->t_contact+latent_period;                      // Calculate time of end of latent period
      ip->createTravelPlan(w, ip, thread_no, latent_end);
      int timeStepsAway = (int) ((latent_end-w->T)/w->P->timestep_hours);                    // Number of timesteps between now, and contact time
      timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                    // Modulo maths to choose the right list
      w->contactQueue[thread_no][timeStepsAway].push_back(ip);  // And schedule the individual for the contact-finding algorithm in that timestep.
      timeStepsAway = (int) ((w->P->symptom_delay+latent_end-w->T)/w->P->timestep_hours);
      timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
      w->symptomQueue[thread_no][timeStepsAway].push_back(ip);

    }
  }
  errline=10842;
}

void makePlaceContacts(world* w, int thread_no, infectedPerson* infected) {
  errline=10846;
 
  double new_contact_time;
  unsigned int i;
  person* susceptible;
  bool apply_proph_or_closure=false;
  unsigned char place_type = infected->personPointer->place_type;
  unsigned char country = infected->personPointer->house->country;
  double t_at_home=0.0;
  place* e = w->places[country][place_type].at(infected->personPointer->place);
  
  if ((e->closure_start<0) || (w->T<e->closure_start) || (w->T>e->closure_end)) {  // If the place is not closed...
  
    if (infected->travel_plan==NULL) t_at_home=infected->t_inf;
    else t_at_home=infected->t_inf-infected->travel_plan->duration;

    double p_contact = 1-exp((-w->a_units[infected->personPointer->house->unit].B_place[place_type]*
                       w->a_units[infected->personPointer->house->unit].getSeasonality(w,infected->personPointer->house->lat)*
                       (t_at_home/24.0))/(e->total_hosts-1));

    if (infected->personPointer->house->q_start>=0) {
      if ((w->T>=infected->personPointer->house->q_start) && (w->T<=infected->personPointer->house->q_end)) {
        p_contact*=w->a_units[infected->personPointer->house->unit].q_s_wp_rate;
      }
    }

    unsigned int n_contacts = (unsigned int) ignbin_mt(e->total_hosts-1,p_contact,thread_no);
    unsigned int accumulator=0;
    unsigned int within_group_contacts=(int) (0.5+(n_contacts*w->a_units[infected->personPointer->house->unit].P_group[infected->personPointer->place_type]));
    i=0;
    while (i<n_contacts) {
      unsigned int host_no;
      if (i<within_group_contacts) {              // Choose the within-group contacts
        host_no=(int) (ranf_mt(thread_no)*e->group_member_count[infected->personPointer->group]);
        for (int j=0; j<infected->personPointer->group; j++)
          host_no+=e->group_member_count[j]; // Add people in previous groups to host_no.
      
      } else {                                    // Choose the outside-group contacts
        host_no=(int) (ranf_mt(thread_no)*(e->total_hosts-e->group_member_count[infected->personPointer->group]));
      
        unsigned int host_counter=0;
        for (int j=0; j<infected->personPointer->group; j++) host_counter+=e->group_member_count[j];
        if (host_no>host_counter) host_no+=e->group_member_count[infected->personPointer->group];
      }

      int node_no=0;
      int group_no=0;
      accumulator=0;
      
      while (accumulator+e->group_member_node_count[group_no][node_no]<=host_no) {
        accumulator+=e->group_member_node_count[group_no][node_no];
        node_no++;
        if (node_no>=e->no_nodes) {
          node_no=0;
          group_no++;
        }
      }

      if ((e->no_nodes==1) || (node_no==w->mpi_rank)) {
        host_no-=accumulator;   // Remove offset - hosts will start from 0 in the array for local host.
        susceptible = e->local_members[group_no][host_no];

        if ((susceptible->status & STATUS_SUSCEPTIBLE)>0) {
          new_contact_time=infected->getNextContactWhileAtHomeOrWorking(w,thread_no);

          if (ranf_mt(thread_no)<susceptible->getSusceptibility(w,thread_no)*infected->getInfectiousness(w,new_contact_time,thread_no)) {
            infectedPerson* ip = new infectedPerson(w,thread_no,susceptible);
            w->a_units[susceptible->house->unit].add_place_case(w,susceptible->house->unit,place_type,thread_no,
                (ip->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED);
            ip->t_contact=(float)new_contact_time;
            ip->travel_plan=NULL;
            susceptible->status-=STATUS_SUSCEPTIBLE;
            susceptible->status+=STATUS_CONTACTED;
            ip->t_inf = w->P->getInfectiousPeriodLength(thread_no);
            float latent_period = w->P->getLatentPeriodLength(thread_no);
            float latent_end = ip->t_contact+latent_period; // Calculate end of latent period
            ip->createTravelPlan(w, ip, thread_no, latent_end);
            int timeStepsAway = (int) ((latent_end-w->T)/w->P->timestep_hours);                    // Number of timesteps between now, and contact time
            timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                    // Modulo maths to choose the right list
            w->contactQueue[thread_no][timeStepsAway].push_back(ip);  // And schedule the individual for the contact-finding algorithm in that timestep.
  
            timeStepsAway = (int) ((w->P->symptom_delay+latent_end-w->T)/w->P->timestep_hours);
            timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
            w->symptomQueue[thread_no][timeStepsAway].push_back(ip);

            if ((ip->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED) apply_proph_or_closure=true;
          }

        }
      } else {   // Need to send MPI request, as the host that belongs to this establishment is on another node.

        new_contact_time = (w->T+w->P->timestep_hours+(ranf_mt(thread_no)*infected->t_inf)); // Pick random (uniform) time (hours) for a contact to be scheduled
        double infectiousness = infected->getInfectiousness(w,new_contact_time,thread_no);
        addPlaceInfectionMsg(w,thread_no,node_no,infected->personPointer->house->country,infected->personPointer->place_type,
            infected->personPointer->place,host_no,new_contact_time,infectiousness,infected->t_inf);

      }

      i++;
    }
    if (apply_proph_or_closure) {
      if (w->a_units[susceptible->house->unit].pph_susc>=0) {
        if (ranf_mt(thread_no)<w->a_units[susceptible->house->unit].pph_coverage)
        w->places[susceptible->house->country][susceptible->place_type].at(susceptible->place)->applyProphylaxis(w,thread_no,susceptible->place_type,susceptible->place);
      }
      if (w->a_units[susceptible->house->unit].c_threshold>=0)
        w->places[susceptible->house->country][susceptible->place_type].at(susceptible->place)->applyClosure(w,thread_no,susceptible->house->unit,susceptible->place_type,susceptible->place);
    }    
  } else if (w->T>e->closure_end) {
    e->closure_start=-1;
    e->acc_cases=0;
  }
  errline=10958;
}

void processConfirmationQueue(world *w) {
  errline=10962;
  int thread_no;

  // We now process the confirmations list that was generated a timestep ago, and was possible updated by the MPI communication that has just happened.
  // We process confirmQueue[thread][0 or 1][individuals]. Note that again, the thread index is just for safety in building this list, and we will
  // trickle through the threads as if they were one concatenated list, for optimum load balancing.
  // Since confirmation always happens in the timestep following the arrangement of the contacts, we only need 2 timesteps here, which can toggle (middle array index).

  // So, for each newly infected host, for each contact it has made, we schedule the moment when that contact will become infected, and choose its victims.
  // We also schedule the time when the infected host will recover - storing it in a separate array for each thread.
  w->con_toggle=1-w->con_toggle;   // Deal with the confirmations from the previous timestep (buffered)

  #pragma omp parallel for private(thread_no) schedule(static,1)
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    int queue_no=0;
    int person_no=thread_no;
    infectedPerson* infected;
    int j;

    int timeStepsAway;
    while ((queue_no<w->thread_count) && (person_no>=w->confirmQueue[queue_no][w->con_toggle].size())) {
      person_no-=(int) w->confirmQueue[queue_no][w->con_toggle].size();
      queue_no++;
    }
    while (queue_no<w->thread_count) {
      infected = w->confirmQueue[queue_no][w->con_toggle].at(person_no);
      if (infected==NULL) {
        printf("%d,%d,%d, CQ Infected=NULL\n",w->mpi_rank,thread_no,w->T);
        fflush(stdout);
      } else {
        if ((infected->personPointer->house->susc_people>0) &&
           (infected->personPointer->house->no_people>0)) makeHouseholdContacts(w,thread_no,infected);
        if ((infected->personPointer->place_type<w->P->no_place_types)
           && (infected->personPointer->place<w->no_places[infected->personPointer->house->country][infected->personPointer->place_type]))
           makePlaceContacts(w,thread_no,infected);

        for (j=0; j<infected->n_contacts; j++) {         // For each new contact that individual i has chosen
          if (infected->contacts[j]!=NULL) {
            if ((infected->contacts[j]->personPointer->status & STATUS_SUSCEPTIBLE)>0) {      // They are susceptible...
              infected->contacts[j]->personPointer->status-=STATUS_SUSCEPTIBLE;               // No longer susceptible
              infected->contacts[j]->personPointer->status+=STATUS_CONTACTED;                 // Now contacted.
              infected->contacts[j]->t_inf = w->P->getInfectiousPeriodLength(thread_no);      // Set infectious period
              float latent_period = w->P->getLatentPeriodLength(thread_no);                   // Set latent period
              float latent_end = infected->contacts[j]->t_contact+latent_period;              // Calculate end of latent period
              infected->contacts[j]->createTravelPlan(w, infected->contacts[j], thread_no, latent_end);  // Decide travel plan
              w->a_units[infected->contacts[j]->personPointer->house->unit].add_comm_case(w,
                  infected->contacts[j]->personPointer->house->unit,thread_no,
                  (infected->contacts[j]->flags & (SYMPTOMATIC+DETECTED))==SYMPTOMATIC+DETECTED);

              // Schedule end of latent period

              timeStepsAway = (int) ((latent_end-w->T)/w->P->timestep_hours);                    // Number of timesteps between now, and contact time
              timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                    // Modulo maths to choose the right list
              w->contactQueue[thread_no][timeStepsAway].push_back(infected->contacts[j]);  // And schedule the individual for the contact-finding algorithm in that timestep.

              // Schedule onset of symptoms

              timeStepsAway = (int) ((latent_end+w->P->symptom_delay-w->T)/w->P->timestep_hours);
              timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;
              w->symptomQueue[thread_no][timeStepsAway].push_back(infected->contacts[j]);
            }
          }
        }

        delete [] infected->contacts;     // All done with the contacts.
        infected->contacts=NULL;
        infected->n_contacts=0;
        timeStepsAway = (int) ((w->P->timestep_hours+infected->t_inf)/w->P->timestep_hours);   // Now schedule individual i's recovery - T_inf from now.
        timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;                             // Modulo maths again,
        
        w->recoveryQueue[thread_no][timeStepsAway].push_back(infected);                       // And schedule.
      }

       // Next individual
      person_no+=w->thread_count;
      while ((queue_no<w->thread_count) && (person_no>=w->confirmQueue[queue_no][w->con_toggle].size())) { // Dealt with all individuals in this thread queue.
        person_no-=(int)w->confirmQueue[queue_no][w->con_toggle].size();
        queue_no++;
      }
    } // End of while loop - continue tricking through the arrays of confirmations.
  } // Have to break here since previous section adds to symptom queue

  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    w->confirmQueue[thread_no][w->con_toggle].clear();
  }
  errline=101047;
}

void processSymptomaticQueue(world *w) {
  errline=101051;
  // And process symptomatic queue - which is independent of all other things,
  // except by mitigation. It's for firing detection events at the right time.

  int thread_no;
  #pragma omp parallel for private(thread_no) schedule(static,1)
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    int queue_no=0;
    int person_no=thread_no;
    infectedPerson* infected;
    while ((queue_no<w->thread_count) && (person_no>=w->symptomQueue[queue_no][w->infectionMod].size())) {
      person_no-=(int) w->symptomQueue[queue_no][w->infectionMod].size();
      queue_no++;
    }

    while (queue_no<w->thread_count) {
      infected = w->symptomQueue[queue_no][w->infectionMod].at(person_no);
      if (infected==NULL) {
        printf("%d,%d,%d, SQ Infected=NULL\n",w->mpi_rank,thread_no,w->T);
        fflush(stdout);
      } else {
        infected->updateStats(w,thread_no,1,0);
      }
      person_no+=w->thread_count;
      while ((queue_no<w->thread_count) && (person_no>=w->symptomQueue[queue_no][w->infectionMod].size())) { // Dealt with all individuals in this thread queue.
        person_no-=(int)w->symptomQueue[queue_no][w->infectionMod].size();
        queue_no++;
      }
    }
  } // End of OpenMP thread loop
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    w->symptomQueue[thread_no][w->infectionMod].clear();
  }
  errline=101083;
}

void processRecoveryQueue(world* w) {
  int thread_no;
  errline=101088;

  #pragma omp parallel for private(thread_no) schedule(static,1)
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    int queue_no=0;
    int person_no=thread_no;
    infectedPerson* infected;
    while ((queue_no<w->thread_count) && (person_no>=w->recoveryQueue[queue_no][w->infectionMod].size())) {
      person_no-=(int) w->recoveryQueue[queue_no][w->infectionMod].size();
      queue_no++;
    }

    while (queue_no<w->thread_count) {
      infected = w->recoveryQueue[queue_no][w->infectionMod].at(person_no);

      if (infected!=NULL) {
        infected->personPointer->status-=STATUS_CONTACTED;
        infected->personPointer->status+=STATUS_IMMUNE;
        infected->updateStats(w,thread_no,-1,1);
        delete infected;

      } else {
        printf("%d,%d,%d, RQ Infected=NULL\n",w->mpi_rank,thread_no,w->T);
        fflush(stdout);
      }

      // Next individual
      person_no+=w->thread_count;
      while ((queue_no<w->thread_count) && (person_no>=w->recoveryQueue[queue_no][w->infectionMod].size()))  { // Dealt with all individuals in this thread queue.
        person_no-=(int)w->recoveryQueue[queue_no][w->infectionMod].size();
        queue_no++;
      }
    }
  }

  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    w->recoveryQueue[thread_no][w->infectionMod].clear();
  }
  errline=101126;
}

void processContactQueue(world *w) {
  errline=101130;

  int thread_no=0;

  // Process a list of individuals who will become infected in timestep t.
  // The list of individuals is spread over contactLists[thread][timestep][individuals] - but the use of the thread index 
  // is only to ensure write-safety, so for this list, OpenMP should treat the list as if all the per-thread arrays were concatenated into one list of individuals for this timstep.
  
  // For each individual, decisions are made about where the individual was (ie, at home or away) - and if away, then the location.
  // Each contact that was chosen is added to the contact list for that individual.
  // The individual is then added to the confirm queue - and when replies of any MPI messages are received in the following timestep, the "tentative" contacts are then completed.



  #pragma omp parallel for private(thread_no) schedule(static,1)
  for (thread_no=0; thread_no<w->thread_count; thread_no++) {
    unit* i_unit;
    int done_contact=0;
    int n_local;
    unsigned short contact_no=0; 
    unsigned char first_travel=0;
    float new_contact_time;
    infectedPerson* infected;
    short n_contacts;
    localPatch *visitor_patch;
    person *visitor_person;
    double visitor_relocate_lon,visitor_relocate_lat;           // To store "temporary" lat/lon for a visitor.
    unsigned char visitor_unset;         // A flag that is '1' when a visitor has not been "identified" at his source location. (Has to be done!)
    int queue_no=0;
    int person_no=thread_no;
    while ((queue_no<w->thread_count) && (person_no>=w->contactQueue[queue_no][w->infectionMod].size())) {
      person_no-=(int) w->contactQueue[queue_no][w->infectionMod].size();
      queue_no++;
    }
    while (queue_no<w->thread_count) {
      contact_no=0;
      visitor_unset=1;
      visitor_person=NULL;
      visitor_patch=NULL;
      visitor_relocate_lat=-999;  // If I am a visitor, may need to set a temporary lat and lon. (exactly once).

      infected = w->contactQueue[queue_no][w->infectionMod].at(person_no);
      w->a_units[infected->personPointer->house->unit].contact_makers[thread_no]++;
      int parent=w->a_units[infected->personPointer->house->unit].parent_id;
      while (parent!=-1) {
        w->a_units[parent].contact_makers[thread_no]++;
        parent=w->a_units[parent].parent_id;
      }

      w->confirmQueue[thread_no][w->con_toggle].push_back(infected);
      i_unit = &w->a_units[infected->personPointer->house->unit];
      double p_contact = (infected->t_inf/24.0)*i_unit->B_spat*i_unit->getSeasonality(w,infected->personPointer->house->lat);
      if ((infected->personPointer->place_type>=0) && (infected->personPointer->place_type<=w->P->no_place_types)) {
        if (infected->personPointer->place<w->no_places[infected->personPointer->house->country][infected->personPointer->place_type]) {
          if (w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_start>=0) {
            if ((w->T>=w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_start) &&
              (w->T<=w->places[infected->personPointer->house->country][infected->personPointer->place_type].at(infected->personPointer->place)->closure_end)) {
                p_contact*=w->a_units[infected->personPointer->house->unit].c_comm_mul;
            }
          }
        }
      }
      
      // Modify probability if household quarantine is on

      if (infected->personPointer->house->q_start>=0) {
        if ((w->T>=infected->personPointer->house->q_start) && (w->T<=infected->personPointer->house->q_end)) {
          p_contact*=w->a_units[infected->personPointer->house->unit].q_community;
        }
      }

      // Modify probability for place absenteeism

      if ((infected->flags & SEVERE)>0) {
        if (ranf_mt(thread_no)<i_unit->abs_place_sev[infected->personPointer->place_type]) {
          p_contact*=i_unit->abs_place_sev_cc_mul[infected->personPointer->place_type];
        }
      } else if ((infected->flags & SYMPTOMATIC)>0) {
        if (ranf_mt(thread_no)<i_unit->abs_place_sympt[infected->personPointer->place_type]) {
          p_contact*=i_unit->abs_place_sympt_cc_mul[infected->personPointer->place_type];
        }
      }
      
      
      n_contacts = (short) ignpoi_mt(p_contact,thread_no);    // Calculate number of community contacts.
      infected->n_contacts=n_contacts;
      infected->contacts = new infectedPerson *[n_contacts];
      for (int i=0; i<n_contacts; i++) {
        infected->contacts[i]=NULL;
      }
      infected->contact_order = new unsigned short[n_contacts];
      n_local=0;                  // Count local contacts. (Remove unnecessary ones later if remote contacts are found)
      first_travel=0;             // A flag to indicate the first MPI travel message (for efficiency when receiving)
      while (n_local<n_contacts) {
        done_contact=0;
        new_contact_time = (float) (w->T+w->P->timestep_hours+(ranf_mt(thread_no)*infected->t_inf));                     // Pick random (uniform) time (hours) for a contact to be scheduled
        
        // Establish Long-Range Contacts
        if (infected->travel_plan!=NULL) {                                                                                          // Either traveller or visitor  
          if (infected->travel_plan->traveller==TRAVELLER) {                                                                        // Actually, Traveller
            if ((new_contact_time>=infected->travel_plan->t_start) && (new_contact_time<=infected->travel_plan->t_start+infected->travel_plan->duration)) { // They are away at this time
              if (infected->travel_plan->travel_node==w->mpi_rank) {
                if (infected->travel_plan->x<-500) infectedPerson::locateTravel(w,infected,thread_no);                              // 1st time - get (x,y) for traveller
                makeCommunityContact(w,thread_no,infected->travel_plan->patch,infected->travel_plan->x,infected->travel_plan->y,infected,n_local,contact_no,n_contacts,new_contact_time);
                done_contact=1;

              } else {      // The traveller was on another node at that time...
                if (contact_no<n_contacts*10) {     // At most request 10 times as many remotes as locals. ( Just to reduce MPI burden. ** Check whether this ever causes undersample. Unlikely. )
                  addTravelRequest(w,thread_no,infected,new_contact_time,contact_no,first_travel,MSG_TRAVELLER);
                  first_travel=1; // All travel is to same node, on same thread, so one flag is enough.
                  contact_no++;
                }
              }
            }  // Traveller is at home at this time - don't do anything - continue with usual contact-making algorithm.
            
          } else /* if (infected->travel_plan->traveller==VISITOR) */ {
            if ((new_contact_time>=infected->travel_plan->t_start) && (new_contact_time<=infected->travel_plan->t_start+infected->travel_plan->duration)) { // Is "on holiday here" at this time
              if (visitor_relocate_lat<-500) {                                        // if a temporary latitude has not yet been set...
                visitor_relocate_lat = lsIndexToLat(w->localPatchList[infected->personPointer->house->patch]->x+(ranf_mt(thread_no)*w->localPatchList[infected->personPointer->house->patch]->size));
                visitor_relocate_lon = lsIndexToLon(w->localPatchList[infected->personPointer->house->patch]->y+(ranf_mt(thread_no)*w->localPatchList[infected->personPointer->house->patch]->size));
              }
              makeCommunityContact(w,thread_no,w->localPatchList[infected->personPointer->house->patch],visitor_relocate_lon,visitor_relocate_lat,infected,n_local,contact_no,n_contacts,new_contact_time);
              done_contact=1;
            } else {    // Not in the visitor window - so they're at "home" wherever that is.
              if (infected->travel_plan->travel_node==w->mpi_rank) { // Country is on this node
                if (infected->travel_plan->x<-500) infectedPerson::locateTravel(w,infected,thread_no);
 
              // Find local patch where the visitor comes from.
                int tries=0;
                while ((visitor_unset==1) && (tries<100)) {   // The tries<100 is for the rare case where no susceptibles are left in the country, which case we bite the bullet.
                  tries++;
                  visitor_patch = infectedPerson::getPatchForPerson(w,(int) (ranf_mt(thread_no)*w->people_per_country_per_node[infected->travel_plan->country][w->mpi_rank]),infected->travel_plan->country,thread_no);
                  
                  visitor_person = &visitor_patch->people[(int) (ranf_mt(thread_no)*visitor_patch->no_people)];
                  if ((visitor_person->status & STATUS_SUSCEPTIBLE)>0) {
                    visitor_person->status-=STATUS_SUSCEPTIBLE;
                    visitor_person->status+=STATUS_CONTACTED;
                    infectedPerson* ip = new infectedPerson(w,thread_no,visitor_person);
                    ip->travel_plan=NULL;
                    ip->updateStats(w,thread_no,1,0);
                    unsigned int timeStepsAway = 8; // Number of timesteps between now, and contact time
                    timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow; // Modulo maths to choose the right list
                    w->recoveryQueue[thread_no][timeStepsAway].push_back(ip);             // And schedule recovery
                    
                    visitor_unset=0;
                  }
                }
            

                  // Now establish a contact.
                if (visitor_person!=NULL) { 
                  makeCommunityContact(w,thread_no,visitor_patch,visitor_person->house->lon,visitor_person->house->lat,infected,n_local,contact_no,n_contacts,new_contact_time);
                }
                done_contact=1;

              } else {              // Country is on other node
                if (contact_no<n_contacts*10) {
                  addTravelRequest(w, thread_no,infected,new_contact_time,contact_no,first_travel,MSG_VISITOR);   // Send special visitor message.
                  first_travel=1;
                }
                visitor_unset=0;
                contact_no++;
                done_contact=1;
              }
            }
          }
        } // Actually, no travel plan.
        if ((done_contact==0) && ((infected->travel_plan==NULL) || (infected->travel_plan->traveller==TRAVELLER))) {    // If we haven't found a contact through other means, and we live here, then basic community contact.
          makeCommunityContact(w,thread_no,w->localPatchList[infected->personPointer->house->patch],infected->personPointer->house->lon,infected->personPointer->house->lat,infected,n_local,contact_no,n_contacts,new_contact_time);
        } 
        
        if (contact_no>n_contacts*10) {    // If for the visitor case especially, it seems like we're not going to get any local contacts...
          for (unsigned int j=n_local; j<(unsigned int) n_contacts; j++) infected->contact_order[j]=(unsigned short) (contact_no+j);
          n_local=n_contacts;
        }
      } // End of find contacts loop
      // If the person was a visitor, check that the "source" is now immune.
      if (infected->travel_plan!=NULL) {
        if (infected->travel_plan->traveller==VISITOR) {

          if (visitor_unset==1) {
            if (infected->travel_plan->travel_node==w->mpi_rank) {  // Home node
              int tries=0;
              while ((visitor_unset==1) && (tries<100)) {   // The tries<100 is for the rare case where no susceptibles are left in the country, which case we bite the bullet.
                tries++;
                visitor_patch = infectedPerson::getPatchForPerson(w,(int) (ranf_mt(thread_no)*w->people_per_country_per_node[infected->travel_plan->country][w->mpi_rank]),infected->travel_plan->country,thread_no);
                  
                visitor_person = &visitor_patch->people[(int) (ranf_mt(thread_no)*visitor_patch->no_people)];
                if ((visitor_person->status & STATUS_SUSCEPTIBLE)>0) {
                  visitor_person->status-=STATUS_SUSCEPTIBLE;
                  visitor_person->status+=STATUS_CONTACTED;
                  infectedPerson* ip = new infectedPerson(w,thread_no,visitor_person);
                  ip->updateStats(w,thread_no,1,0);
                  ip->t_inf=24; // Dummy to make sure recovery happens reasonably.
                  unsigned int timeStepsAway = (unsigned int) ((ip->t_inf+w->P->timestep_hours)/w->P->timestep_hours);       // Number of timesteps between now, and contact time
                  timeStepsAway = (w->infectionMod+timeStepsAway) % w->P->infectionWindow;             // Modulo maths to choose the right list
                  w->recoveryQueue[thread_no][timeStepsAway].push_back(ip);             // And schedule recovery
                  visitor_unset=0; 
                }
              }
              // Send the message
            } else { // Visitor on remote node
                  
              addTravelRequest(w, thread_no,infected,0,0,first_travel,MSG_NULL_VISITOR);
              contact_no=n_local+1;                         // Ensure that finaliseRemoteRequest is called.

            }
          }

        }
      }
      if (contact_no>n_local) {
         
        finaliseRemoteRequest(w,thread_no,infected);
      }
      // Next individual
      person_no+=w->thread_count;
      while ((queue_no<w->thread_count) && (person_no>=w->contactQueue[queue_no][w->infectionMod].size())) { // Dealt with all individuals in this thread queue.
        person_no-=(int) w->contactQueue[queue_no][w->infectionMod].size();
        queue_no++;
      }
    } // end of while loop - continue through lists of individuals until there are no more thread-queues left.
  } // next thread number (just for OpenMP)
  
  for (thread_no=0; thread_no<w->thread_count; thread_no++)
    w->contactQueue[thread_no][w->infectionMod].clear();
  errline=101357;
}


void runSim(world *w) {
  w->T=0;                                  // w->T is time in hours. 
  w->continue_status=1;                    // continue_status>=1 means there is work to do (on any node).
  while (w->continue_status>=1) {          // See messages.cpp for synchronisation of continue_status.
    w->continue_status=0;                  // Suppose that there's nothing left to do... then set to 1 if we find there is still work.
    if (w->P->next_seed<w->P->no_seeds) w->continue_status=1;  // If we haven't performed all seed events yet, definitely continue
    else {                                                     // Otherwise...
      for (int i=0; i<w->thread_count; i++) {
        if ((w->confirmQueue[i][0].size()>0) || (w->confirmQueue[i][1].size()>0)) {   // If there's a contact waiting to be confirmed, 
          w->continue_status=1;                                                       // then continue
          i=w->thread_count;                                                          // And abort this loop
        } else {
          for (int j=0; j<w->P->infectionWindow; j++) {                               // Otherwise, if there's anything anywhere in the queues...
            if ((w->contactQueue[i][j].size()>0) || (w->symptomQueue[i][j].size()>0) || (w->recoveryQueue[i][j].size()>0)) {
              w->continue_status=1;                                                   // Continue
              j=w->P->infectionWindow;                                                // And abort the loops.
              i=w->thread_count;
            }
          } 
        }
      }
    }
    w->T_day=(float) (1.0*w->T/24.0);        // Calculate day number for convenience
    
    seedScheduledInfections(w);              // Check for any seed events
    processContactQueue(w);                  // Deal with people who become infected and schedule their contacts this timestep 
    doMessage(w);                            // Send MPI messages for next timestep, and receive replies from last timestep
    handleIncomingMessage(w);                // Process the incoming MPI message
    processConfirmationQueue(w);             // Process list of "confirmed" contact attempts. (IE, unnecessary remote contacts are now gone)
    processSymptomaticQueue(w);              // Process queue of people who become symptomatic this timestep
    processRecoveryQueue(w);                 // Process queue of people who recover in this timestep

    w->infectionMod=(w->infectionMod + 1) % w->P->infectionWindow;  // Rotate timing windows.
    errline=101394;
    statsTimestep(w);                                               // Perform statistical aggregation for this timestep 
    if ((w->log_flat) && (w->mpi_rank==0)) logFlatfile(w);          // Write flat file output if requested. (Just rank 0)
    if ((w->log_db) && (w->mpi_rank==w->mpi_size-1)) logDB(w);      // Write to database if requested (Just the last node - hence, FF and DB will be simultaneous)
    errline=101398;
    for (int i=0; i<w->no_units; i++) {
      for (int j=0; j<w->a_units[i].no_interventions; j++) {   // For each unit, check whether any interventions have been 
        w->a_units[i].interventions[j].checkStatus(w);         // triggered on or off.
      }
    }
      
    resetUnitStats(w);                       // Reset counters for next timestep
    errline=101406;
    if (w->log_movie) updateImage(w);        // Update the image if requested.
    w->T+=(int)w->P->timestep_hours;         // Update timestep
    errline=101409;
  }
  if ((w->log_flat) && (w->mpi_rank==0)) fclose(w->ff); // Remember to flush/close flatfile output if it was opened.
}

void infectPerson(world* w, person* p) {
  if ((p->status & STATUS_SUSCEPTIBLE)>0) {
    p->status-=STATUS_SUSCEPTIBLE;
    p->status+=STATUS_CONTACTED;
    infectedPerson* ip = new infectedPerson(w,0,p);
    ip->t_contact=(float) w->T;      
    ip->t_inf = (float) (w->P->getInfectiousPeriodLength(0));
    w->contactQueue[0][0].push_back(ip);  // Add to the queue zero for first timestep.
  } 
}

void seedInfection(unsigned int count, world *w, int ls_x, int ls_y) {
  // For now, this seeds an infection of 'count' people, all in the same patch, to give the epidemic a reasonable start.
  
  if (w->localPatchLookup[ls_x/20][ls_y/20]>=0) {                               // NB - assuming 20x20 landscan cells per local patch here.
    localPatch* lp = w->localPatchList[w->localPatchLookup[ls_x/20][ls_y/20]];  // Get the local patch
    int size = (int) lp->no_people;                                             // How many people in patch?
    if (size>0) {                                                               // If it's not empty (which it ALWAYS will be unless debugging)
      unsigned int i=0;
      while (i<count) {                                                         // Seed 'count' infections
        int index = (int) (ranf_mt(0)*size);                                    // Get them all from rnd[0]
        infectPerson(w,&lp->people[index]);                                     // And infect each one.
        i++;
      }
    } 
  } 
}



void seedScheduledInfections(world* w) {
  if (w->P->next_seed<w->P->no_seeds) {
    while (w->P->seed_ts[w->P->next_seed]==(int)w->T) {
      seedInfection(w->P->seed_no[w->P->next_seed],w,lonToLsIndex(w->P->seed_long[w->P->next_seed]),latToLsIndex(w->P->seed_lat[w->P->next_seed]));
      w->P->next_seed++;
    }
  }
}

#ifdef MEMORY_CHECK
void PrintMemoryInfo( world* w, DWORD processID ) {
  HANDLE hProcess;
  PROCESS_MEMORY_COUNTERS pmc;
  int rank=-1;
  if (w!=NULL) rank=w->mpi_rank;
  printf( "%d: \nProcess ID: %u\n", rank, processID );
  hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
  if (NULL == hProcess) return;
  if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
    printf("%d: \tPageFaultCount: %I64d\n", rank,pmc.PageFaultCount );
    printf("%d: \tPEAK MEMORY CONSUMPTION: %I64d\n", rank,pmc.PeakWorkingSetSize);
    printf("%d: \tCURRENT MEMORY CONSUMPTION: %I64d\n",rank, pmc.WorkingSetSize);
    printf("%d: \tQuotaPeakPagedPoolUsage: %I64d\n", rank, pmc.QuotaPeakPagedPoolUsage);
    printf("%d: \tQuotaPagedPoolUsage: %I64d\n", rank,pmc.QuotaPagedPoolUsage);
    printf("%d: \tQuotaPeakNonPagedPoolUsage: %I64d\n", rank,pmc.QuotaPeakNonPagedPoolUsage);
    printf("%d: \tQuotaNonPagedPoolUsage: %I64d\n", rank,pmc.QuotaNonPagedPoolUsage);
    printf("%d: \tPagefileUsage: %I64d\n", rank,pmc.PagefileUsage); 
    printf("%d: \tPeakPagefileUsage: %I64d\n", rank,pmc.PeakPagefileUsage);
  }
  fflush(stdout);
  CloseHandle( hProcess );
}

#endif

int main(int argc, char* argv[])  {
  printf("Starting GSIM 1.0\n"); fflush(stdout);
  signal(SIGABRT, &handle_aborts);
  world *w = new world(argc,argv);
  initialiseMessages(w);

  #ifdef _USEMPI
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Errhandler_set(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
  #endif
  
  resetAllUnitStats(w);

  printf("Running at time %f\n",MPI_Wtime()); fflush(stdout);
  runSim(w);              // Go
  printf("Done at time %f\n",MPI_Wtime()); fflush(stdout);
  #ifdef MEMORY_CHECK
    PrintMemoryInfo( w, GetCurrentProcessId() );
  #endif

  #ifdef _USEMPI
    MPI_Finalize();
  #endif
  return 0;
}

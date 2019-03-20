/* patch.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Class and functions for local and remote patches
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

#include "patch.h"

localPatch::~localPatch() {
  delete [] q_prob;
  delete [] q_patch;
  delete [] households;
  delete [] people;
  
}

double patch::distance(patch *p1, int x, int y, int size) {
  patch* tempPatch = new patch();
  tempPatch->x=x;
  tempPatch->y=y;
  tempPatch->size=size;
  double d = patch::distance(p1,tempPatch);
  delete tempPatch;
  return d;
}


double patch::distance(patch *p1, patch *p2) {
  if (p2->x<p1->x) {
    patch *p3 = p1;
    p1=p2;
    p2=p3;
  }

  if (p1->x==p2->x) {
    if (p1->y==p2->y) return 0; // Same patch
    else if (p1->y==p2->y+p2->size) return 0; // p2 above p1, x equal
    else if (p1->y+p1->size==p2->y) return 0; // p2 below p1, x equal
  } else if (p1->x+p1->size==p2->x) {
    if (p1->y==p2->y) return 0; // p2 on right of p1, y equal
    else if (p1->y+p1->size==p2->y) return 0; // p2 on right, p1 above p2 touching
    else if (p1->y==p2->y+p2->size) return 0; // p2 on right, p2 above p1 touching
  } 
  
    // Cases...
    // 1. P2 is above P1, and within both x boundaries
    // 2. P2 is above P1, and just left side is within x boundaries
    // 3. P2 is below P1, and within both x boundaries
    // 4. P2 is below P1, and just left side is within x boundaries
    // 5. P2 is above, and completely to the right of P1.
    // 6. P2 is to the right, and "partly" above P1 (bottom of P2 overlaps y boundaries)
    // 7. P2 is to the right, and "partly" below P1 (top of P2 overlaps y boundaries)
    // 8. P2 is below, and completely to the right of P2.
    // 9. P2 is above and below P1 with larger y boundaries surrounding those of P1)
    // 10. P2 is to the right of P1, but P1's y boundaries surround those of P2)


  const unsigned int p1_x_end = p1->x+p1->size;
  const unsigned int p1_y_end = p1->y+p1->size;
  //const unsigned int p2_x_end = p2->x+p2->size;
  const unsigned int p2_y_end = p2->y+p2->size;

  if (isBetween(p2->x,p1->x,p1_x_end)) {                     // Filter for cases 1,2,3,4
    if (p2->y<p1->y) {                                       // Cases 1,2
      return landscan_y_distance(p2->x,p2_y_end,p1->y);
    } else {                                                 // Cases 3,4
      return landscan_y_distance(p2->x,p2->y,p1_y_end);
    }
  } else if (p2_y_end<p1->y) {                               // Case 5
    return landscan_diagonal_distance(p1_x_end,p1->y,p2->x,p2_y_end);
  } else if (p2->y>p1_y_end) {                               // Case 8
    return landscan_diagonal_distance(p1_x_end,p1_y_end,p2->x,p2->y);
  } else {                                                   // Cases 6,7,9,10
    return landscan_x_distance(p2->y,p1_x_end,p2->x);
  } 
}

// This is a binary chop for getting the contact patch out of the cumulative distribution.
// Note it is not 100% traditional - we want the entry AFTER the one that a traditional
// binary chop would pick, as we want to choose the patch with the greater probability.
// eg, probabilites 0.1, 0.2, 0.5, 0.8. If you pick 0.75, you want the 0.8 patch, not the 0.5.

patch* patch::getCommunityContactPatch(world* w, double r, localPatch* p) {
  int size = (int) p->no_qpatches;
  if (size==0) {
    return static_cast<patch*>(p);
  } else {

    unsigned int L=0;
    if (r<p->q_prob[0]) L=0;
    else if (r>p->q_prob[size-1]) L=size-1;
    else {
      unsigned int R=size+1;
      unsigned int P;
      double val;
      P=(R-L)/2;
      while (P!=0) {
        P=L+P;
        if (P>=p->no_qpatches) {
          P = (int) p->no_qpatches-1;
  //        printf("Correction 2\n");
        }
        val = p->q_prob[P];
        if (val<r) L=P;
        else R=P;
        P=(R-L)/2;
      }
      L++;
    }
    if (L>=p->no_qpatches) {
      L=(int)p->no_qpatches-1;
    }
  return w->allPatchList[p->q_patch[L]];
  }
}
    

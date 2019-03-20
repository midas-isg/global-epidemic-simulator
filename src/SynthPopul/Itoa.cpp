/* Itoa.cpp, part of the Global Epidemic Simulation v1.0 BETA
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



#include "Itoa.h"

// itoa is not standard. This is taken from 
// code.google.com/p/my-itoa.
// radix is kept as argument for compatibility
// but over-written. (It is 10 in all calls).

void my_itoa(int val, char* buf,int radix) {
  radix=10;  
  char* p;
  unsigned int a;        //every digit
  int len;
  char* b;            //start of the digit char
  char temp;
  unsigned int u;
  p = buf;
  if (val < 0) {
    *p++ = '-';
    val = 0 - val;
  }
  u = (unsigned int)val;
  b = p;
  do {
    a = u % radix;
    u /= radix;
    *p++ = a + '0';
  } while (u > 0);

  len = (int)(p - buf);

  *p-- = 0;

  //swap
  do {
    temp = *p;
    *p = *b;
    *b = temp;
    --p;
    ++b;
  } while (b < p);
}


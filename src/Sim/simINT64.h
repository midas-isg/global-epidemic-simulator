/* simINT64.h, part of the Global Epidemic Simulation v1.0 BETA
/* Definitions for cross-platform INT64 use
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

#ifndef SIM_I64_H
  #define SIM_I64_H

  #ifdef _WIN32
    #include "windows.h"
    #include "io.h"
    #define SIM_I64 __int64
  
  #else
    #include <stdint.h>
    #define SIM_I64 int64_t
  #endif


#endif
/* synthetic_population_file_format.h, part of the Global Epidemic Simulation v1.0 BETA
/* Format of the synthetic population output file
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



// file header:
class SynthPopulationFileHeader
{
	unsigned short AgeGroupNum;            // number of age groups in the model
	float AgeGroupLB[AgeGroupNum];         // array of lower age group boundaries

	unsigned int NumHouseholds;            // total number of households
	unsigned int numHosts;                 // total number of hosts
};


// the header is followed by an array of Household records of the following format:
class Household
{
	double Lat;                            // lattitude household coordinate
	double Lon;                            // longitude household coordinate
	unsigned short NumHosts;               // number of hosts in household
	unsigned int Dummy;                    // dummy field; set to -1; (0xFFFFFFFFh)
	Host[NumHosts] HostDescr;              // array of Host records
};

class Host
{
	unsigned char AgeGroupIndex;           // host's age group index
	float Age;                             // host's age
	unsigned short ID1;                    // establishment ID1 (ID of a file that contains a table of establishments)
	unsigned int ID2;                      // establishment ID2 (index in the table of establishments)
	unsigned short GroupID;                // ID of a group within an establishment a hosts belongs to (e.g. class in a school or work group in a firm); memebers of staff all have UNDEFINED (-1) group ID
};

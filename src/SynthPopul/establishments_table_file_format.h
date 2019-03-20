/* establishments_table_file_format.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Format of the establishment table output file
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



// File header:
class EstablishmentTableFileHeader
{
	unsigned short ID1;               // ID of the file
	unsigned int NumEstablishments;   // total number of establishments
};


// the header is followed by an array of Establishment records of the following format:
class Establishment
{
	double Lat;                       // lattitude establishment coordinate
	double Lon;                       // lattitude establishment coordinate
	unsigned int LndScnX;             // Landscan/GRUMP X coordinate of the cell containing the establishment
	unsigned int LndScnY;             // Landscan/GRUMP Y coordinate of the cell containing the establishment
	unsigned int NumHosts;            // number of hosts associated with the establishment
	unsigned int NumStaff;            // number of staff in the establishment (the total amount of individuals associated with the given establishment is then (NumStaff + NumHosts)!)
	unsigned int Dummy;               // dummy field; set to -1; (0xFFFFFFFFh)
};

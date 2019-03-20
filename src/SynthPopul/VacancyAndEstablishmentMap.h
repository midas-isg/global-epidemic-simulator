/* VacancyAndEstablishmentMap.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* vacancy map and establishment distribution classes used to synthesize establishments */

#ifndef VACANCY_AND_ESTABLISHMENT_MAP
#define VACANCY_AND_ESTABLISHMENT_MAP



/* map of vacancies in the country */
class Country::VacancyGeoMap
{
public:
	/* vacancy map cell */
	class VacancyGeoMapCell
	{
	public:
		unsigned int LndScnX;      // LandScan X index
		unsigned int LndScnY;      // LandScan Y index
		float Density;             // density of vacancies in a LandScan cell (in fact, probability value)
	};

	unsigned int NumCols;          // number of columns
	unsigned int NumRows;          // number of rows
	unsigned int NumRecs;          // number of records in the table; NumRecs = NumCols*NumRows for the _whole_ LandScan dataset
	unsigned int NumHosts;         // number of hosts in the country (actually it should be a number of working hosts or hosts of school age depending on the type of the places we consider)
	float XLLCorner;               // x-coordinate of the lower-left corner of the lower-left cell
	float YLLCorner;               // y-coordinate of the lower-left corner of the lower-left cell
	float CellSize;                // LandScan cell angular resolution (30 arcseconds) in degrees
	VacancyGeoMapCell *Table;      // pointer to the table
	
	/* constructor; builds a map of vacancy probabilities across the country based on the LandScan dataset and tralvel distance distribution */
	/* pCLSCTbl: pointer to LandScan table for the country */
	VacancyGeoMap(CntryLndScnCellTbl *pCLSCTbl, HostToEstTravelDistrKernel *pHETDKernel);

	~VacancyGeoMap();

	/* returns lattitude for LandScan cell with index Y */
	double LndScnYToLat(unsigned int y);

	/* returns longitude for LandScan cell with index X */
	double LndScnXToLon(unsigned int x);

	/* returns distance between LandScan cells (LndScnX1, LndScny1) and (LndScnX2, LndScny2) */
	double LndScnCellDist(unsigned int LndScnX1, unsigned int LndScnY1, unsigned int LndScnX2, unsigned int LndScnY2);

	/* host-to-establishment travel distance distribution kernel */
	//double F(double r);
};



/* vector of distribution of establishment locations */
class Country::ELDVec
{
public:
	/* element of vector */
	class ELDVEl
	{
	public:
		unsigned int Indx;      // index of record in LandScan table (CntryLndScnCellTblCl) for the country
		float CProb;            // corresponding cumulative probability value
	};
			
	/* used to output sampled location of a place */
	class Location
	{
	public:
		unsigned int LndScnX;   // LandScan X index
		unsigned int LndScnY;   // LandScan Y index
		double Lat;             // lattitude
		double Lon;             // longitude
	};
	
	unsigned int NumEls;        // number of elements in vector
	ELDVEl *V;                   // vector of LDVEl elements
	
	/* constructor; builds the cumulative distribution vector */
	ELDVec(Country::VacancyGeoMap *pVacancyGeoMap);

	~ELDVec();

	/* samples establishment location (lat and lon coordinates) */
	/* PVacancyMap: pointer to vacancy map object; pLoc is a pointer to a Location class variable through which location coordinates are returned */
	void SampleLocation(Country::VacancyGeoMap *pVacancyGeoMap, Country::ELDVec::Location *pLoc);
};



#endif

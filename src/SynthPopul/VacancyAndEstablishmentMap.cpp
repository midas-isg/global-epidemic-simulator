/* VacancyAndEstablishmentMap.cpp, part of the Global Epidemic Simulation v1.0 BETA
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




#include "LandScan.h"
#include "EstablishmentStruct.h"
#include "EstablishmentDistr.h"
#include "VacancyAndEstablishmentMap.h"
#ifdef _OPENMP
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#endif



/* constructor; builds a map of  vacancy probabilities across the country based on the LandScan dataset and tralvel distance distribution */
/* pCLSCTbl: pointer to LandScan table for the country */
Country::VacancyGeoMap::VacancyGeoMap(CntryLndScnCellTbl *pCLSCTbl, HostToEstTravelDistrKernel *pHETDKernel) : NumCols(pCLSCTbl->NumCols), NumRows(pCLSCTbl->NumRows), \
	NumRecs(pCLSCTbl->NumRecs), NumHosts(pCLSCTbl->NumHosts), XLLCorner(pCLSCTbl->XLLCorner), YLLCorner(pCLSCTbl->YLLCorner), CellSize(pCLSCTbl->CellSize)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::VacancyGeoMap::VacancyGeoMap(CntryLndScnCellTbl*, HostToEstTravelDistrKernel*) started...\n";
#endif

	try
	{
		Table = new VacancyGeoMapCell[NumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of Country::VacancyGeoMap::VacancyGeoMapCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	
	#pragma omp parallel for
	for(unsigned int i=0; i < NumRecs; i++)
	{
		Table[i].LndScnX = pCLSCTbl->Table[i].LndScnX;
		Table[i].LndScnY = pCLSCTbl->Table[i].LndScnY;
		
		Table[i].Density = (float)pCLSCTbl->Table[i].NumHosts/(float)pCLSCTbl->NumHosts;
	}
	
	/*
	PointerToKernFunc F = pHETDKernel->F();

	float normConst = 0.0f;                   // normalisation constant
	#pragma omp parallel for reduction(+:normConst)
	for(unsigned int i=0; i < NumRecs; i++)
	{
		Table[i].LndScnX = pCLSCTbl->Table[i].LndScnX;
		Table[i].LndScnY = pCLSCTbl->Table[i].LndScnY;

		Table[i].Density = 0.0f;

		for(unsigned int j=0; j < NumRecs; j++)
		{
			double r = LndScnCellDist(Table[i].LndScnX, Table[i].LndScnY, pCLSCTbl->Table[j].LndScnX, pCLSCTbl->Table[j].LndScnY);
			Table[i].Density += (double)pCLSCTbl->Table[j].NumHosts * (pHETDKernel->*F)(r);
		}

		normConst += Table[i].Density;
	}

	#pragma omp parallel for
	for(unsigned int i=0; i < NumRecs; i++)
		Table[i].Density /= normConst;
	*/

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::VacancyGeoMap::VacancyGeoMap(CntryLndScnCellTbl*, HostToEstTravelDistrKernel*) completed.\n";
#endif
}



Country::VacancyGeoMap::~VacancyGeoMap()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::VacancyGeoMap::~VacancyGeoMap() started...\n";
#endif

	if( Table != NULL )
	{
		delete [] Table;
		Table = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::VacancyGeoMap::~VacancyGeoMap()completed.\n";
#endif
}



/* returns lattitude for LandScan cell with index Y */
double Country::VacancyGeoMap::LndScnYToLat(unsigned int y)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnYToLat(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnYToLat(unsigned int) completed.\n\n";
#endif

	return (YLLCorner + NumRows * CellSize - CellSize * y);
}



/* returns longitude for LandScan cell with index X */
double Country::VacancyGeoMap::LndScnXToLon(unsigned int x)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnXToLon(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnXToLon(unsigned int) completed.\n\n";
#endif

	return (XLLCorner + CellSize * x);
}



/* returns distance between LandScan cells (LndScnX1, LndScny1) and (LndScnX2, LndScny2) */
double Country::VacancyGeoMap::LndScnCellDist(unsigned int LndScnX1, unsigned int LndScnY1, unsigned int LndScnX2, unsigned int LndScnY2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnCellDist(unsigned int, unsigned int, unsigned int, unsigned int) started...\n";
#endif

	if( (LndScnX1 == LndScnX2) && (LndScnY1 == LndScnY2) )
		return 0.0;

	double latRad1 = PI/180.0 * LndScnYToLat(LndScnY1);
	double latRad2 = PI/180.0 * LndScnYToLat(LndScnY2);
	double latSepRad = latRad1 - latRad2;
	double lonSepRad = PI/180.0 * (LndScnXToLon(LndScnX2) - LndScnXToLon(LndScnX1));

	double var = sin(latSepRad * 0.5) * sin(latSepRad * 0.5) + cos(latRad1) * cos(latRad2) * sin(lonSepRad * 0.5) * sin(lonSepRad * 0.5);

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::VacancyGeoMap::LndScnCellDist(unsigned int, unsigned int, unsigned int, unsigned int) completed.\n\n";
#endif

	return 2.0 * GEO_MRADIUS * asin(min(1.0, sqrt(var)));
}



/* constructor; builds the cumulative distribution vector */
Country::ELDVec::ELDVec(Country::VacancyGeoMap *pVacancyGeoMap) : NumEls(pVacancyGeoMap->NumRecs+1)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ELDVec::ELDVec(Country::VacancyGeoMap*) started...\n";
#endif

	try
	{
		V = new ELDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of cumulative probability values (CRITICAL) in Country::ELDVec::ELDVec(VacancyGeoMap*). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	
	V[0].Indx = (unsigned int)UNDEFINED;
	V[0].CProb = 0.0f;

	for(unsigned int i=0; i < pVacancyGeoMap->NumRecs; i++)
	{
		V[i+1].Indx = i;
		V[i+1].CProb = pVacancyGeoMap->Table[i].Density + V[i].CProb;
	}

	for(unsigned int i=1; i < NumEls - 1; i++)
		V[i].CProb /= V[NumEls-1].CProb;

	V[NumEls-1].CProb = 1.0f + TOLER_VAL;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ELDVec::ELDVec(Country::VacancyGeoMap*) completed\n";
#endif
}



Country::ELDVec::~ELDVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ELDVec::~ELDVec() started...\n";
#endif

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ELDVec::~ELDVec() completed.\n";
#endif
}



/* samples establishment location (lat and lon coordinates) */
/* PVacancyMap: pointer to vacancy map object; pLoc is a pointer to a Location class variable through which location coordinates are returned */
void Country::ELDVec::SampleLocation(Country::VacancyGeoMap *PVacancyGeoMap, Country::ELDVec::Location *pLoc)
{
#ifdef OUTPUT_LOG_L2
	cout << "***Country::ELDVec::SampleLocation(Country::VacancyGeoMap*, Country::ELDVec::Location*) started...\n";
#endif

	double rnd = ranf_();
	unsigned int lIndx = 0;
	unsigned int rIndx = NumEls - 1;
	unsigned int L;

	while( (L = rIndx - lIndx) > 1 )
	{
		if( rnd > V[lIndx + L/2].CProb )
			lIndx += L/2;
		else
			rIndx -= L/2;
	}

	unsigned int indx = V[rIndx].Indx;

	pLoc->LndScnX = PVacancyGeoMap->Table[indx].LndScnX;
	pLoc->LndScnY = PVacancyGeoMap->Table[indx].LndScnY;
	pLoc->Lat = PVacancyGeoMap->LndScnYToLat(pLoc->LndScnY) - PVacancyGeoMap->CellSize * ranf_();
	pLoc->Lon = PVacancyGeoMap->LndScnXToLon(pLoc->LndScnX) + PVacancyGeoMap->CellSize * ranf_();
#ifdef OUTPUT_LOG_L2
	cout << "***Country::ELDVec::SampleLocation(Country::VacancyGeoMap*, Country::ELDVec::Location*) completed\n\n";
#endif
}

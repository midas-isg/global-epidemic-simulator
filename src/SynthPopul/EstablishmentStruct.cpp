/* EstablishmentStruct.cpp, part of the Global Epidemic Simulation v1.0 BETA
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



#include "Includes_n_Definitions.h"
#include "LandScan.h"
#include "HouseholdStruct.h"
#include "EstablishmentDistr.h"
#include "EstablishmentStruct.h"
#ifdef _OPENMP
#include "omp.h"
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#include "Stubs.h"
#endif
#ifndef _WIN32
#include "Itoa.h"
#endif



#define DEFAULT_TBL_SIZE 16
#define DEFAULT_VEC_LNGTH 16
#define DEFAULT_EXT_COEFF 0.5



/* fills in vacancy; returns the number of remaining host vacancies */
unsigned int Country::Establishment::FillHostVacancy()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Establishment::FillHostVacancy() started...\n";
#endif

	if( NumHosts == NumHostVacancs )
		throw TABLE_TOO_SMALL;        // no vacancies, establishment is full - throw an exception here

	NumHosts++;
	
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Establishment::FillHostVacancy() completed.\n\n";
#endif

	return (NumHostVacancs - NumHosts);
}



/* fills in staff vacancy; returns the number of remaining staff vacancies  */
unsigned int Country::Establishment::FillStaffVacancy()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Establishment::FillStaffVacancy() started...\n";
#endif

	if( NumStaff == NumStaffVacancs )
		throw TABLE_TOO_SMALL;        // no vacancies, establishment is full - throw an exception here

	NumStaff++;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Establishment::FillStaffVacancy() completed.\n\n";
#endif

	return (NumStaffVacancs - NumStaff);
}



/* default constructor; creates undefined vector */
Country::MdESDVec::MdESDVec() : NumEls(0), V(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::MdESDVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::MdESDVec() completed.\n\n";
#endif
}



/* constructor; creates empty vector based on input establishment size distribution vector */
/* ESDV: input establishment size distribution vector */
Country::MdESDVec::MdESDVec(ESDVec *pESDV) : NumEls(pESDV->NumEls)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::MdESDVec(ESDVec*) started...\n";
#endif

	try
	{
		 V = new MdESDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of HETDVec::HETDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned int i=0; i < NumEls; i++)
	{
		V[i].Size = pESDV->V[i].Size;
		V[i].NumEst = 0;
		V[i].Prob = 0.0f;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::MdESDVec(ESDVec*) completed.\n\n";
#endif
}



Country::MdESDVec::~MdESDVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::~MdESDVec() started...\n";
#endif

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::MdESDVec::~MdESDVec() completed.\n\n";
#endif
}



/* default constructor; creates undefined table */
Country::EstablishmentTbl::EstablishmentTbl() : ExtCoeff(DEFAULT_EXT_COEFF), NumRecs(0), NumRecsMax(0), Table(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::EstablishmentTbl() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::EstablishmentTbl() completed.\n";
#endif
}



/* constructor; creates empty table of Establishment objects */
/* numEst: total number of establishments; extCoeff: extension coefficient for Table */
Country::EstablishmentTbl::EstablishmentTbl(unsigned int TblSize, float extCoeff) : ExtCoeff(extCoeff), NumRecs(0), NumRecsMax(TblSize)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::EstablishmentTbl(unsigned int, float) started...\n";
#endif

	try
	{
		Table = new Establishment[NumRecsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating table (Table) of Establishment objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::EstablishmentTbl(unsigned int, float) completed\n";
#endif
}



Country::EstablishmentTbl::~EstablishmentTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::~EstablishmentTbl() started...\n";
#endif

	if( Table != NULL )
	{
		delete [] Table;
		Table = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::~EstablishmentTbl() completed.\n";
#endif
}



/* adds record in establishment table */
/* pEst: pointer to the record to be added */
void Country::EstablishmentTbl::AddRec(Establishment *pEst)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AddRec(Establishment*) started...\n";
#endif

	if( NumRecs == NumRecsMax )      // no space left in Table, extend it
	{
		if( NumRecsMax != 0 )
			NumRecsMax *= (1 + ExtCoeff);
		else
			NumRecsMax = DEFAULT_TBL_SIZE;

		Establishment *TableTemp;
		try
		{
			TableTemp = new Establishment[NumRecsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (TableTemp) of Country::Establishment objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

		#pragma omp parallel for
		for(unsigned int i=0; i < NumRecs; i++)
			TableTemp[i] = Table[i];

		if( Table != NULL )
			delete [] Table;
		Table = TableTemp;
	}

	Table[NumRecs++] = *pEst;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AddRec(Establishment*) completed.\n\n";
#endif
}



/* appends a table to an existing one */
void Country::EstablishmentTbl::AppendTable(EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AppendTable(EstablishmentTbl*) started...\n";
#endif

	unsigned int i, j;
	unsigned int numRecsNew = pEstTbl->NumRecs + NumRecs;
	if( numRecsNew > NumRecsMax )
	{
		Establishment *TableTemp;
		try
		{
			TableTemp = new Establishment[numRecsNew];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (TableTemp) of Country::Establishment objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(i=0; i < NumRecs; i++)
			TableTemp[i] = Table[i];
		for(j=0; i < numRecsNew; i++, j++)
			TableTemp[i] = pEstTbl->Table[j];
		if( Table != NULL )
			delete [] Table;
		Table = TableTemp;
		NumRecsMax = numRecsNew;
	}
	else
	{
		for(i = NumRecs, j=0; i < numRecsNew; i++, j++)
			Table[i] = pEstTbl->Table[j];
	}
	NumRecs = numRecsNew;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AppendTable(EstablishmentTbl*) completed.\n\n";
#endif
}



/* gets the first empty record index in Table */
unsigned int Country::EstablishmentTbl::GetFirstEmptyRecIndex()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetFirstEmptyRecIndex() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetFirstEmptyRecIndex() completed.\n\n";
#endif

	return NumRecs;
}



/* resets table */
void Country::EstablishmentTbl::ResetTable()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::ResetTable() started...\n";
#endif

	NumRecs = 0;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::ResetTable() completed.\n\n";
#endif
}



/* gets Establishment object out of Table */
/* estObjIndx: index of Establishment object (record); pEst: pointer to location in memory through which this object is returned */
void Country::EstablishmentTbl::GetEstablishmentRec(unsigned int estObjIndx, Country::Establishment *pEst)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishment(unsigned int, Country::Establishment*) started...\n";
#endif

	*pEst = Table[estObjIndx];

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishment(unsigned int, Country::Establishment*) completed.\n\n";
#endif
}



/* get establishment coordinates */
/* estIndx: establishment index; *pLat and *pLon: pointers to variables through which Lat and Lon are returned */
void Country::EstablishmentTbl::GetEstablishmentCoords(unsigned int estObjIndx, double *pLat, double *pLon)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishmentCoords(unsigned int, double*, double*) started...\n";
#endif

	*pLat = Table[estObjIndx].Lat;
	*pLon = Table[estObjIndx].Lon;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishmentCoords(unsigned int, double*, double*) completed.\n\n";
#endif
}



/* gets X and Y indexes of the LandScan cell that contains the establishment */
/* estObjIndx: establishment index; *pLndScnX and *pLndScnY: pointers to the variables through which the indexes are returned */
void Country::EstablishmentTbl::GetEstablishmentLndScnIndxs(unsigned int estObjIndx, unsigned int *pLndScnX, unsigned int *pLndScnY)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishmentLndScnIndxs(unsigned int, unsigned int*, unsigned int*) started...\n";
#endif

	*pLndScnX = Table[estObjIndx].LndScnX;
	*pLndScnY = Table[estObjIndx].LndScnY;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetEstablishmentLndScnIndxs(unsigned int, unsigned int*, unsigned int*) completed.\n\n";
#endif
}



/* assigns host to establishment; returns the number of remaining host vacancies */
/* estObjIndx: establishment index (establishment ID) */
unsigned int Country::EstablishmentTbl::AssignHost(unsigned int estObjIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AssignHost(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AssignHost(unsigned int) completed.\n\n";
#endif
	return Table[estObjIndx].FillHostVacancy();
}



/* assigns staff member to establishment; returns the number of remaining staff vacancies */
/* estObjIndx: establishment index (establishment ID) */
unsigned int Country::EstablishmentTbl::AssignStaff(unsigned int estObjIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AssignStaff(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::AssignStaff(unsigned int) completed.\n\n";
#endif
	return Table[estObjIndx].FillStaffVacancy();
}



/* gets the total number of establishments (number of records in Table) */
unsigned int Country::EstablishmentTbl::GetNumEstablishments()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetNumEstablishments() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetNumEstablishments() completed.\n\n";
#endif

	return NumRecs;
}



/* gets random unoccupied establishment */
unsigned int Country::EstablishmentTbl::GetRndUnoccpdEstablishment()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetRndUnoccpdEstablishment() started...\n";
#endif

	unsigned int indx = NumRecs * ranf_();
	for(unsigned int i = indx; i < NumRecs; i++)
		if( Table[i].NumHosts == 0 )
			return i;
	for(int i = indx - 1; i >=0; i--)
		if( Table[i].NumHosts == 0 )
			return i;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::GetRndUnoccpdEstablishment() completed.\n\n";
#endif

	return (unsigned int)UNDEFINED;
}



/* modifies record in Table */
/* estObjIndx: establishment index; pEst: pointer to new Establishment object that replaces one with index estIndx */
void Country::EstablishmentTbl::ModifyRec(unsigned int estObjIndx, Country::Establishment *pEst)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::ModifyRec(unsigned int, Country::Establishment*) started...\n";
#endif

	Table[estObjIndx] = *pEst;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::EstablishmentTbl::ModifyRec(unsigned int, Country::Establishment*) completed.\n\n";
#endif
}



/* computes model establishment size distribution */
Country::MdESDVec* Country::EstablishmentTbl::CompModelEstSizeDistr(ESDVec *pESDV)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::CompModelEstSizeDistr(ESDVec*) started...\n";
#endif

	MdESDVec* pMdESDVec = new MdESDVec(pESDV);
	
	unsigned int totNumEst = 0;   // total number of establishments

	for(unsigned int i=0; i < NumRecs; i++)
	{
		unsigned int lIndx = 0;
		unsigned int rIndx = pMdESDVec->NumEls - 1;
		unsigned int L;
		while( (L = rIndx - lIndx) > 1 )
		{
			if( Table[i].NumHosts > pMdESDVec->V[lIndx + L/2].Size )
				lIndx += L/2;
			else
				rIndx -= L/2;
		}

		pMdESDVec->V[rIndx].NumEst++;
		totNumEst++;
	}

	for(unsigned int i=0; i < pMdESDVec->NumEls; i++)
		pMdESDVec->V[i].Prob = (float)pMdESDVec->V[i].NumEst/(float)totNumEst;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::EstablishmentTbl::CompModelEstSizeDistr(ESDVec*) completed.\n\n";
#endif

	return pMdESDVec;
}



/* default constructor; creates undefined vector */
Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec() : ExtCoeff(DEFAULT_EXT_COEFF), NumEls(0), NumElsMax(0), V(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec() completed.\n\n";
#endif
}



/* constructor; creates empty vector with initial capacity estIndxIniVecLngth and extension coefficient extCoeff */
Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(unsigned int estIndxIniVecLngth, float extCoeff) : \
	ExtCoeff(extCoeff), NumEls(0), NumElsMax(estIndxIniVecLngth)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(unsigned int, float) started...\n";
#endif

	if( NumElsMax != 0 )
	{
		try
		{
			V = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(unsigned int, float). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(unsigned int, float) completed.\n\n";
#endif
}



/* copy constructor */
Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(const EstablishmentIndxVec& PIV) : \
	ExtCoeff(PIV.ExtCoeff), NumEls(PIV.NumEls), NumElsMax(PIV.NumElsMax)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(const EstablishmentIndxVec&) started...\n";
#endif

	if( PIV.V != NULL)
	{
		try
		{
			V = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of Establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(const EstablishmentIndxVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = PIV.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::EstablishmentIndxVec(const EstablishmentIndxVec&) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::~EstablishmentIndxVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::~EstablishmentIndxVec() started...\n";
#endif

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::~EstablishmentIndxVec() completed.\n\n";
#endif
}



/* assignment operator */
Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::operator=(const EstablishmentIndxVec& PIV2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::operator=(const EstablishmentIndxVec&) started...\n";
#endif

	if( this == &PIV2)
		return *this;

	ExtCoeff = PIV2.ExtCoeff;
	NumEls = PIV2.NumEls;
	NumElsMax = PIV2.NumElsMax;

	if( V != NULL )
		delete [] V;

	if( PIV2.V != NULL)
	{
		try
		{
			V = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::operator=(const EstablishmentIndxVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = PIV2.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::operator=(const EstablishmentIndxVec&) completed.\n\n";
#endif

	return *this;
}



/* adds the establishment index (in the table of establishment objects) into vector V */
/* returns the index (in V) of the added element */
/* estObjIndx: index in the table of establishment objects */
unsigned int Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::AddEstablishmentIndx(unsigned int estObjIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::AddEstablishmentIndx(unsigned int) started...\n";
#endif

	if( NumEls == NumElsMax )      // no space left in V, extend it
	{
		if( NumElsMax != 0 )
			NumElsMax *= (1 + ExtCoeff);
		else
			NumElsMax = DEFAULT_VEC_LNGTH;

		unsigned int *tempV;
		try
		{
			tempV = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::AddEstablishmentIndx(unsigned int). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			tempV[i] = V[i];
		if( V != NULL)
			delete [] V;
		V = tempV;
	}

	unsigned int indx = NumEls++;
	V[indx] = estObjIndx;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::AddEstablishmentIndx(unsigned int) completed.\n\n";
#endif

	return indx;
}



/* gets establishment object index (can be the last one in V or a random one depending on implementation) */
void Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::GetEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx *pIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::GetEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx*) started...\n";
#endif

	unsigned int estIndx = NumEls - 1;
	if( estIndx != -1 )
	{
		pIndx->ElIndx = estIndx;
		pIndx->EstObjIndx = V[estIndx];
	}
	else
	{
		pIndx->ElIndx = (unsigned int)UNDEFINED;
		pIndx->EstObjIndx = (unsigned int)UNDEFINED;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::GetEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx*) completed.\n\n";
#endif
}



/* removes the establishment index from vector V returning the number of remaining indexes */
/* pIndx: describes index of the element in V to be removed */
unsigned int Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::RemoveEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx *pIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::RemoveEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx*) started...\n";
#endif

	unsigned int indx = pIndx->ElIndx;
	if( (indx < NumEls) && (pIndx->EstObjIndx == V[indx]) )
	{
		if( NumEls != 0 && indx < --NumEls )
			V[indx] = V[NumEls];               // copy the last element of V in place of the element to be removed
	}
	
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::RemoveEstablishmentIndx(Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Indx*) completed.\n\n";
#endif

	return NumEls;
}



/* resets vector */
void Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Reset()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Reset() started...\n";
#endif

	NumEls = 0;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::EstablishmentIndxVec::Reset() completed.\n\n";
#endif
}



/* default constructor; creates undefined vector */
Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec() : NumEls(0), NumElsMax(0), V(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec() completed\n\n";
#endif
}



/* constructor; creates empty vector of length PDVecLngth */
Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(unsigned int PDVecLngth) : NumEls(0), NumElsMax(PDVecLngth)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(unsigned int) started...\n";
#endif

	try
	{
		V = new PDVEl[NumElsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of probability distribution values (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(unsigned int). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	V[0].GeoPtchIndx = (unsigned int)UNDEFINED;
	V[0].CProb = 0.0f;
	NumEls++;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(unsigned int) completed.\n\n";
#endif
}



/* copy constructor */
Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(const PDVec& PDV) : NumEls(PDV.NumEls), NumElsMax(PDV.NumElsMax)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(const PDVec&) started...\n";
#endif

	if( PDV.V != NULL )
	{
		try
		{
			V = new PDVEl[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of probability distribution values (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(const PDVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = PDV.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::PDVec(const PDVec&) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatch::PDVec::~PDVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::~PDVec() started...\n";
#endif

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::~PDVec() completed.\n\n";
#endif
}



/* assignment operator */
Country::HostToEstablishmentMap::GeoPatch::PDVec Country::HostToEstablishmentMap::GeoPatch::PDVec::operator=(const PDVec& PDV2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::operator=(const PDVec&) started...\n";
#endif

	if( this == &PDV2)
		return *this;

	NumEls = PDV2.NumEls;
	NumElsMax = PDV2.NumElsMax;

	if( V != NULL )
		delete [] V;

	if( PDV2.V != NULL )
	{
		try
		{
			V = new PDVEl[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of probability distribution values (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::PDVec::operator=(const PDVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = PDV2.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::operator=(const PDVec&) completed.\n\n";
#endif

	 return *this;
}



/* adds probability value to the cumulative probability distribution vector */
void Country::HostToEstablishmentMap::GeoPatch::PDVec::AddProbVal(unsigned int geoPtchIndx, double probVal)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::AddProbVal(unsigned int, double) started...\n";
#endif

	if( NumEls == NumElsMax )
	{
		cerr << "*** Error attempiting to add an element into a vector of of probability distribution values in Country::HostToEstablishmentMap::GeoPatch::PDVec::AddProbVal(). Operation ignored as the vector is full. ***\n";
		return;
	}

	if( probVal > 0.0f )
	{
		unsigned int indx = NumEls++;
		V[indx].GeoPtchIndx = geoPtchIndx;
		V[indx].CProb = V[indx-1].CProb + probVal;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::AddProbVal(unsigned int, double) completed.\n\n";
#endif
}



/* normalizes cumulative probability vector */
void Country::HostToEstablishmentMap::GeoPatch::PDVec::Normalize()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::Normalize() started...\n";
#endif

	if( NumEls > 1 )
	{
		for(unsigned int i=1; i < NumEls-1 ; i++)
			V[i].CProb /= V[NumEls-1].CProb;
		V[NumEls-1].CProb = 1.0 + TOLER_VAL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::PDVec::Normalize() completed.\n\n";
#endif
}



/* constructor; creates empty vector */
Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec() : NumEls(0), NumElsMax(HOST_TO_EST_MAP_GEOPATCH_GRANURLTY * HOST_TO_EST_MAP_GEOPATCH_GRANURLTY)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(unsigned int) started...\n";
#endif

	try
	{
		V = new unsigned int[NumElsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of LandScan cell indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(unsigned int). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(unsigned int) completed.\n\n";
#endif
}



/* copy constructor */
Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(const LndScnCellVec& LSCVec) : NumEls(LSCVec.NumEls), NumElsMax(LSCVec.NumElsMax)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(const LndScnCellVec&) started...\n";
#endif

	if( LSCVec.V != NULL )
	{
		try
		{
			V = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of LandScan cell indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(const LndScnCellVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = LSCVec.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::LndScnCellVec(const LndScnCellVec&) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::~LndScnCellVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::~LndScnCellVec() started...\n";
#endif

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::~LndScnCellVec() completed.\n\n";
#endif
}



/* assignment operator */
Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::operator= (const LndScnCellVec& LSCVec2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::operator= (const LndScnCellVec&) started...\n";
#endif

	if( this == &LSCVec2 )
		return *this;

	NumEls = LSCVec2.NumEls;
	NumElsMax = LSCVec2.NumElsMax;

	if( V != NULL )
		delete [] V;

	if( LSCVec2.V != NULL )
	{
		try
		{
			V = new unsigned int[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of LandScan cell indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::operator= (const LndScnCellVec&). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			V[i] = LSCVec2.V[i];
	}
	else
		V = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::operator= (const LndScnCellVec&) completed.\n\n";
#endif

	return *this;
}



/* adds LandScan cell index into vector */
void Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::AddLandScanCellIndx(unsigned int LndScnCellIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::AddLandScanCellIndx(unsigned int) started...\n";
#endif

	if( NumEls == NumElsMax )      // no space left in fixed size vector
		throw INDX_OUT_OF_RANGE;

	unsigned int indx = NumEls++;
	V[indx] = LndScnCellIndx;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::LndScnCellVec::AddLandScanCellIndx(unsigned int) completed.\n\n";
#endif
}



/* default constructor; creates undefined GeoPatch object */
Country::HostToEstablishmentMap::GeoPatch::GeoPatch() : GPLndScnX((unsigned int)UNDEFINED), GPLndScnY((unsigned int)UNDEFINED), PEstIndxV(NULL), PPDV(NULL), PLSCVec(NULL), NumRepHits(0), NumHostVacancs(0), NumStaffVacancs(0)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch() completed.\n\n";
#endif
}



/* constructor; builds a GeoPatch object */
/* gpLndScnX and gpLndScnY: X and Y patch coordinates represented as LandScan indexes of the upper left corner cell */
/* estIndxVecIniLngth: initial length of vector of establishment indexes EstablishmentIndxVec; estIndxVecExtCoeff: extension coefficient of that vector */
Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int gpLndScnX, unsigned int gpLndScnY, unsigned int estIndxVecIniLngth, float estIndxVecExtCoeff) : \
	GPLndScnX(gpLndScnX), GPLndScnY(gpLndScnY), PEstIndxV(new EstablishmentIndxVec(estIndxVecIniLngth, estIndxVecExtCoeff)), PPDV(NULL), PLSCVec(new LndScnCellVec),\
	NumRepHits(0), NumHostVacancs(0), NumStaffVacancs(0)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int, unsigned int, unsigned int, float) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int, unsigned int, unsigned int, float) completed.\n\n";
#endif
}



/* constructor; builds a GeoPatch object */
/* gpLndScnX and gpLndScnY: X and Y patch coordinates represented as LandScan indexes of the upper left corner cell */
Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int gpLndScnX, unsigned int gpLndScnY) : \
	GPLndScnX(gpLndScnX), GPLndScnY(gpLndScnY), PEstIndxV(new EstablishmentIndxVec), PPDV(NULL), PLSCVec(new LndScnCellVec),\
	 NumRepHits(0), NumHostVacancs(0), NumStaffVacancs(0)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int, unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(unsigned int, unsigned int) completed.\n\n";
#endif
}



/* copy constructor */
Country::HostToEstablishmentMap::GeoPatch::GeoPatch(const GeoPatch& GP) : GPLndScnX(GP.GPLndScnX), GPLndScnY(GP.GPLndScnY),\
	NumRepHits(GP.NumRepHits), NumHostVacancs(GP.NumHostVacancs), NumStaffVacancs(GP.NumStaffVacancs)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(const GeoPatch&) started...\n";
#endif

	if( GP.PEstIndxV != NULL )
		PEstIndxV = new EstablishmentIndxVec(*GP.PEstIndxV);
	else
		PEstIndxV = NULL;

	if( GP.PPDV != NULL )
		PPDV = new PDVec(*GP.PPDV);
	else
		PPDV = NULL;

	if( GP.PLSCVec != NULL )
		PLSCVec = new LndScnCellVec(*GP.PLSCVec);
	else
		PLSCVec = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::GeoPatch(const GeoPatch&) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatch::~GeoPatch()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::~GeoPatch() started...\n";
#endif

	if( PEstIndxV != NULL )
	{
		delete PEstIndxV;
		PEstIndxV = NULL;
	}
	
	if( PPDV != NULL )
	{
		delete PPDV;
		PPDV = NULL;
	}

	if( PLSCVec != NULL )
	{
		delete PLSCVec;
		PLSCVec = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::~GeoPatch() completed.\n\n";
#endif
}



/* assignment operator */
Country::HostToEstablishmentMap::GeoPatch Country::HostToEstablishmentMap::GeoPatch::operator=(const GeoPatch& GP2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::operator=(const GeoPatch&) started...\n";
#endif

	if( this == &GP2 )
		return *this;

	GPLndScnX = GP2.GPLndScnX;
	GPLndScnY = GP2.GPLndScnY;
	NumRepHits = GP2.NumRepHits;
	NumHostVacancs = GP2.NumHostVacancs;
	NumStaffVacancs = GP2.NumStaffVacancs;
	
	if( PEstIndxV != NULL )
		delete PEstIndxV;

	if( GP2.PEstIndxV != NULL )
		PEstIndxV = new EstablishmentIndxVec(*GP2.PEstIndxV);
	else
		PEstIndxV = NULL;


	if( PPDV != NULL )
		delete PPDV;
	
	if( GP2.PPDV != NULL )
		PPDV = new PDVec(*GP2.PPDV);
	else
		PPDV = NULL;


	if( PLSCVec != NULL )
		delete PLSCVec;

	if( GP2.PLSCVec != NULL )
		PLSCVec = new LndScnCellVec(*GP2.PLSCVec);
	else
		PLSCVec = NULL;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::operator=(const GeoPatch&) completed.\n\n";
#endif

	return *this;
}



/* default constructor; creates undefined vector */
Country::HostToEstablishmentMap::GeoPatchVec::GeoPatchVec() : ExtCoeff(DEFAULT_EXT_COEFF), NumEls(0), NumEstPtchs(0), NumAvailEstPtchs(0), NumElsMax(0), V(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::operator=(const GeoPatch&) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatch::operator=(const GeoPatch&) started...\n";
#endif
}



/* constructor; creates empty vector with initial capacity gpPatchVecIniLngth and extension coefficient extCoeff */
Country::HostToEstablishmentMap::GeoPatchVec::GeoPatchVec(unsigned int gpPatchVecIniLngth, float extCoeff) : ExtCoeff(extCoeff), NumEls(0), NumEstPtchs(0), NumAvailEstPtchs(0), NumElsMax(gpPatchVecIniLngth)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::GeoPatchVec(unsigned int, float) started...\n";
#endif

	try
	{
		V = new GeoPatch[NumElsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatchVec::GeoPatchVec(unsigned int, float). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::GeoPatchVec(unsigned int, float) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatchVec::~GeoPatchVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::~GeoPatchVec() started...\n";
#endif

	if( V != NULL)
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::~GeoPatchVec() completed.\n\n";
#endif
}



/* adds GeoPatch object to V */
/* pGPtch: pointer to an object to be added */
/* returns index of the added geographical patch in V */
unsigned int Country::HostToEstablishmentMap::GeoPatchVec::AddGeoPatch(GeoPatch *pGPtch)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::AddGeoPatch(GeoPatch*) started...\n";
#endif

	if( NumEls == NumElsMax )     // no space left in V, extend it
	{
		if( NumElsMax != 0 )
			NumElsMax *= (1 + ExtCoeff);
		else
			NumElsMax = DEFAULT_VEC_LNGTH;

		GeoPatch *tempV;
		try
		{
			tempV = new GeoPatch[NumElsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of establishment indexes (CRITICAL) in Country::HostToEstablishmentMap::GeoPatchVec::AddGeoPatch(GeoPatch*). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned int i=0; i < NumEls; i++)
			tempV[i] = V[i];
		if( V != NULL)
			delete [] V;
		V = tempV;
	}

	unsigned int indx = NumEls++;
	V[indx] = *pGPtch;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::AddGeoPatch(GeoPatch*) completed.\n\n";
#endif

	return indx;
}



/* returns pointer to geographical patch with index geoPatchIndx in vector V */
Country::HostToEstablishmentMap::GeoPatch* Country::HostToEstablishmentMap::GeoPatchVec::GetGeoPatch(unsigned int geoPatchIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::GetGeoPatch(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::GetGeoPatch(unsigned int) completed.\n\n";
#endif
	if( geoPatchIndx >= NumEls )
		throw INDX_OUT_OF_RANGE;

	return &V[geoPatchIndx];
}


/* increments the counter of geographical patches with establishments */
void Country::HostToEstablishmentMap::GeoPatchVec::IncrEstPtchsCntr()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::IncrEstPtchsCntr() started...\n";
#endif

	NumEstPtchs++;
	NumAvailEstPtchs++;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::IncrEstPtchsCntr() completed.\n\n";
#endif
}



/* increments the counter of geographical patches with available establishments */
/* returns new value of counter */
unsigned int Country::HostToEstablishmentMap::GeoPatchVec::IncrAvailEstPtchsCntr()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::IncrAvailEstPtchsCntr() started...\n";
#endif

	return ++NumAvailEstPtchs;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::IncrAvailEstPtchsCntr() completed.\n\n";
#endif
}



/* decrements the counter of available geographical patches with establishments */
/* returns new value of counter */
unsigned int Country::HostToEstablishmentMap::GeoPatchVec::DecrAvailEstPtchsCntr()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::DecrAvailEstPtchsCntr() started...\n";
#endif

	return --NumAvailEstPtchs;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::DecrAvailEstPtchsCntr() completed.\n\n";
#endif
}



/* resets the counter of geographical patches that contain establishments  */
void Country::HostToEstablishmentMap::GeoPatchVec::ResetEstPtchsCntr()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::ResetEstPtchsCntr() started...\n";
#endif

	NumEstPtchs = 0;
	NumAvailEstPtchs = 0;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchVec::ResetEstPtchsCntr() completed.\n\n";
#endif
}



/* constructor; builds empty matrix */
Country::HostToEstablishmentMap::GeoPatchIndxMtrx::GeoPatchIndxMtrx(unsigned int xRef, unsigned int yRef, unsigned int gpGrnlty, unsigned int dimX, unsigned int dimY) : \
	XRef(xRef), YRef(yRef), Grnlty(gpGrnlty), DimX(dimX), DimY(dimY)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::GeoPatchIndxMtrx(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) started...\n";
#endif

	try
	{
		M = new unsigned int* [DimY];
		M[0] = new unsigned int [DimY * DimX];
		for(unsigned int i=1; i < DimY; i++)
			M[i] = M[i-1] + DimX;

		for(unsigned int i=0; i < DimY; i++)
			for(unsigned int j=0; j < DimX; j++)
				M[i][j] = (unsigned int)UNDEFINED;
	}
	catch(...)
	{
		cerr << "*** Error allocating matrix (M) of geographical patch indexes (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::GeoPatchIndxMtrx(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) completed.\n";
#endif
}



Country::HostToEstablishmentMap::GeoPatchIndxMtrx::~GeoPatchIndxMtrx()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::~GeoPatchIndxMtrx() started...\n";
#endif

	delete [] M[0];
	delete [] M;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::~GeoPatchIndxMtrx() completed.\n";
#endif
}



/* calculates geographical patch index X for a LandScan cell with index LndScnX */
unsigned int Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnXToX(unsigned int LndScnX)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnXToX(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnXToX(unsigned int) completed.\n\n";
#endif

	return (LndScnX - XRef)/Grnlty;
}



/* calculates geographical patch index Y for a LandScan cell with index LndScnY */
unsigned int Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnYToY(unsigned int LndScnY)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnYToY(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnYToY(unsigned int) completed.\n\n";
#endif

	return (LndScnY - YRef)/Grnlty;
}



/* calculates X coordinate (LandScan X index of the LandScan cell in the upper left patch corner) of the geographical patch where the LandScan cell with X coordinate LndScnCellX is located */
unsigned int Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellXToGPLdnScnX(unsigned int LndScnCellX)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellXToGPLdnScnX(unsigned int) started...\n";
#endif

	unsigned int l = (LndScnCellX - XRef)/Grnlty;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellXToGPLdnScnX(unsigned int) completed.\n\n";
#endif

	return XRef + l * Grnlty;
}



/* calculates Y coordinate (LandScan Y index of the LandScan cell in the upper left patch corner) of the geographical patch where the LandScan cell with Y coordinate LndScnCellY is located */
unsigned int Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellYToGPLdnScnY(unsigned int LndScnCellY)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellYToGPLdnScnY(unsigned int) started...\n";
#endif

	unsigned int l = (LndScnCellY - YRef)/Grnlty;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::LndScnCellYToGPLdnScnY(unsigned int) completed.\n\n";
#endif

	return YRef + l * Grnlty;
}



/* returns distance between geographical patches with indexes (x1, y1) and (x2, y2) */
/* pCLSCTbl: pointer to LandScan cell table for the country */
double Country::HostToEstablishmentMap::GeoPatchIndxMtrx::PatchDist(CntryLndScnCellTbl *pCLSCTbl, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::PatchDist(CntryLndScnCellTbl*, unsigned int, unsigned int, unsigned int, unsigned int) started...\n";
#endif

	if( (x1 == x2) && (y1 == y2) )
		return 0.0;

	unsigned int LndScnX1 = XRef + x1*Grnlty;   // corresponding LandScan indexes
	unsigned int LndScnY1 = YRef + y1*Grnlty;
	unsigned int LndScnX2 = XRef + x2*Grnlty;
	unsigned int LndScnY2 = YRef + y2*Grnlty;

	unsigned int LndScnX1_, LndScnY1_, LndScnX2_, LndScnY2_;
	if( (LndScnX1 < LndScnX2) || ((LndScnX1 == LndScnX2) && (LndScnY1 > LndScnY2)) )
	{
		LndScnX1_ = LndScnX1;
		LndScnY1_ = LndScnY1;
		LndScnX2_ = LndScnX2;
		LndScnY2_ = LndScnY2;
	}
	else
	{
		LndScnX1_ = LndScnX2;
		LndScnY1_ = LndScnY2;
		LndScnX2_ = LndScnX1;
		LndScnY2_ = LndScnY1;
	}
	if( LndScnX1_ < LndScnX2_ )
	{
		LndScnX1_ += Grnlty;
		if( LndScnY1_ > LndScnY2_ )
			LndScnY2_ += Grnlty;
		else if( LndScnY1_ < LndScnY2_ )
			LndScnY1_ += Grnlty;
	}
	else
		LndScnY2_ += Grnlty;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::GeoPatchIndxMtrx::PatchDist(CntryLndScnCellTbl*, unsigned int, unsigned int, unsigned int, unsigned int) completed.\n\n";
#endif

	return pCLSCTbl->LndScnCellDist(LndScnX1_, LndScnY1_, LndScnX2_, LndScnY2_);
}



/* default constructor */
Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::IndexEntry() : NumEls(0), NumElsMax(0), Indx(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::IndexEntry() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::IndexEntry() completed.\n\n";
#endif
}



/* constructor; builds empty index array of numElsMax elements */
Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::IndexEntry(unsigned int numElsMax) : NumEls(0), NumElsMax(numElsMax)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::IndexEntry::IndexEntry(unsigned int) started...\n";
#endif

	try
	{
		Indx = new unsigned int[NumElsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating index array (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	for(unsigned int i=0; i < NumElsMax; i++)
		Indx[i] = (unsigned int)UNDEFINED;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::IndexEntry::IndexEntry(unsigned int) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::~IndexEntry()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::~IndexEntry() started...\n";
#endif
	
	if( Indx != NULL )
		delete [] Indx;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::~IndexEntry() completed.\n\n";
#endif
}



/* adds host index into establishment index entry */
/* hostIndx: index of host in Host table */
void Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::AddIndex(unsigned int hostIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::IndexEntry::AddIndex(unsigned int) started...\n";
#endif

	if( NumEls == NumElsMax )
		throw INDX_OUT_OF_RANGE;

	Indx[NumEls++] = hostIndx;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::IndexEntry::AddIndex(unsigned int) completed.\n\n";
#endif
}



/* extracts random host index from establishment index entry */
unsigned int Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry::ExtractRndIndex()
{
	if( NumEls == 0 )
		throw INDX_OUT_OF_RANGE;

	unsigned int i = NumEls * ranf_();
	unsigned int hostIndx = Indx[i];

	if( i < --NumEls )
		Indx[i] = Indx[NumEls];

	return hostIndx;
}



/* constructor; builds empty table of numElsMax entries */
Country::HostToEstablishmentMap::HostToEstIndexTbl::HostToEstIndexTbl(unsigned int numElsMax) : NumElsMax(numElsMax)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::HostToEstIndexTbl(unsigned int) started...\n";
#endif

	try
	{
		Table = new IndexEntry[NumElsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating table of Country::HostToEstablishmentMap::IndexEntry entries (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::HostToEstIndexTbl(unsigned int) completed.\n";
#endif
}



Country::HostToEstablishmentMap::HostToEstIndexTbl::~HostToEstIndexTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::~HostToEstIndexTbl started...\n";
#endif

	delete [] Table;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::~HostToEstIndexTbl completed.\n";
#endif
}



/* adds empty index entry into table */
/* estIndx: establishment index in Table; estCapacty: establishment capacity */
void Country::HostToEstablishmentMap::HostToEstIndexTbl::AddEmptyIndexEntry(unsigned int estIndx, unsigned int estCapacty)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::AddIndexEntry(unsigned int, unsigned int) started...\n";
#endif

	if( estIndx >= NumElsMax )
		throw INDX_OUT_OF_RANGE;

	Table[estIndx].NumEls = 0;
	Table[estIndx].NumElsMax = estCapacty;
	try
	{
		Table[estIndx].Indx = new unsigned int[estCapacty];
	}
	catch(...)
	{
		cerr << "*** Error allocating index array (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	for(unsigned int i=0; i < estCapacty; i++)
		Table[estIndx].Indx[i] = (unsigned int)UNDEFINED;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::AddIndexEntry(unsigned int, unsigned int) completed.\n\n";
#endif
}



Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry* Country::HostToEstablishmentMap::HostToEstIndexTbl::GetIndxEntry(unsigned int estIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::GetIndxEntry(unsigned int) started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::HostToEstIndexTbl::GetIndxEntry(unsigned int) completed.\n\n";
#endif

	return &Table[estIndx];
}



/* (re)builds auxiliary probability distribution vectors, PDVec, for HOST vacancies from the current age group in each geographical patch */
void Country::HostToEstablishmentMap::BuildHostPDVs(CntryLndScnCellTbl *pCLSCTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::BuilHostdPDVs(CntryLndScnCellTbl*) started...\n";
#endif

	#pragma omp parallel for
	for(unsigned int i=0; i < PGPVec->NumEls; i++)
	{
		if( PGPVec->V[i].PPDV != NULL )                                                        // delete the old vector
			delete PGPVec->V[i].PPDV;

		PGPVec->V[i].PPDV = new GeoPatch::PDVec(PGPVec->NumEstPtchs + 1);                      // create a new  probability distribution vector
		
		unsigned int estGPXi = PGPIndxMtrx->LndScnXToX(PGPVec->V[i].GPLndScnX);                // X and Y indexes of the geographical patch that has index i in the vector GeoPatchVec 
		unsigned int estGPYi = PGPIndxMtrx->LndScnYToY(PGPVec->V[i].GPLndScnY);

		for(unsigned int j=0; j < PGPVec->NumEstPtchs; j++)                                    // loop over all the elements in the vector of geographical patches
		{
			unsigned int numVacancs = PGPVec->V[j].NumHostVacancs;                             // number of vacancies in patch j
			if( numVacancs == 0 )
				continue;
			
			unsigned int estGPXj = PGPIndxMtrx->LndScnXToX(PGPVec->V[j].GPLndScnX);            // X and Y indexes of the geographical patch that has index j in the vector GeoPatchVec 
			unsigned int estGPYj = PGPIndxMtrx->LndScnYToY(PGPVec->V[j].GPLndScnY);

			double R = PGPIndxMtrx->PatchDist(pCLSCTbl, estGPXi, estGPYi, estGPXj, estGPYj);   // distance between these patches

			PGPVec->V[i].PPDV->AddProbVal(j, (PHETDKernel->*F)(R) * numVacancs);               // add probability value into cumulative probability distribution vector (yet to be normalized!)
		}

		PGPVec->V[i].PPDV->Normalize();                                                        // normalize cumulative probability distribution vector
	}

	#pragma omp parallel for
	for(unsigned int i=0; i < PGPVec->NumEls; i++)                                             // reset the number of establishments and the number of vacancies
	{
		PGPVec->V[i].PEstIndxV->Reset();
		PGPVec->V[i].NumHostVacancs = 0;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::BuildHostPDVs(CntryLndScnCellTbl*) completed.\n";
#endif
}



/* (re)builds auxiliary probability distribution vectors, PDVec, for STAFF vacancies PDVec in each geographical patch */
void Country::HostToEstablishmentMap::BuildStaffPDVs(CntryLndScnCellTbl *pCLSCTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::BuildStaffPDVs(CntryLndScnCellTbl*) started...\n";
#endif

	#pragma omp parallel for
	for(unsigned int i=0; i < PGPVec->NumEls; i++)
	{
		PGPVec->V[i].NumRepHits = 0;

		if( PGPVec->V[i].PPDV != NULL )                                                        // delete the old vector
			delete PGPVec->V[i].PPDV;

		PGPVec->V[i].PPDV = new GeoPatch::PDVec(PGPVec->NumEstPtchs + 1);                      // create a new  probability distribution vector

		unsigned int estGPXi = PGPIndxMtrx->LndScnXToX(PGPVec->V[i].GPLndScnX);                // X and Y indexes of the geographical patch that has index i in the vector GeoPatchVec 
		unsigned int estGPYi = PGPIndxMtrx->LndScnYToY(PGPVec->V[i].GPLndScnY);

		for(unsigned int j=0; j < PGPVec->NumEstPtchs; j++)                                    // loop over all the elements in the vector of geographical patches
		{
			unsigned int numVacancs = PGPVec->V[j].NumStaffVacancs;                            // number of vacancies in patch j
			if( numVacancs == 0 )
				continue;
			
			unsigned int estGPXj = PGPIndxMtrx->LndScnXToX(PGPVec->V[j].GPLndScnX);            // X and Y indexes of the geographical patch that has index j in the vector GeoPatchVec 
			unsigned int estGPYj = PGPIndxMtrx->LndScnYToY(PGPVec->V[j].GPLndScnY);

			double R = PGPIndxMtrx->PatchDist(pCLSCTbl, estGPXi, estGPYi, estGPXj, estGPYj);   // distance between these patches

			PGPVec->V[i].PPDV->AddProbVal(j, (PHETDKernel->*F)(R) * numVacancs);               // add probability value into cumulative probability distribution vector (yet to be normalized!)
		}

		PGPVec->V[i].PPDV->Normalize();                                                        // normalize cumulative probability distribution vector
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::BuildStaffPDVs(CntryLndScnCellTbl*) completed.\n";
#endif
}



/* samples a patch; used in the rejection-acceptance establishment sampling */
/* x and y are coordinates (indexes in GeoPatchIndxMtrx matrix) of geographical patch for which sampling is done */
unsigned int Country::HostToEstablishmentMap::SamplePatch(unsigned int x, unsigned int y)
{
#ifdef OUTPUT_LOG_L2
	cout << "***  Country::HostToEstablishmentMap::SamplePatch(unsigned int, unsigned int) started...\n";
#endif

	unsigned int GPIndx = PGPIndxMtrx->M[y][x];
	GeoPatch::PDVec::PDVEl *V = PGPVec->V[GPIndx].PPDV->V;

	double rnd = ranf_();
	unsigned int lIndx = 0;
	unsigned int rIndx =  PGPVec->V[GPIndx].PPDV->NumEls - 1;
	unsigned int L;

	while( (L = rIndx - lIndx) > 1 )
	{
		if( rnd > V[lIndx + L/2].CProb )
			lIndx += L/2;
		else
			rIndx -= L/2;
	}

#ifdef OUTPUT_LOG_L2
	cout << "***  Country::HostToEstablishmentMap::SamplePatch(unsigned int, unsigned int) completed.\n\n";
#endif

	return V[rIndx].GeoPtchIndx;
}



/* creates Establishment object and adds it to Establishment table */
/* estGPIndx: index of a geographical patch (in the vector GeoPatchVec) where an establishment is to be located; */
/* pCLSCTbl: pointer to the LandScan cell table for the country; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
void Country::HostToEstablishmentMap::CreateEstablishment(unsigned int estGPIndx, CntryLndScnCellTbl *pCLSCTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::CreateEstablishment(unsigned int, CntryLndScnCellTbl*, ESDVec*, Country::EstablishmentTbl*) started...\n";
#endif

	Country::Establishment *pEst = new Establishment();

	GeoPatch *pGPtchAux = PGPVec->GetGeoPatch(estGPIndx);
	unsigned int cellIndx = pGPtchAux->PLSCVec->NumEls * ranf_();
	unsigned int LndScnIndx = pGPtchAux->PLSCVec->V[cellIndx];
	pEst->LndScnX = pCLSCTbl->Table[LndScnIndx].LndScnX;
	pEst->LndScnY = pCLSCTbl->Table[LndScnIndx].LndScnY;
	pEst->Lat = pCLSCTbl->LndScnYToLat(pEst->LndScnY) - pCLSCTbl->CellSize * ranf_();
	pEst->Lon = pCLSCTbl->LndScnXToLon(pEst->LndScnX) + pCLSCTbl->CellSize * ranf_();
	pEst->NumHostVacancs = pESDV->SampleSize();
	pEst->NumHosts = 0;
	pEst->NumStaffVacancs = (float)pEst->NumHostVacancs * pESDV->GetStaffRatio();
	if( pEst->NumStaffVacancs == 0 )
		pEst->NumStaffVacancs++;
	pEst->NumStaff = 0;
	PGPVec->V[estGPIndx].PEstIndxV->AddEstablishmentIndx(pEstTbl->GetFirstEmptyRecIndex());
	pEstTbl->AddRec(pEst);

	PGPVec->V[estGPIndx].NumHostVacancs += pEst->NumHostVacancs;
	PGPVec->V[estGPIndx].NumStaffVacancs += pEst->NumStaffVacancs;
				
	delete pEst;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostToEstablishmentMap::CreateEstablishment(unsigned int, CntryLndScnCellTbl*, ESDVec*, Country::EstablishmentTbl*) completed.\n\n";
#endif
}



/* creates table of references between Establishment table and Host table */
void Country::HostToEstablishmentMap::CreateHostToEstIndexTbl(Country::EstablishmentTbl *pEstTbl, Country::HostTbl *pHostTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CreateHostToEstIndexTbl(Country::EstablishmentTbl*, Country::HostTbl*) started...\n";
#endif

	if( PHEIndxTbl != NULL )
		delete PHEIndxTbl;

	PHEIndxTbl = new HostToEstIndexTbl(pEstTbl->GetNumEstablishments());

	unsigned short crntAgeGroupIndx = PHETDKernel->GetCrntAgeGroupIndx();

	unsigned int maxNumThreads = omp_get_max_threads();
	Country::Host *Host = new Country::Host[maxNumThreads];
	Country::Establishment *Est = new Country::Establishment[maxNumThreads];

	unsigned int numHosts = pHostTbl->GetNumHosts();
	#pragma omp parallel for
	for(unsigned int i=0; i < numHosts; i++)
	{
		pHostTbl->GetHostRec(i, &Host[omp_get_thread_num()]);

		if( Host[omp_get_thread_num()].EstID.ID1 != crntAgeGroupIndx )
			continue;

		unsigned int estIndx = Host[omp_get_thread_num()].EstID.ID2;

		if( estIndx == (unsigned int)UNDEFINED )
			continue;

		pEstTbl->GetEstablishmentRec(estIndx, &Est[omp_get_thread_num()]);
		unsigned int estCapcty = Est[omp_get_thread_num()].NumHosts + Est[omp_get_thread_num()].NumStaff;

		Country::HostToEstablishmentMap::HostToEstIndexTbl::IndexEntry *pIndxEntry;
		#pragma omp critical (AddIndex)
		{
		pIndxEntry = PHEIndxTbl->GetIndxEntry(estIndx);
		if( pIndxEntry->Indx == NULL )
			PHEIndxTbl->AddEmptyIndexEntry(estIndx, estCapcty);
		pIndxEntry->AddIndex(i);
		#pragma omp flush
		}
	}

	delete [] Host;
	delete [] Est;

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Index table of " << pEstTbl->GetNumEstablishments() << " entries created.\n";
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CreateHostToEstIndexTbl(Country::EstablishmentTbl*, Country::HostTbl*) completed.\n";
#endif
}



void Country::HostToEstablishmentMap::DeleteHostToEstIndexTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::DeleteHostToEstIndexTbl() started...\n";
#endif

	if( PHEIndxTbl != NULL )
	{
		delete PHEIndxTbl;
		PHEIndxTbl = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::DeleteHostToEstIndexTbl() completed.\n";
#endif
}



/* auxiliary class describing household index vector */
class HouseholdIndxVec
{
private:
	unsigned int NumEls;              // number of nonempty elements in V
	unsigned int NumElsMax;           // max. allowed number of elements
	unsigned int *V;                  // vector of indexes in Household table
	
public:
	class Indx
	{
	public:
		unsigned int ElIndx;          // index of element in vector V
		unsigned int HouseholdIndx;   // index of household in Household table
	};

	HouseholdIndxVec(unsigned int numHslhds) : NumEls(numHslhds), NumElsMax(numHslhds)
	{
		try
		{
			V = new unsigned int[NumEls];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (V) of household indexes (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

		for(unsigned int i=0; i < NumEls; i++)
			V[i] = i;
	}


	~HouseholdIndxVec()
	{
		if( V != NULL)
		{
			delete [] V;
			V = NULL;
		}
	}


	/* gets random household index */
	void GetRndHshldIndx(Indx *pIndx)
	{
		if( NumEls != 0 )
		{
			unsigned int indx = NumEls * ranf_();
			pIndx->ElIndx = indx;
			pIndx->HouseholdIndx = V[indx];
		}
		else
		{
			pIndx->ElIndx = (unsigned int)UNDEFINED;
			pIndx->HouseholdIndx = (unsigned int)UNDEFINED;
		}
	}


	/* removes element (household index) from vector V */
	void RemoveHshldIndx(Indx *pIndx)
	{
		unsigned int indx = pIndx->ElIndx;
		if( (indx < NumEls) && (pIndx->HouseholdIndx == V[indx]) )
		{
			if( NumEls != 0 && indx < --NumEls )
				V[indx] = V[NumEls];
		}
	}


	/* returns the number of available households */
	unsigned int GetNumAvailHshlds()
	{
		return NumEls;
	}
};



/* constructor; builds geographical patch index matrix and vector of geographical patches for allocating HOSTS from the target age group to establishments */
/* pCLSCTbl: LandScan cell table for the country; GPGrnlty: granularity of geographical patches (number of LandScan cells in each dimension); pEstTbl: table of Establishment objects */
/* pHETDKernel: kernel function object */
Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl *pCLSCTbl, unsigned int GPGrnlty, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel) : \
	PHETDKernel(pHETDKernel), PHEIndxTbl(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl*, unsigned int, Country::EstablishmentTbl*, Country::EstablishmentTbl::EstablishmentParams*, HostToEstTravelDistrKernel*) started...\n";
#endif

	unsigned int LndScnXL, LndScnXR;                    // leftmost and righmost X indexes in the LandScan dataset for the given country - find them
	LndScnXL = LndScnXR = pCLSCTbl->Table[0].LndScnX;
	unsigned int LndScnYU, LndScnYB;                    // upper and bottom Y indexes in the LandScan dataset - find them
	LndScnYU = LndScnYB = pCLSCTbl->Table[0].LndScnY;
	for(unsigned i=1; i < pCLSCTbl->NumRecs; i++)
	{
		if( LndScnXL > pCLSCTbl->Table[i].LndScnX )
			LndScnXL = pCLSCTbl->Table[i].LndScnX;
		if( LndScnXR < pCLSCTbl->Table[i].LndScnX )
			LndScnXR = pCLSCTbl->Table[i].LndScnX;
		if( LndScnYU > pCLSCTbl->Table[i].LndScnY )
			LndScnYU = pCLSCTbl->Table[i].LndScnY;
		if( LndScnYB < pCLSCTbl->Table[i].LndScnY )
			LndScnYB = pCLSCTbl->Table[i].LndScnY;
	}

	unsigned int GPXRef = LndScnXL;                                                       // coordinates of the reference point - the start of the "local" geographical patch grid
	unsigned int GPYRef = LndScnYU;                                                       // represented as a pair of LandScan indexes of the upper left LandScan cell in the upper left patch

	unsigned int dimX = (LndScnXR - LndScnXL + 1)/GPGrnlty;                               // X and Y dimensions of the geographical patch index matrix
	if( (LndScnXR - LndScnXL + 1) % GPGrnlty != 0 )
		dimX++;
	unsigned int dimY = (LndScnYB - LndScnYU + 1)/GPGrnlty;
	if( (LndScnYB - LndScnYU + 1) % GPGrnlty != 0 )
		dimY++;

	PGPIndxMtrx = new GeoPatchIndxMtrx(GPXRef, GPYRef, GPGrnlty, dimX, dimY);             // create empty geographical patch index matrix

	PGPVec = new GeoPatchVec();                                                           // create empty geographical patch vector

	unsigned int numEst = pEstTbl->GetNumEstablishments();                                // get the number of establishments
	for(unsigned int i=0; i < numEst; i++)                                                // fill in geographical patch vector GeoPatchVec in this loop
	{
		unsigned int estLndScnX, estLndScnY;
		pEstTbl->GetEstablishmentLndScnIndxs(i, &estLndScnX, &estLndScnY);                // get the indexes of the LandScan cell where the establishment is located
		unsigned int estGPX = PGPIndxMtrx->LndScnXToX(estLndScnX);                        // get the indexes of the geographical patch where this LandScan cell is located
		unsigned int estGPY = PGPIndxMtrx->LndScnYToY(estLndScnY);
		unsigned int estGPIndx = PGPIndxMtrx->M[estGPY][estGPX];                          // get the appropriate patch index in the vector of geographical patches GeoPatchVec
		if( estGPIndx == (unsigned int)UNDEFINED )                                        // geographical patch index is not in GeoPatchIndxMtrx matrix and it is not referenced in GeoPatchVec vector yet - add it here
		{
			unsigned int GPLndScnX = PGPIndxMtrx->LndScnCellXToGPLdnScnX(estLndScnX);
			unsigned int GPLndScnY = PGPIndxMtrx->LndScnCellYToGPLdnScnY(estLndScnY);
			GeoPatch *pGPtch = new GeoPatch(GPLndScnX, GPLndScnY);
			estGPIndx = PGPVec->AddGeoPatch(pGPtch);
			PGPVec->IncrEstPtchsCntr();
			PGPIndxMtrx->M[estGPY][estGPX] = estGPIndx;
			delete pGPtch;
		}
		unsigned int estIndx = PGPVec->V[estGPIndx].PEstIndxV->AddEstablishmentIndx(i);   // add an establishment index (in the Establishment objects table) into the vector of indexes associated with a given patch
	}

	for(unsigned int i=0; i < pCLSCTbl->NumRecs; i++)
	{
		unsigned int GPX = PGPIndxMtrx->LndScnXToX(pCLSCTbl->Table[i].LndScnX);
		unsigned int GPY = PGPIndxMtrx->LndScnYToY(pCLSCTbl->Table[i].LndScnY);

		unsigned int GPIndx = PGPIndxMtrx->M[GPY][GPX];
		if( GPIndx == (unsigned int)UNDEFINED )
		{
			unsigned int GPLndScnX = PGPIndxMtrx->LndScnCellXToGPLdnScnX(pCLSCTbl->Table[i].LndScnX);
			unsigned int GPLndScnY = PGPIndxMtrx->LndScnCellYToGPLdnScnY(pCLSCTbl->Table[i].LndScnY);
			GeoPatch *pGPtch = new GeoPatch(GPLndScnX, GPLndScnY);
			GPIndx = PGPVec->AddGeoPatch(pGPtch);
			PGPIndxMtrx->M[GPY][GPX] = GPIndx;
			delete pGPtch;
		}

		GeoPatch *pGPtchAux = PGPVec->GetGeoPatch(GPIndx);
		pGPtchAux->PLSCVec->AddLandScanCellIndx(i);
	}

	Establishment *pEst = new Establishment;
	for(unsigned int i=0; i < PGPVec->NumEstPtchs; i++)
	{
		unsigned int numEst = PGPVec->V[i].PEstIndxV->NumEls;                        // number of establishments in patch i
		if( numEst == 0 )                                                            // all the establishments in patch i have been occupied; if this condition is met, do not add the patch index in the probability vector PGPVec->V[i].PPDV
			continue;
			
		for(unsigned int j=0; j < numEst; j++)                                       // count the number of vacancies in a patch
		{
			pEstTbl->GetEstablishmentRec(PGPVec->V[i].PEstIndxV->V[j], pEst);
			PGPVec->V[i].NumHostVacancs += pEst->NumHostVacancs;
			PGPVec->V[i].NumStaffVacancs += pEst->NumStaffVacancs;
		}
	}
	delete pEst;

	F = PHETDKernel->F();                                                            // get the pointer to the chosen kernel function
	BuildHostPDVs(pCLSCTbl);                                                         // build auxiliary cumulative probability distribution vectors in the geographical patches

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl*, unsigned int, Country::EstablishmentTbl*, Country::EstablishmentTbl::EstablishmentParams*, HostToEstTravelDistrKernel*) completed.\n";
#endif
}



/* constructor; builds geographical patch index matrix and vector of geographical patches for allocating STAFF (hosts from a selected age group, different from the target one) to establishments */
/* hosts are assumed to have been previously allocated to establishments */
/* pCLSCTbl: LandScan cell table for the country; GPGrnlty: granularity of geographical patches (number of LandScan cells in each dimension); pESDV: pointer to the establishment size distribution vector */
/* pEstTbl: table of Establishment objects; pHETDKernel: kernel function object */
Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl *pCLSCTbl, unsigned int GPGrnlty, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel) : \
	PHETDKernel(pHETDKernel), PHEIndxTbl(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl*, unsigned int, Country::EstablishmentTbl*, HostToEstTravelDistrKernel*) started...\n";
#endif

	unsigned int LndScnXL, LndScnXR;                    // leftmost and righmost X indexes in the LandScan dataset for the given country - find them
	LndScnXL = LndScnXR = pCLSCTbl->Table[0].LndScnX;
	unsigned int LndScnYU, LndScnYB;                    // upper and bottom Y indexes in the LandScan dataset - find them
	LndScnYU = LndScnYB = pCLSCTbl->Table[0].LndScnY;
	for(unsigned i=1; i < pCLSCTbl->NumRecs; i++)
	{
		if( LndScnXL > pCLSCTbl->Table[i].LndScnX )
			LndScnXL = pCLSCTbl->Table[i].LndScnX;
		if( LndScnXR < pCLSCTbl->Table[i].LndScnX )
			LndScnXR = pCLSCTbl->Table[i].LndScnX;
		if( LndScnYU > pCLSCTbl->Table[i].LndScnY )
			LndScnYU = pCLSCTbl->Table[i].LndScnY;
		if( LndScnYB < pCLSCTbl->Table[i].LndScnY )
			LndScnYB = pCLSCTbl->Table[i].LndScnY;
	}

	unsigned int GPXRef = LndScnXL;                                                       // coordinates of the reference point - the start of the "local" geographical patch grid
	unsigned int GPYRef = LndScnYU;                                                       // represented as a pair of LandScan indexes of the upper left LandScan cell in the upper left patch

	unsigned int dimX = (LndScnXR - LndScnXL + 1)/GPGrnlty;                               // X and Y dimensions of the geographical patch index matrix
	if( (LndScnXR - LndScnXL + 1)%GPGrnlty != 0 )
		dimX++;
	unsigned int dimY = (LndScnYB - LndScnYU + 1)/GPGrnlty;
	if( (LndScnYB - LndScnYU + 1)%GPGrnlty != 0 )
		dimY++;

	PGPIndxMtrx = new GeoPatchIndxMtrx(GPXRef, GPYRef, GPGrnlty, dimX, dimY);             // create empty geographical patch index matrix

	PGPVec = new GeoPatchVec();                                                           // create empty geographical patch vector

	unsigned int numEst = pEstTbl->GetNumEstablishments();                                // get the number of establishments
	for(unsigned int i=0; i < numEst; i++)                                                // fill in geographical patch vector GeoPatchVec in this loop
	{
		unsigned int estLndScnX, estLndScnY;
		pEstTbl->GetEstablishmentLndScnIndxs(i, &estLndScnX, &estLndScnY);                // get the indexes of the LandScan cell where the establishment is located
		unsigned int estGPX = PGPIndxMtrx->LndScnXToX(estLndScnX);                        // get the indexes of the geographical patch where this LandScan cell is located
		unsigned int estGPY = PGPIndxMtrx->LndScnYToY(estLndScnY);
		unsigned int estGPIndx = PGPIndxMtrx->M[estGPY][estGPX];                          // get the appropriate patch index in the vector of geographical patches GeoPatchVec
		if( estGPIndx == (unsigned int)UNDEFINED )                                        // geographical patch index is not in GeoPatchIndxMtrx matrix and it is not referenced in GeoPatchVec vector yet - add it here
		{
			unsigned int GPLndScnX = PGPIndxMtrx->LndScnCellXToGPLdnScnX(estLndScnX);
			unsigned int GPLndScnY = PGPIndxMtrx->LndScnCellYToGPLdnScnY(estLndScnY);
			GeoPatch *pGPtch = new GeoPatch(GPLndScnX, GPLndScnY);
			estGPIndx = PGPVec->AddGeoPatch(pGPtch);
			PGPVec->IncrEstPtchsCntr();
			PGPIndxMtrx->M[estGPY][estGPX] = estGPIndx;
			delete pGPtch;
		}
		unsigned int estIndx = PGPVec->V[estGPIndx].PEstIndxV->AddEstablishmentIndx(i);   // add an establishment index (in the Establishment objects table) into the vector of indexes associated with a given patch
	}

	for(unsigned int i=0; i < pCLSCTbl->NumRecs; i++)
	{
		unsigned int GPX = PGPIndxMtrx->LndScnXToX(pCLSCTbl->Table[i].LndScnX);
		unsigned int GPY = PGPIndxMtrx->LndScnYToY(pCLSCTbl->Table[i].LndScnY);

		unsigned int GPIndx = PGPIndxMtrx->M[GPY][GPX];
		if( GPIndx == (unsigned int)UNDEFINED )
		{
			unsigned int GPLndScnX = PGPIndxMtrx->LndScnCellXToGPLdnScnX(pCLSCTbl->Table[i].LndScnX);
			unsigned int GPLndScnY = PGPIndxMtrx->LndScnCellYToGPLdnScnY(pCLSCTbl->Table[i].LndScnY);
			GeoPatch *pGPtch = new GeoPatch(GPLndScnX, GPLndScnY);
			GPIndx = PGPVec->AddGeoPatch(pGPtch);
			PGPIndxMtrx->M[GPY][GPX] = GPIndx;
			delete pGPtch;
		}

		GeoPatch *pGPtchAux = PGPVec->GetGeoPatch(GPIndx);
		pGPtchAux->PLSCVec->AddLandScanCellIndx(i);
	}

	Establishment *pEst = new Establishment;
	for(unsigned int i=0; i < PGPVec->NumEstPtchs; i++)
	{
		unsigned int numEst = PGPVec->V[i].PEstIndxV->NumEls;                        // number of establishments in patch i
		if( numEst == 0 )                                                            // all the establishments in patch i have been occupied; if this condition is met, do not add the patch index in the probability vector PGPVec->V[i].PPDV
			continue;
			
		for(unsigned int j=0; j < numEst; j++)                                       // count the number of vacancies in a patch
		{
			pEstTbl->GetEstablishmentRec(PGPVec->V[i].PEstIndxV->V[j], pEst);
			PGPVec->V[i].NumHostVacancs += pEst->NumHostVacancs;
			PGPVec->V[i].NumStaffVacancs += pEst->NumStaffVacancs;
		}
	}
	delete pEst;

	unsigned short hostAgeGroupIndx = pESDV->GetAgeGroupIndx();
	unsigned short staffAgeGroupIndx = pESDV->GetStaffAgeGroupIndx();
	pESDV->SetAgeGroupIndx(staffAgeGroupIndx);
	PHETDKernel->SetAgeGroupIndx(staffAgeGroupIndx);
	F = PHETDKernel->F();                                                            // get the pointer to the chosen kernel function
	BuildStaffPDVs(pCLSCTbl);                                                        // build auxiliary cumulative probability distribution vectors in the geographical patches
	
	pESDV->SetAgeGroupIndx(hostAgeGroupIndx);
	PHETDKernel->SetAgeGroupIndx(hostAgeGroupIndx);
	F = PHETDKernel->F();

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::HostToEstablishmentMap(CntryLndScnCellTbl*, unsigned int, Country::EstablishmentTbl*, HostToEstTravelDistrKernel*) completed.\n";
#endif
}



Country::HostToEstablishmentMap::~HostToEstablishmentMap()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::~HostToEstablishmentMap() started...\n";
#endif

	if( PGPVec != NULL )
	{
		delete PGPVec;
		PGPVec = NULL;
	}

	if( PGPIndxMtrx != NULL )
	{
		delete PGPIndxMtrx;
		PGPIndxMtrx = NULL;
	}

	if( PHEIndxTbl != NULL )
	{
		delete PHEIndxTbl;
		PHEIndxTbl = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::~HostToEstablishmentMap() completed.\n";
#endif
}



#define ALG_VER_1   // algorithm version
#define NUM_REP_HITS_THRESHOLD_HTE_1 100000
#define NUM_REP_HITS_THRESHOLD_HTE_2 300
/* assigns hosts to establishments */
/* pCLSCTbl: pointer to the LandScan cell table for the country; pHostTbl: pointer to the table of hosts; */
/* pHshldTbl: pointer to the Household table; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
void Country::HostToEstablishmentMap::AssignHostsToEstablishments(CntryLndScnCellTbl *pCLSCTbl, Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::AssignHostsToEstablishments(CntryLndScnCellTbl*, Country::HostTbl*, Country::HouseholdTbl*, ESDVec*, Country::EstablishmentTbl*) started...\n";
#endif

	pEstTbl->ResetTable();
	PGPVec->ResetEstPtchsCntr();

	unsigned short hostAgeGroupIndx = pESDV->GetAgeGroupIndx();
	unsigned short staffAgeGroupIndx = pESDV->GetStaffAgeGroupIndx();
	unsigned int numRepHitsThreshold = 0;
	if( hostAgeGroupIndx != staffAgeGroupIndx )
		numRepHitsThreshold = NUM_REP_HITS_THRESHOLD_HTE_1;
	else
		numRepHitsThreshold = NUM_REP_HITS_THRESHOLD_HTE_2;

	HouseholdIndxVec *pHshldIndxV = new HouseholdIndxVec(pHshldTbl->GetNumHouseholds());     // create vector of household indexes

	float ageLB = PHETDKernel->GetCrntLowerAgeGroupBoundary();
	float ageUB = PHETDKernel->GetCrntUpperAgeGroupBoundary();

	float fillRatio = pESDV->GetFillRatio();

	
	#pragma omp parallel
	{
	HouseholdIndxVec::Indx hshldIndx;
	unsigned short hostIndx;
	Household *pHshld = new Household;

	while(true)
	{
		pHshldIndxV->GetRndHshldIndx(&hshldIndx);                                            // get a random household index
		if( hshldIndx.HouseholdIndx == (unsigned int)UNDEFINED )                             // if all the Household objects have been processed, terminate the loop
			break;

		pHshldTbl->GetHousehold(hshldIndx.HouseholdIndx, pHshld);                            // get the Household object
		hostIndx = pHostTbl->GetRndUnassgndHostIndx(pHshld, ageLB, ageUB);                   // get a random unassigned host index (within household)
		if( hostIndx == (unsigned short)UNDEFINED )
		{
			#pragma omp critical (RemoveHshldIndx)
			pHshldIndxV->RemoveHshldIndx(&hshldIndx);
			#pragma omp flush

			continue;
		}
		
		float rnd = ranf_();
		if( rnd > fillRatio )
		{
			#pragma omp critical (AccessGeoPatchData)
			{
			if( pHshld->GetEstablishmentID1(hostIndx) == (unsigned short)UNDEFINED )
			{
				pHshld->SetEstablishmentID(hostIndx, hostAgeGroupIndx, (unsigned int)UNDEFINED);
			}
			#pragma omp flush
			}

			continue;
		}

		double hostLat = pHshld->Lat;                                                        // host Lat and Lon coordinates
		double hostLon = pHshld->Lon;                                                        //

		unsigned int hostLndScnX = pHshld->LndScnX;                                          // LandScan X and Y indexes of the cell containing the household
		unsigned int hostLndScnY = pHshld->LndScnY;                                          //

		unsigned int hostGPX = PGPIndxMtrx->LndScnXToX(hostLndScnX);                         // X and Y indexes of the appropriate geographical patch
		unsigned int hostGPY = PGPIndxMtrx->LndScnYToY(hostLndScnY);                         //

#ifndef ALG_VER_2
		while(true)
#endif
		{
			unsigned int estGPIndx = SamplePatch(hostGPX, hostGPY);                          // for the given geogr. patch (hostGPX, hostGPY) where the host is located, sample a geographical patch index in the vector GeoPatchVec
			if( PGPVec->V[estGPIndx].PEstIndxV->NumEls == 0 )
			{
				#pragma omp atomic
				++PGPVec->V[estGPIndx].NumRepHits;                                           // increment the hit counter

				if( PGPVec->V[estGPIndx].NumRepHits <= numRepHitsThreshold )
					continue;

				//#pragma omp critical (AccessGeoPatchData)
				if( PGPVec->V[estGPIndx].NumRepHits > numRepHitsThreshold )                  // check if the hit counter is above the threshold
				{
					#pragma omp critical (AccessGeoPatchData)
					{
					PGPVec->V[estGPIndx].NumRepHits = 0;                                     // set the hit counter back to zero
					CreateEstablishment(estGPIndx, pCLSCTbl, pESDV, pEstTbl);                // create a new establishment in the patch
					PGPVec->IncrEstPtchsCntr();                                              // increment the counter of geographical patches containing establishments
					#pragma omp flush
					}
				}
			}
			
			GeoPatch::EstablishmentIndxVec::Indx estIndx;
			#pragma omp critical (AccessGeoPatchData)
			{
			PGPVec->V[estGPIndx].PEstIndxV->GetEstablishmentIndx(&estIndx);                  // get the index of an establishment located within a patch (the last one in the vector)
			}
			if( estIndx.EstObjIndx == (unsigned int)UNDEFINED )
				continue;

			double estLat, estLon;
			pEstTbl->GetEstablishmentCoords(estIndx.EstObjIndx, &estLat, &estLon);           // get the establishment Lat and Lon coordinates
			double r = CntryLndScnCellTbl::PointDist(hostLat, hostLon, estLat, estLon);      // distance between the host and the establishment

			unsigned int estGPX = PGPIndxMtrx->LndScnXToX(PGPVec->V[estGPIndx].GPLndScnX);   // get the establishment geographical patch indexes 
			unsigned int estGPY = PGPIndxMtrx->LndScnYToY(PGPVec->V[estGPIndx].GPLndScnY);   // 
			double R = PGPIndxMtrx->PatchDist(pCLSCTbl, hostGPX, hostGPY, estGPX, estGPY);   // distance between the patches where the host and the establishment are located

			if( ranf_() < (PHETDKernel->*F)(r)/(PHETDKernel->*F)(R) )                        // if this condition is met, accept a sample; otherwise reject and resample
			{
				#pragma omp critical (AccessGeoPatchData)
				{
				if( pHshld->GetEstablishmentID1(hostIndx) == (unsigned short)UNDEFINED )
				{
					try
					{
						if( pEstTbl->AssignHost(estIndx.EstObjIndx) == 0 )                                 // attempt to assign the host to the establishment
						{
							if( PGPVec->V[estGPIndx].PEstIndxV->RemoveEstablishmentIndx(&estIndx) == 0 )   // if all the vacancies have been filled, remove the element with index estIndx from the vector of establishment indexes in the patch estGPIndx
								PGPVec->DecrAvailEstPtchsCntr();                                           // decrement the amount of patches with availble establishments
						}
						pHshld->SetEstablishmentID(hostIndx, hostAgeGroupIndx, estIndx.EstObjIndx);        // add the establishment ID to the host record
					}
					catch(int excCode)
					{
						if( excCode != TABLE_TOO_SMALL )
							throw;
					}
				}
				#pragma omp flush
				}
#ifndef ALG_VER_2
				break;
#endif
			}
		}
	}

	delete pHshld;
	}

	delete pHshldIndxV;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::AssignHostsToEstablishments(CntryLndScnCellTbl*, Country::HostTbl*, Country::HouseholdTbl*, ESDVec*, Country::EstablishmentTbl*) completed.\n";
#endif
}



#define NUM_REP_HITS_THRESHOLD_STE 10000000
/* assigns staff to establishments */
/* pCLSCTbl: pointer to the LandScan cell table for the country; pHostTbl: pointer to the table of hosts; */
/* pHshldTbl: pointer to the Household table; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
void Country::HostToEstablishmentMap::AssignStaffToEstablishments(CntryLndScnCellTbl *pCLSCTbl, Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::AssignStaffToEstablishments(CntryLndScnCellTbl*, Country::HostTbl*, Country::HouseholdTbl*, ESDVec*, Country::EstablishmentTbl*) started...\n";
#endif

	unsigned short hostAgeGroupIndx = pESDV->GetAgeGroupIndx();
	unsigned short staffAgeGroupIndx = pESDV->GetStaffAgeGroupIndx();
	pESDV->SetAgeGroupIndx(staffAgeGroupIndx);
	PHETDKernel->SetAgeGroupIndx(staffAgeGroupIndx);
	F = PHETDKernel->F();

	HouseholdIndxVec *pHshldIndxV = new HouseholdIndxVec(pHshldTbl->GetNumHouseholds());      // create vector of household indexes

	float ageLB = PHETDKernel->GetCrntLowerAgeGroupBoundary();
	float ageUB = PHETDKernel->GetCrntUpperAgeGroupBoundary();


	bool runLoopFlag = true;
	#pragma omp parallel shared(runLoopFlag)
	{
	HouseholdIndxVec::Indx hshldIndx;
	unsigned short hostIndx;
	Household *pHshld = new Household;

	while(runLoopFlag)
	{
		pHshldIndxV->GetRndHshldIndx(&hshldIndx);                                             // get a random household index
		if( hshldIndx.HouseholdIndx == (unsigned int)UNDEFINED )                              // in case all the Household objects have been processed, terminate the loop
		{
			runLoopFlag = false;
			#pragma omp flush

			break;
		}

		pHshldTbl->GetHousehold(hshldIndx.HouseholdIndx, pHshld);                             // get the Household object
		hostIndx = pHostTbl->GetRndUnassgndHostIndx(pHshld, ageLB, ageUB);                    // get a random unassigned host index (within household)
		if( hostIndx == (unsigned short)UNDEFINED )
		{
			#pragma omp critical (RemoveHshldIndx)
			pHshldIndxV->RemoveHshldIndx(&hshldIndx);
			#pragma omp flush

			continue;
		}

		double hostLat = pHshld->Lat;                                                         // host Lat and Lon coordinates
		double hostLon = pHshld->Lon;                                                         //

		unsigned int hostLndScnX = pHshld->LndScnX;                                           // LandScan X and Y indexes of the cell containing the household
		unsigned int hostLndScnY = pHshld->LndScnY;                                           //

		unsigned int hostGPX = PGPIndxMtrx->LndScnXToX(hostLndScnX);                          // X and Y indexes of the appropriate geographical patch
		unsigned int hostGPY = PGPIndxMtrx->LndScnYToY(hostLndScnY);                          //

		unsigned int estGPIndx;
		#pragma omp critical (CheckHitsThreshold)
		{
		estGPIndx = SamplePatch(hostGPX, hostGPY);                                            // for the given geogr. patch (hostGPX, hostGPY) where the host-prospective staff memeber is located, sample a geographical patch index in the vector GeoPatchVec
		}

		if( estGPIndx == (unsigned int)UNDEFINED )                                            // if this condition is met, all staff vacancies have been filled
		{
			runLoopFlag = false;
			#pragma omp flush

			break;
		}

		if( PGPVec->V[estGPIndx].PEstIndxV->NumEls == 0 )                                     // check if there are any establishments in the sampled patch with vacancies for staff remaining unfilled
		{
			#pragma omp critical (CheckHitsThreshold)
			{
			if( ++PGPVec->V[estGPIndx].NumRepHits > NUM_REP_HITS_THRESHOLD_STE )              // if not, increment the hit counter
			{
				BuildStaffPDVs(pCLSCTbl);                                                     // if the hit counter reaches the threshold value, rebuild the probability distribution vectors
				cout << "*** Warning: Check the travel distance distribution tail; increase of NUM_REP_HITS_THRESHOLD_STE may be needed to get a more proper approximation.\n";
				#pragma omp flush
			}
			}
			continue;
		}
		
		GeoPatch::EstablishmentIndxVec::Indx estIndx;
		#pragma omp critical (AccessGeoPatchData)
		{
		PGPVec->V[estGPIndx].PEstIndxV->GetEstablishmentIndx(&estIndx);                       // get the index of an establishment located within a patch (the last one in the vector)
		}
		if( estIndx.EstObjIndx == (unsigned int)UNDEFINED )
			continue;

		double estLat, estLon;
		pEstTbl->GetEstablishmentCoords(estIndx.EstObjIndx, &estLat, &estLon);                // get the establishment Lat and Lon coordinates
		double r = CntryLndScnCellTbl::PointDist(hostLat, hostLon, estLat, estLon);           // distance between the host and the establishment

		unsigned int estGPX = PGPIndxMtrx->LndScnXToX(PGPVec->V[estGPIndx].GPLndScnX);        // get the establishment geographical patch indexes 
		unsigned int estGPY = PGPIndxMtrx->LndScnYToY(PGPVec->V[estGPIndx].GPLndScnY);        // 
		double R = PGPIndxMtrx->PatchDist(pCLSCTbl, hostGPX, hostGPY, estGPX, estGPY);        // distance between the patches where the host and the establishment are located

		if( ranf_() < (PHETDKernel->*F)(r)/(PHETDKernel->*F)(R) )                             // if this condition is met, accept a sample; otherwise reject and resample
		{
		#pragma omp critical (AccessGeoPatchData)
		{
			if( pHshld->GetEstablishmentID1(hostIndx) == (unsigned short)UNDEFINED )
			{
				try
				{
					if( pEstTbl->AssignStaff(estIndx.EstObjIndx) == 0 )                                // attempt to assign the staff member to the establishment
					{
						if( PGPVec->V[estGPIndx].PEstIndxV->RemoveEstablishmentIndx(&estIndx) == 0 )   // if all the vacancies have been filled, remove the element with index estIndx from the vector of establishment indexes in the patch estGPIndx
						{
							if( PGPVec->DecrAvailEstPtchsCntr() == 0 )                                 // decrement the amount of patches with availble establishments
								runLoopFlag = false;
						}
					}
					pHshld->SetEstablishmentID(hostIndx, hostAgeGroupIndx, estIndx.EstObjIndx);        // add the establishment ID to the host record; now this host is a member of staff in the given establishment
					PGPVec->V[estGPIndx].NumStaffVacancs--;                                            // decrement the amount of staff vacancies in the given patch
				}
				catch(int excCode)
				{
					if( excCode != TABLE_TOO_SMALL )
						throw;
				}
			}
			#pragma omp flush
		}
		}
	}

	delete pHshld;
	}

	delete pHshldIndxV;

	pESDV->SetAgeGroupIndx(hostAgeGroupIndx);
	PHETDKernel->SetAgeGroupIndx(hostAgeGroupIndx);
	F = PHETDKernel->F();

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::AssignStaffToEstablishments(CntryLndScnCellTbl*, Country::HostTbl*, Country::HouseholdTbl*, ESDVec*, Country::EstablishmentTbl*) completed.\n";
#endif
}



/* helper class used to assign group IDs to hosts */
class HostAgeBandCounter
{
public:
	class HostCounter
	{
	public:
		unsigned short CrntGroupID;   // currrent host group ID
		unsigned int GroupSize;       // host group size
		unsigned int NumGroups;       // total number of groups corresponding to current age band
		unsigned int CrntGroupSize;   // current host group size
		unsigned int CrntNumGroups;   // current number of groups (CrntGroupNum <= NumGroups)
		unsigned int NumHosts;        // number of hosts corresponding to age band

		/* default constructor */
		HostCounter();

		/* calculates number of groups */
		void SetGroupSize(unsigned int groupSize);
	};
	
	static unsigned short NumAgeBands;   // number of age bands in age group (number of elements in V)
	unsigned short MaxGroupID;           // max host group ID
	HostCounter *V;                      // vector of HostCounter elements

	/* default constructor */
	HostAgeBandCounter();

	~HostAgeBandCounter();

	/* returns host group ID */
	unsigned short GetGroupID(unsigned short ageBandIndx);

	/* resets host counters */
	void Reset();
};

unsigned short HostAgeBandCounter::NumAgeBands = 0;


/* default constructor */
HostAgeBandCounter::HostCounter::HostCounter() : CrntGroupID((unsigned short)UNDEFINED), GroupSize(0), NumGroups(0), CrntGroupSize(0), CrntNumGroups(0), NumHosts(0)
{
}


/* calculates number of groups */
void HostAgeBandCounter::HostCounter::SetGroupSize(unsigned int groupSize)
{
	GroupSize = groupSize;

	unsigned int numGroups = NumHosts / GroupSize;
	unsigned int rem = NumHosts % GroupSize;
	if( rem >= GroupSize / 2 )
		numGroups++;
	if( numGroups == 0 )
		numGroups++;

	NumGroups = numGroups;
}


/* default constructor */
HostAgeBandCounter::HostAgeBandCounter() : MaxGroupID(0)
{
	try
	{
		V = new HostAgeBandCounter::HostCounter[NumAgeBands];
	}
	catch(...)
	{
		cerr << "*** Error allocating array of HostAgeBandCounter::HostCounter elements (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
}


HostAgeBandCounter::~HostAgeBandCounter()
{
	delete [] V;
}


/* returns host group ID */
unsigned short HostAgeBandCounter::GetGroupID(unsigned short ageBandIndx)
{
	if( (V[ageBandIndx].CrntGroupSize != 0) && ((V[ageBandIndx].CrntGroupSize < V[ageBandIndx].GroupSize) || (V[ageBandIndx].CrntNumGroups == V[ageBandIndx].NumGroups)) )
	{
		V[ageBandIndx].CrntGroupSize++;
		return V[ageBandIndx].CrntGroupID;
	}
	else
	{
		V[ageBandIndx].CrntGroupSize = 1;
		V[ageBandIndx].CrntNumGroups++;
		V[ageBandIndx].CrntGroupID = MaxGroupID++;
		return V[ageBandIndx].CrntGroupID;
	}
}


/* resets host counters */
void HostAgeBandCounter::Reset()
{
	MaxGroupID = 0;
	for(unsigned short i=0; i < NumAgeBands; i++)
	{
		V[i].CrntGroupID = (unsigned short)UNDEFINED;
		V[i].GroupSize = 0;
		V[i].NumGroups = 0;
		V[i].CrntGroupSize = 0;
		V[i].CrntNumGroups = 0;
		V[i].NumHosts = 0;
	}
}



/* creates groups of hosts within establishments for current age group */
/* pHostTbl: pointer to the table of hosts, pESDV: pointer to the establishment size distribution vector, pEstTbl: pointer to the table of establishments */
void Country::HostToEstablishmentMap::CreateHostGroups(Country::HostTbl *pHostTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CreateHostGroups(Country::EstablishmentTbl*, Country::HostTbl*) started...\n";
#endif

	CreateHostToEstIndexTbl(pEstTbl, pHostTbl);                                                // create Establishment->Host index table

	float ageLB = PHETDKernel->GetCrntLowerAgeGroupBoundary();
	float ageUB = PHETDKernel->GetCrntUpperAgeGroupBoundary();

	float ageBand;
	unsigned short hostAgeGroupIndx = PHETDKernel->GetCrntAgeGroupIndx();
	unsigned short staffAgeGroupIndx = pESDV->GetStaffAgeGroupIndx();
	if( hostAgeGroupIndx != staffAgeGroupIndx )
		ageBand = SCHOOL_HOST_GROUP_AGE_BAND;
	else
		ageBand = ageUB - ageLB;

	HostAgeBandCounter::NumAgeBands = ceil((ageUB - ageLB) / ageBand);

	unsigned int maxNumThreads = omp_get_max_threads();

	HostAgeBandCounter *HABCntr = new HostAgeBandCounter[maxNumThreads];
	Country::Host *Host = new Country::Host[maxNumThreads];

	unsigned int numIndexEntries = PHEIndxTbl->NumElsMax;
	#pragma omp parallel for
	for(unsigned int i=0; i < numIndexEntries; i++)
	{
		unsigned int numHostIndxs = PHEIndxTbl->Table[i].NumEls;
		if( numHostIndxs == 0 )
			continue;

		for(unsigned int j=0; j < numHostIndxs; j++)                                            // count hosts within each age band
		{
			pHostTbl->GetHostRec(PHEIndxTbl->Table[i].Indx[j], &Host[omp_get_thread_num()]);
			unsigned int ageBandIndx = floor((Host[omp_get_thread_num()].Age - ageLB) / ageBand);
			HABCntr[omp_get_thread_num()].V[ageBandIndx].NumHosts++;
		}

		for(unsigned short j=0; j < HostAgeBandCounter::NumAgeBands; j++)                      // assign host group sizes and calculate number of groups
		{
			if( HABCntr[omp_get_thread_num()].V[j].NumHosts == 0 )
				continue;
			HABCntr[omp_get_thread_num()].V[j].SetGroupSize(pESDV->GetAvGroupSize());
		}

		for(unsigned int j=0; j < numHostIndxs; j++)                                           // assign host group IDs
		{
			unsigned int rndHostIndx = PHEIndxTbl->Table[i].ExtractRndIndex();
			pHostTbl->GetHostRec(rndHostIndx, &Host[omp_get_thread_num()]);
			unsigned int ageBandIndx = floor((Host[omp_get_thread_num()].Age - ageLB) / ageBand);
			unsigned short groupID = HABCntr[omp_get_thread_num()].GetGroupID(ageBandIndx);
			pHostTbl->SetHostGroupID(rndHostIndx, groupID);
		}
		
		HABCntr[omp_get_thread_num()].Reset();
	}

	delete [] HABCntr;
	delete [] Host;

	DeleteHostToEstIndexTbl();

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CreateHostGroups(Country::EstablishmentTbl*, Country::HostTbl*) completed.\n";
#endif
}



/* computes model host-to-establishment travel statistics */
/* pHostTbl: pointer to the table of hosts; pHshldTbl: pointer to the household table; pEstTbl: pointer to the table of establishments */
HETDVec* Country::HostToEstablishmentMap::CompHostToEstablishmentTravelStat(Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, Country::EstablishmentTbl *pEstTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CompHostToEstablshmntTravelStat(Country::HostTbl*, Country::HouseholdTbl*, Country::EstablishmentTbl*) started...\n";
#endif

	Household *pHshld = new Household;
	Host *pHostList = new Host[pHshldTbl->GetMaxHshldSize()];
	Establishment *pEst = new Establishment;

	float maxTravleDist = 0.0f;

	for(unsigned int i=0; i < pHshldTbl->GetNumHouseholds(); i++)   // find the maximum travel distance
	{
		pHshldTbl->GetHousehold(i, pHshld);
		pHostTbl->GetHostList(pHshld, pHostList);

		for(unsigned short j=0; j < pHshld->NumRecs; j++)
		{
			unsigned int estID = pHostList[j].EstID.ID2;
			if( estID == (unsigned int)UNDEFINED )
				continue;
			pEstTbl->GetEstablishmentRec(estID, pEst);
			double r = CntryLndScnCellTbl::PointDist(pHshld->Lat, pHshld->Lon, pEst->Lat, pEst->Lon);
			if( maxTravleDist < r )
				maxTravleDist = r;
		}
	}

	unsigned int numEls = (unsigned int)ceil(maxTravleDist/TRAVEL_DIST_BAND);

	HETDVec* pMdHETDVec = new HETDVec(numEls);

	unsigned int numHosts=0;

	for(unsigned int i=0; i < pHshldTbl->GetNumHouseholds(); i++)   // compute host-to-establishment travel distance distribution
	{
		pHshldTbl->GetHousehold(i, pHshld);
		pHostTbl->GetHostList(pHshld, pHostList);

		for(unsigned short j=0; j < pHshld->NumRecs; j++)
		{
			unsigned int estID = pHostList[j].EstID.ID2;
			if( estID == (unsigned int)UNDEFINED )
				continue;
			pEstTbl->GetEstablishmentRec(estID, pEst);
			double r = CntryLndScnCellTbl::PointDist(pHshld->Lat, pHshld->Lon, pEst->Lat, pEst->Lon);
			unsigned int indx = (unsigned int)(ceil(r/TRAVEL_DIST_BAND))-1;
			pMdHETDVec->V[indx].NumHosts++;
			numHosts++;
		}
	}

	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		pMdHETDVec->V[i].Prob = (double)pMdHETDVec->V[i].NumHosts/(double)numHosts;

	delete pHshld;
	delete [] pHostList;
	delete pEst;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CompHostToEstablshmntTravelStat(Country::HostTbl*, Country::HouseholdTbl*, Country::EstablishmentTbl*) completed.\n\n";
#endif

	return pMdHETDVec;
}



/* computes model host-to-establishment travel statistics for specific age group */
/* pHostTbl: pointer to the table of hosts; pHshldTbl: pointer to the household table; pEstTbl: pointer to the table of establishments; */
/* pHETDKernel: pointer to host-to-establishment travel distribution kernel; ageGroupIndx: age group index */
HETDVec* Country::HostToEstablishmentMap::CompHostToEstablishmentTravelStat(Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel, unsigned short ageGroupIndx)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CompHostToEstablshmntTravelStat(Country::HostTbl*, Country::HouseholdTbl*, Country::EstablishmentTbl*, HostToEstTravelDistrKernel*, unsigned short) started...\n";
#endif

	Household *pHshld = new Household;
	Host *pHostList = new Host[pHshldTbl->GetMaxHshldSize()];
	Establishment *pEst = new Establishment;

	float maxTravleDist = 0.0f;

	for(unsigned int i=0; i < pHshldTbl->GetNumHouseholds(); i++)   // find the maximum travel distance
	{
		pHshldTbl->GetHousehold(i, pHshld);
		pHostTbl->GetHostList(pHshld, pHostList);

		for(unsigned short j=0; j < pHshld->NumRecs; j++)
		{
			unsigned int estID = pHostList[j].EstID.ID2;
			if( estID == (unsigned int)UNDEFINED )
				continue;
			if( pHostList[j].EstID.ID1 != pHETDKernel->GetCrntAgeGroupIndx() )
				continue;
			if( pHETDKernel->GetAgeGroupIndx(pHostList[j].Age) != ageGroupIndx )
				continue;
			pEstTbl->GetEstablishmentRec(estID, pEst);
			double r = CntryLndScnCellTbl::PointDist(pHshld->Lat, pHshld->Lon, pEst->Lat, pEst->Lon);
			if( maxTravleDist < r )
				maxTravleDist = r;
		}
	}

	unsigned int numEls = (unsigned int)ceil(maxTravleDist/TRAVEL_DIST_BAND);

	HETDVec* pMdHETDVec = new HETDVec(numEls);

	unsigned int numHostsTotal=0, numHosts=0;

	for(unsigned int i=0; i < pHshldTbl->GetNumHouseholds(); i++)   // compute host-to-establishment travel distance distribution
	{
		pHshldTbl->GetHousehold(i, pHshld);
		pHostTbl->GetHostList(pHshld, pHostList);

		for(unsigned short j=0; j < pHshld->NumRecs; j++)
		{
			if( pHostList[j].EstID.ID1 != pHETDKernel->GetCrntAgeGroupIndx() )
				continue;
			if( pHETDKernel->GetAgeGroupIndx(pHostList[j].Age) != ageGroupIndx )
				continue;
			numHostsTotal++;   // count total number of hosts in specific age group
			unsigned int estID = pHostList[j].EstID.ID2;
			if( estID == (unsigned int)UNDEFINED )
				continue;
			
			pEstTbl->GetEstablishmentRec(estID, pEst);
			double r = CntryLndScnCellTbl::PointDist(pHshld->Lat, pHshld->Lon, pEst->Lat, pEst->Lon);
			unsigned int indx = (unsigned int)(ceil(r/TRAVEL_DIST_BAND))-1;
			pMdHETDVec->V[indx].NumHosts++;
			numHosts++;        // count number of hosts in specific age group associated with establishments
		}
	}

	pMdHETDVec->NumHostsTotal = numHostsTotal;
	pMdHETDVec->NumHosts = numHosts;

	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		pMdHETDVec->V[i].Prob = (double)pMdHETDVec->V[i].NumHosts/(double)numHosts;

	delete pHshld;
	delete [] pHostList;
	delete pEst;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostToEstablishmentMap::CompHostToEstablshmntTravelStat(Country::HostTbl*, Country::HouseholdTbl*, Country::EstablishmentTbl*, HostToEstTravelDistrKernel*, unsigned short) completed.\n\n";
#endif

	return pMdHETDVec;
}



/* creates host-to-establishment map */
void Country::CreateHostToEstablishmentMap()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateHostToEstablishmentMap() started...\n";
#endif

	if( PHstToEstMap != NULL )
		delete PHstToEstMap;

	PHstToEstMap = new HostToEstablishmentMap(PCLSCTbl, HOST_TO_EST_MAP_GEOPATCH_GRANURLTY, PEstTbl, PHETDKernel);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateHostToEstablishmentMap() completed.\n\n";
#endif
}



/* assigns hosts to establishments */
void Country::AssignHostsToEstablishments()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::AssignHostsToEstablishments() started...\n";
#endif

	PHstToEstMap->AssignHostsToEstablishments(PCLSCTbl, PHostTbl, PHshldTbl, PESDV, PEstTbl);

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Total number of establishments generated: " << PEstTbl->GetNumEstablishments() << "\n";
	cout << "Fill ratio: " << PESDV->GetFillRatio() << "\n";
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::AssignHostsToEstablishments() completed.\n\n";
#endif
}



/* creates staff-to-establishment map */
void Country::CreateStaffToEstablishmentMap()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateStaffToEstablishmentMap() started...\n";
#endif

	if( PHETDKernel->GetCrntAgeGroupIndx() == PESDV->GetStaffAgeGroupIndx() )
		return;

	if( PHstToEstMap != NULL )
		delete PHstToEstMap;

	PHstToEstMap = new HostToEstablishmentMap(PCLSCTbl, HOST_TO_EST_MAP_GEOPATCH_GRANURLTY, PESDV, PEstTbl, PHETDKernel);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateStaffToEstablishmentMap() completed.\n\n";
#endif
}



/* assigns staff to establishments */
void Country::AssignStaffToEstablishments()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::AssignStaffToEstablishments() started...\n";
#endif

	if( PHETDKernel->GetCrntAgeGroupIndx() == PESDV->GetStaffAgeGroupIndx() )
		return;

	PHstToEstMap->AssignStaffToEstablishments(PCLSCTbl, PHostTbl, PHshldTbl, PESDV, PEstTbl);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::AssignStaffToEstablishments() completed.\n\n";
#endif
}



/* creates groups of hosts within establishments for current age group */
void Country::CreateHostGroups()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateHostGroups() started...\n";
#endif

	PHstToEstMap->CreateHostGroups(PHostTbl, PESDV, PEstTbl);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CreateHostGroups() completed.\n\n";
#endif
}



/* resets establishment IDs assigned to hosts */
void Country::ResetHostEstIDs()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ResetHostEstIDs() started...\n";
#endif

	PHostTbl->ResetEstIDs();

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ResetHostEstIDs() completed.\n\n";
#endif
}



/* computes and outputs model establishment size distribution */
void Country::CompModelEstablishmentSizeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelEstSizeDistr() started...\n";
#endif

	MdESDVec* pMdESDVec = PEstTbl->CompModelEstSizeDistr(PESDV);

#ifdef CON_OUTPUT
	cout << "Model establishment size distribution:\n";
	for(unsigned int i=0; i < pMdESDVec->NumEls; i++)
		cout << pMdESDVec->V[i].Size << "\t" << pMdESDVec->V[i].Prob << "\t" << pMdESDVec->V[i].NumEst << "\n";
	cout << "\n";
#endif

	delete pMdESDVec;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelEstSizeDistr() completed.\n\n";
#endif
}



/* computes and outputs model host-to-establishment travel statistics */
void Country::CompModelHostToEstablishmentTravelStat()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHostToEstablshmntTravelStat() started...\n";
#endif

	HETDVec* pMdHETDVec = PHstToEstMap->CompHostToEstablishmentTravelStat(PHostTbl, PHshldTbl, PEstTbl);

#ifdef CON_OUTPUT
	cout << "\n---------------------------------------------------------------------------------\n";
	cout << "Kernel function code: " << PHETDKernel->GetKernelFunctionCode() << "\n";
	cout << "Parameter set index: " << PHETDKernel->GetCrntParamSetIndx() << "\n";
	cout << "a0 = " << PHETDKernel->GetParamA0() << ",\tb0 = " << PHETDKernel->GetParamB0() << \
		",\ta1 = " << PHETDKernel->GetParamA1() << ",\tb1 = " << PHETDKernel->GetParamB1() << ",\tc1 = " << PHETDKernel->GetParamC1() << "\n";
	cout << "Model host-to-establishment travel distribution vector:\n";
	cout << "Distance (km)\t" << "Probability\t" << "Number of hosts\n";
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		cout << pMdHETDVec->V[i].Dist << "\t" << pMdHETDVec->V[i].Prob << "\t" << pMdHETDVec->V[i].NumHosts << "\n";
	cout << "\n";
#endif

	HETDVec *pDtHETDVec = new HETDVec(pMdHETDVec->NumEls, PHETDKernel, PHETDKernel->F_rw());

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Approximation of the real world statistics and deviation of the model statistics from the data:\n";
	cout << "Distance (km)\t" << "Probability\t" << "Difference between the data and model\n";
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		cout << pDtHETDVec->V[i].Dist << "\t" << pDtHETDVec->V[i].Prob << "\t" << pDtHETDVec->V[i].Prob - pMdHETDVec->V[i].Prob << "\n";
	cout << "\n";
#endif

	/*
	double maxAbsDeviation = 0.0, maxRelDeviation = 0.0;
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
	{
		double deltaAbs = abs(pDtHETDVec->V[i].Prob - pMdHETDVec->V[i].Prob);
		if( maxAbsDeviation < deltaAbs )
			maxAbsDeviation = deltaAbs;

		double deltaRel = deltaAbs/pDtHETDVec->V[i].Prob;
		if( maxRelDeviation < deltaRel )
			maxRelDeviation = deltaRel;
	}
#ifdef CON_OUTPUT_
	cout << "Maximum absolute deviation of the model statistics from the data: " << maxAbsDeviation << "\n";
	cout << "Maximum relative deviation of the model statistics from the data: " << maxRelDeviation << "\n";
	cout << "\n";
#endif
	*/

	delete pDtHETDVec;
	delete pMdHETDVec;

#ifdef OUTPUT_LOG_TEST_
	cout << "Establishments with vacancies remaining unfilled:\n";
	for(unsigned int i=0; i < PEstTbl->GetNumEstablishments(); i++)
	{
		Establishment est;
		PEstTbl->GetEstablishment(i, &est);

		if( est.NumHosts < est.NumVacancs )
			cout << est.LndScnX << "\t" << est.LndScnY << "\t" << est.NumVacancs << "\t" << est.NumHosts << "\n";
	}
	cout << "\n";
#endif

#ifdef CON_OUTPUT
	cout << "---------------------------------------------------------------------------------\n\n";
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHostToEstablshmntTravelStat() completed.\n\n";
#endif
}



/* computes and outputs model host-to-establishment travel statistics for current age group */
void Country::CompModelHostToEstablishmentTravelStatAG()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHostToEstablishmentTravelStatAG() started...\n";
#endif

	HETDVec* pMdHETDVec = PHstToEstMap->CompHostToEstablishmentTravelStat(PHostTbl, PHshldTbl, PEstTbl, PHETDKernel, PHETDKernel->GetCrntAgeGroupIndx());

#ifdef CON_OUTPUT
	cout << "\n---------------------------------------------------------------------------------\n";
	cout << "Kernel function code: " << PHETDKernel->GetKernelFunctionCode() << "\n";
	cout << "Age group index: " << PHETDKernel->GetCrntAgeGroupIndx() << " (" << PHETDKernel->GetCrntLowerAgeGroupBoundary() << "-" << PHETDKernel->GetCrntUpperAgeGroupBoundary() << " years)\n";
	cout << "Parameter set index: " << PHETDKernel->GetCrntParamSetIndx() << "\n";
	cout << "a0 = " << PHETDKernel->GetParamA0() << ",\tb0 = " << PHETDKernel->GetParamB0() << \
		",\ta1 = " << PHETDKernel->GetParamA1() << ",\tb1 = " << PHETDKernel->GetParamB1() << ",\tc1 = " << PHETDKernel->GetParamC1() << "\n";
	cout << "Total number of hosts within given age group: " << pMdHETDVec->NumHostsTotal << "\n";
	cout << "Number of hosts associated with establishments: " << pMdHETDVec->NumHosts << "\n";
	if( pMdHETDVec->NumHosts != 0 )
		cout << "Model fill ratio: " << (float)pMdHETDVec->NumHosts / (float)pMdHETDVec->NumHostsTotal << "\n";
	cout << "Model host-to-establishment travel distribution vector:\n";
	cout << "Distance (km)\t" << "Probability\t" << "Number of hosts\n";
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		cout << pMdHETDVec->V[i].Dist << "\t" << pMdHETDVec->V[i].Prob << "\t" << pMdHETDVec->V[i].NumHosts << "\n";
	cout << "\n";
#endif

	HETDVec *pDtHETDVec = new HETDVec(pMdHETDVec->NumEls, PHETDKernel, PHETDKernel->F_rw());

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Approximation of the real world statistics and deviation of the model statistics from the data:\n";
	cout << "Distance (km)\t" << "Probability\t" << "Difference between the data and model\n";
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
		cout << pDtHETDVec->V[i].Dist << "\t" << pDtHETDVec->V[i].Prob << "\t" << pDtHETDVec->V[i].Prob - pMdHETDVec->V[i].Prob << "\n";
	cout << "\n";
#endif

	/*
	double maxAbsDeviation = 0.0, maxRelDeviation = 0.0;
	for(unsigned int i=0; i < pMdHETDVec->NumEls; i++)
	{
		double deltaAbs = abs(pDtHETDVec->V[i].Prob - pMdHETDVec->V[i].Prob);
		if( maxAbsDeviation < deltaAbs )
			maxAbsDeviation = deltaAbs;

		double deltaRel = deltaAbs/pDtHETDVec->V[i].Prob;
		if( maxRelDeviation < deltaRel )
			maxRelDeviation = deltaRel;
	}
#ifdef CON_OUTPUT_
	cout << "Maximum absolute deviation of the model statistics from the data: " << maxAbsDeviation << "\n";
	cout << "Maximum relative deviation of the model statistics from the data: " << maxRelDeviation << "\n";
	cout << "\n";
#endif
	*/

	delete pDtHETDVec;
	delete pMdHETDVec;

#ifdef OUTPUT_LOG_TEST_
	cout << "Establishments with vacancies remaining unfilled:\n";
	for(unsigned int i=0; i < PEstTbl->GetNumEstablishments(); i++)
	{
		Establishment est;
		PEstTbl->GetEstablishment(i, &est);

		if( est.NumHosts < est.NumVacancs )
			cout << est.LndScnX << "\t" << est.LndScnY << "\t" << est.NumVacancs << "\t" << est.NumHosts << "\n";
	}
	cout << "\n";
#endif

#ifdef CON_OUTPUT
	cout << "---------------------------------------------------------------------------------\n\n";
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHostToEstablishmentTravelStatAG() completed.\n\n";
#endif
}



/* computes and outputs model host-to-establishment travel statistics for "staff" age group */
void Country::CompModelStaffToEstablishmentTravelStatAG()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelStaffToEstablishmentTravelStatAG() started...\n";
#endif

	if( PHETDKernel->GetCrntAgeGroupIndx() == PESDV->GetStaffAgeGroupIndx() )
		return;

	HETDVec* pMdSETDVec = PHstToEstMap->CompHostToEstablishmentTravelStat(PHostTbl, PHshldTbl, PEstTbl, PHETDKernel, PESDV->GetStaffAgeGroupIndx());

	cout << "Total number of staff members associated with the given establishment type: " << pMdSETDVec->NumHosts << "\n";
	cout << "Model staff-to-establishment travel distribution vector:\n";
	cout << "Distance (km)\t" << "Probability\t" << "Number of staff\n";
	for(unsigned int i=0; i < pMdSETDVec->NumEls; i++)
		cout << pMdSETDVec->V[i].Dist << "\t" << pMdSETDVec->V[i].Prob << "\t" << pMdSETDVec->V[i].NumHosts << "\n";
	cout << "\n";

	delete pMdSETDVec;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelStaffToEstablishmentTravelStatAG() completed.\n";
#endif
}



/* outputs table of synthesized establishments */
/* fName: name of file for output */
void Country::OutputSynthEstablishments(char *fNameTmpl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::OutputSynEstablshmnts(char*) started...\n";
#endif

	if( !PHETDKernel->GetOutputFlag() )
	{
#ifdef OUTPUT_LOG_L1
	cout << "*** Output flag has not been set for this parameter set, so no output will be produced.\n*** Country::OutputSynthEstablshmnts(char*) completed.\n\n";
#endif
		return;
	}

	char *fName = new char[FN_BUF_LNGTH];
	char *fNameSfx = new char[FN_BUF_LNGTH];
	char *sfx = new char[FN_BUF_LNGTH];
	
	strcpy(fName, fNameTmpl);

	itoa(PHETDKernel->GetCrntAgeGroupIndx(), sfx, 10);
	if( (strlen(sfx) + 3) >= FN_BUF_LNGTH )
	{
		cout << "Synthesized population file name suffix is too long, no output will be produced.\n";		
		delete [] fName;
		delete [] fNameSfx;
		delete [] sfx;
#ifdef OUTPUT_LOG_L1
		cout << "*** Country::OutputSynthEstablishments(char*) completed.\n\n";
#endif
		return;
	}
	strcpy(fNameSfx, "_");
	strcat(fNameSfx, sfx);
	strcat(fNameSfx, "__");

	itoa(PHETDKernel->GetCrntParamSetIndx(), sfx, 10);
	if( strlen(fNameSfx) + strlen(sfx) >= FN_BUF_LNGTH )
	{
		cout << "Synthesized population file name suffix is too long, no output will be produced.\n";		
		delete [] fName;
		delete [] fNameSfx;
		delete [] sfx;
#ifdef OUTPUT_LOG_L1
		cout << "*** Country::OutputSynthEstablishments(char*) completed.\n\n";
#endif
		return;
	}
	strcat(fNameSfx, sfx);

	if( (strlen(fName) + strlen(fNameSfx)) >= FN_BUF_LNGTH )
	{
		cout << "Synthesized population file name is too long, no output will be produced.\n";		
		delete [] fName;
		delete [] fNameSfx;
		delete [] sfx;
#ifdef OUTPUT_LOG_L1
		cout << "*** Country::OutputSynthEstablishments(char*) completed.\n\n";
#endif
		return;
	}
	strcat(fName, fNameSfx);	

#ifdef OUTPUT_LOG_L1
	cout << "Table of synthesized establishments will be written into the file " << fName << "\n";
#endif

	fstream outF(fName, ios::out | ios::trunc | ios::binary);
	if( !outF )
		throw FILE_OPEN_ERROR;

	Establishment *pEst = new Establishment;

	unsigned int numEstablishments = PEstTbl->GetNumEstablishments();
	unsigned int dummy = (unsigned int)UNDEFINED;                       // dummy field (undefined data)
	unsigned short ageGroupIndx = PHETDKernel->GetCrntAgeGroupIndx();
	outF.write((char*)&ageGroupIndx, sizeof(ageGroupIndx));
	outF.write((char*)&numEstablishments, sizeof(numEstablishments));
	for(unsigned int i=0; i < numEstablishments; i++)
	{
		PEstTbl->GetEstablishmentRec(i, pEst);
		outF.write((char*)&(pEst->Lat), sizeof(pEst->Lat));
		outF.write((char*)&(pEst->Lon), sizeof(pEst->Lon));
		outF.write((char*)&(pEst->LndScnX), sizeof(pEst->LndScnX));
		outF.write((char*)&(pEst->LndScnY), sizeof(pEst->LndScnY));
		outF.write((char*)&(pEst->NumHosts), sizeof(pEst->NumHosts));
		outF.write((char*)&(pEst->NumStaff), sizeof(pEst->NumStaff));
		//outF.write((char*)&(pEst->NumStaffVacancs), sizeof(pEst->NumStaffVacancs));   // these two records are not really needed...
		//outF.write((char*)&(pEst->NumHostVacancs), sizeof(pEst->NumHostVacancs));
		outF.write((char*)&dummy, sizeof(dummy));
	}

	delete pEst;

	delete [] fName;
	delete [] fNameSfx;
	delete [] sfx;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::OutputSynthEstablishments(char*) completed.\n\n";
#endif
}

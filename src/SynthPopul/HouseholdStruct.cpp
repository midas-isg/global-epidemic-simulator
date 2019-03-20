/* HouseholdStruct.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "PopulDistr.h"
#include "EstablishmentStruct.h"
#include "HouseholdStruct.h"
#ifdef _OPENMP
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#endif



#define DEFAULT_TBL_SIZE 16
#define DEFAULT_EXT_COEFF 0.5



/* default constructor; creates undefined matrix */
Country::MdADVMtrx::MdADVMtrx() : NumEls(0), M(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::MdADVMtrx() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::MdADVMtrx() completed\n\n";
#endif
}



/* constructor; builds empty matrix */
/* NumMdADVec: number of age distribution vectors; MdADVecLngth: length of each vector */
Country::MdADVMtrx::MdADVMtrx(unsigned short numMdADVec, unsigned short MdADVecLngth) : NumEls(numMdADVec)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::MdADVMtrx(unsigned short, unsigned short) started...\n";
#endif

	try
	{
		 M = new MdADVec[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (M) of Country::MdADVMtrx::MdADVec objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		M[i].NumEls = MdADVecLngth;
		try
		{
			M[i].V = new MdADVec::MdADVEl[M[i].NumEls];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (M) of Country::MdADVMtrx::MdADVec::MdADVEl objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(unsigned short j=0; j < M[i].NumEls; j++)
		{
			M[i].V[j].Age = (j+1)*AGE_BAND;
			M[i].V[j].Prob = 0.0f;
			M[i].V[j].NumHosts = 0;
		}
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::MdADVMtrx(unsigned short, unsigned short) completed\n";
#endif
}



Country::MdADVMtrx::~MdADVMtrx()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::~MdADVMtrx() started...\n";
#endif

	if( M != NULL )
	{
		for(unsigned short i=0; i < NumEls; i++)
			delete [] M[i].V;
		delete [] M;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdADVMtrx::~MdADVMtrx() completed\n\n";
#endif
}



/* default constructor; creates undefined vector */
Country::MdHSDVec::MdHSDVec() : NumEls(0), V(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdSDVec::MdSDVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdSDVec::MdSDVec() completed\n\n";
#endif
}



/* constructor; builds empty household size distribution vector */
/* MSDVecLngth: length of vector */
Country::MdHSDVec::MdHSDVec(unsigned short MdSDVecLngth) : NumEls(MdSDVecLngth)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdSDVec::MdSDVec(unsigned short) started...\n";
#endif

	try
	{
		 V =  new MdHSDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of Country::MdSDVec::MdSDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		V[i].Size = i+1;
		V[i].Prob = 0;
		V[i].NumHshlds = 0;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::MdSDVec::MdSDVec(unsigned short) completed.\n\n";
#endif
}


Country::MdHSDVec::~MdHSDVec()
{
#ifdef OUTPUT_LOG_L1
	cout << "Country::MdSDVec::~MdSDVec() started...\n";
#endif

	if( V != NULL )
		delete [] V;

#ifdef OUTPUT_LOG_L1
	cout << "Country::MdSDVec::~MdSDVec() completed.\n\n";
#endif
}



/* constructor; builds redundancy and deficiency age group distribution vectors */
/* pMADDev: pointer to probability deviation vector */
Country::ADDevVec::ADDevVec(MADVec *pMADDev)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ADDevVec::ADDevVec() started...\n";
#endif

	unsigned short i, rj, dj;

	MADNumEls = pMADDev->NumEls;
	try
	{
		 MADV =  new MADDevEl[MADNumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (MADV) of Country::ADDevVec::MADDevEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(i=0; i < MADNumEls; i++)
	{
		MADV[i].Age = pMADDev->V[i].Age;
		MADV[i].DevProb = pMADDev->V[i].Prob;
	}

	for(i = rj = dj =0; i < MADNumEls; i++)
	{
		if( MADV[i].DevProb > 0.0f )
			rj++;
		else
			dj++;
	}
	RedNumEls = rj + 1 ;
	DefNumEls = dj + 1;
	try
	{
		 RedV =  new ADDevEl[RedNumEls];
		 DefV =  new ADDevEl[DefNumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating arrays (RedV & DefV) of Country::ADDevVec::ADDevEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	AgeBand = MADV[1].Age - MADV[0].Age;

	RedV[0].Age = DefV[0].Age = 0.0f;
	RedV[0].CProb = DefV[0].CProb = 0.0f;
	for(i=0, rj=1, dj=1; i < MADNumEls; i++)
	{
		if( MADV[i].DevProb > 0.0f )
		{
			RedV[rj].Age = MADV[i].Age;
			RedV[rj].CProb = RedV[rj-1].CProb + MADV[i].DevProb;
			rj++;
		}
		else
		{
			DefV[dj].Age = MADV[i].Age;
			DefV[dj].CProb = DefV[dj-1].CProb + ABS(MADV[i].DevProb);
			dj++;
		}
	}	

	/* normalize obtained cumulative distributions */
	if( RedV[RedNumEls-1].CProb < TOLER_VAL || ABS(RedV[RedNumEls-1].CProb - DefV[DefNumEls-1].CProb) > TOLER_VAL )
		throw INCORRECT_DISTR_VECTOR;
	NormCoeff = DefV[DefNumEls-1].CProb;
	for(i=0; i < RedNumEls-1; i++)
		RedV[i].CProb /= RedV[RedNumEls-1].CProb;
	RedV[RedNumEls-1].CProb = 1.0f + TOLER_VAL;
	for(i=0; i < DefNumEls-1; i++)
		DefV[i].CProb /= DefV[DefNumEls-1].CProb;
	DefV[DefNumEls-1].CProb = 1.0f + TOLER_VAL;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ADDevVec::ADDevVec() completed.\n\n";
#endif
}



Country::ADDevVec::~ADDevVec()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ADDevVec::~ADDevVec() started...\n";
#endif

	if( MADV != NULL )
		delete [] MADV;
	if( RedV != NULL )
		delete [] RedV;
	if( DefV != NULL )
		delete [] DefV;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::ADDevVec::~ADDevVec() completed.\n\n";
#endif
}



/* corrects for marginal age distribution deviation */
/* hst: reference to host object */
void Country::ADDevVec::CorrectDev(Country::Host &hst)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ADDevVec::CorrectDev(Host*) started...\n";
#endif

	unsigned short indx = (unsigned short)(ceil(hst.Age/AgeBand))-1;
	float rnd, age;
	// use "redundancy" population age group for correction
	if( MADV[indx].DevProb < 0.0f )
		return;

	rnd = ranf_();
	if( rnd > MADV[indx].DevProb/NormCoeff )
		return;

	// correct age here using distribution for "deficiency" age group
	rnd = ranf_();
	for(unsigned short i=1; i < DefNumEls; i++)
	{
		if( (DefV[i-1].CProb < rnd) && (DefV[i].CProb >= rnd) )
		{
			do
			{
				age = DefV[i].Age - AgeBand * ranf_();
			}
			while( (age == DefV[i].Age - AgeBand) || (age == DefV[i].Age) );
			hst.Age = age;
			return;
		}
	}
	throw INCORRECT_DISTR_VECTOR;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ADDevVec::CorrectDev(Host*) completed.\n\n";
#endif
}



/* corrects for marginal age distribution deviation - better than previous version as some simple restrictions on householdsize-host's age correlation applied */
/* hshld: reference to household object; pADVM: pointer to matrix of age distribution vectors */
void Country::ADDevVec::CorrectDev(Country::Household &hshld, ADVMtrx *pADVM)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ADDevVec::CorrectDev(Country::Household&, ADVMtrx*) started...\n";
#endif

	for(unsigned short i=0; i < hshld.NumRecs; i++)
	{
		unsigned short indx = (unsigned short)(ceil(hshld.H[i].Age/AgeBand))-1;
		float rnd, age;

		// use "redundancy" population age group for correction
		if( MADV[indx].DevProb < 0.0f )
			continue;
		
		rnd = ranf_();
		if( rnd > MADV[indx].DevProb/NormCoeff )
			continue;

		// correct age here using distribution for "deficiency" age group
		hshld.H[i].Age = (float)UNDEFINED;
		do
		{
			rnd = ranf_();
			for(unsigned short j=1; j < DefNumEls; j++)
			{
				if( (DefV[j-1].CProb < rnd) && (DefV[j].CProb >= rnd) )
				{
					do
					{
						age = DefV[j].Age - AgeBand * ranf_();
					}
					while( (age == DefV[j].Age - AgeBand) || (age == DefV[j].Age) );
					hshld.H[i].Age = age;
					break;
				}
			}
			if( hshld.H[i].Age == (float)UNDEFINED )
				throw INCORRECT_DISTR_VECTOR;
		}
		while( pADVM->GetProb(hshld.NumRecs, age) < CORR_PROB_LIMIT );
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::ADDevVec::CorrectDev(Country::Household&, ADVMtrx*) completed.\n\n";
#endif
}



/* default constructor; assigns undefined IDs */
Country::Host::EstablishmentID::EstablishmentID() : ID1((unsigned short)UNDEFINED), ID2((unsigned int)UNDEFINED), GroupID((unsigned short)UNDEFINED)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Host::EstablishmentID::EstablishmentID() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Host::EstablishmentID::EstablishmentID() completed\n\n";
#endif
}



/* default constructor; creates empty Host object */
Country::Host::Host() : Age(0.0f)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Host::Host() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Host::Host() completed\n\n";
#endif
}



/* default constructor; creates undefined table */
Country::HostTbl::HostTbl() : NumRecs(0), NumRecsMax(0), Table(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::HostTblCl() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::HostTblCl() completed.\n\n";
#endif
}



/* constructor; creates empty table */
/* tblSize: size of Table */
Country::HostTbl::HostTbl(unsigned int tblSize) : NumRecs(0), NumRecsMax(tblSize)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::HostTblCl(unsigned int) started...\n";
#endif

	try
	{
		Table = new Host[NumRecsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of Country::Host objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::HostTblCl(unsigned int) completed.\n\n";
#endif
}



Country::HostTbl::~HostTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::~HostTblCl() started...\n";
#endif

	if( Table != NULL )
		delete [] Table;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::~HostTblCl() completed.\n";
#endif
}



/* reserves undefined host records in Table */
/* numHosts: number of host records */
Country::Host* Country::HostTbl::ReserveHostRecs(unsigned int numHosts)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::ReserveHostRecs(unsigned int) started...\n";
#endif

	if(Table == NULL)
		throw TABLE_UNDEFINED;             // Table is undefined, throw an exception
	if(NumRecs + numHosts > NumRecsMax)
		throw TABLE_TOO_SMALL;             // Table is full - throw an exception
	
	unsigned int indx = NumRecs;
	NumRecs += numHosts;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::ReserveHostRecs(unsigned int) completed\n\n";
#endif

	return &Table[indx];
}



/* sets host record (just age so far) */
/* hostRecIndx: index in Table; age: host's age */
void Country::HostTbl::SetHostRec(unsigned int hostRecIndx, float age)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::SetHostRec(unsigned int, float) started...\n";
#endif

	Table[hostRecIndx].Age = age;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::SetHostRec(unsigned int, float) completed.\n\n";
#endif
}



/* gets host record */
/* hostRecIndx: index in Table; pHost: pointer to Host variable through which the record is returned */
void Country::HostTbl::GetHostRec(unsigned int hostRecIndx, Country::Host *pHost)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetHostrec(unsigned int, Host*) started...\n";
#endif

	*pHost = Table[hostRecIndx];

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetHostrec(unsigned int, Host*) completed.\n\n";
#endif
}



/* sets establishment ID (index in Establishment table) */
/* hostRecIndx: index in Table; EstID1, EstID2: IDs of establishment */
void Country::HostTbl::SetEstablishmentID(unsigned int hostRecIndx, unsigned int estID1, unsigned int estID2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::SetEstablishmentID(unsigned int, unsigned int) started...\n";
#endif

	Table[hostRecIndx].EstID.ID1 = estID1;
	Table[hostRecIndx].EstID.ID2 = estID2;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::SetEstablishmentID(unsigned int, unsigned int) completed.\n\n";
#endif
}



/* sets host group ID */
/* hostRecIndx: index in Table; groupID: group ID */
void Country::HostTbl::SetHostGroupID(unsigned int hostRecIndx, unsigned int groupID)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTbl::SetHostGroupID(unsigned int, unsigned int) started...\n";
#endif

	Table[hostRecIndx].EstID.GroupID = groupID;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTbl::SetHostGroupID(unsigned int, unsigned int) completed.\n\n";
#endif
}


/* gets total number of hosts */
unsigned int Country::HostTbl::GetNumHosts()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetNumHosts() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetNumHosts() completed\n\n";
#endif

	return NumRecs;
}



/* gets list of hosts residing in household */
/* pHshld: pointer to Household object; pHostList: pointer to array of Host objects to be filled in */
void Country::HostTbl::GetHostList(Country::Household *pHshld, Country::Host *pHostList)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetHostList(Country::Household*, Country::Host*) started...\n";
#endif

	for(unsigned short i=0; i < pHshld->NumRecs; i++)
		pHostList[i] =  pHshld->H[i];

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetHostList(Country::Household*, Country::Host*) completed.\n\n";
#endif
}



/* gets random unassigned host from household */
/* pHshld: pointer to Household object */
unsigned short Country::HostTbl::GetRndUnassgndHostIndx(Country::Household *pHshld)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*) started...\n";
#endif
	unsigned short hostIndx = pHshld->NumRecs * ranf_();         // sample host index
	for(unsigned short i = hostIndx; i < pHshld->NumRecs; i++)   // check if there is any unassigned host with index >= hostIndx
		if( pHshld->H[i].EstID.ID1 == (unsigned short)UNDEFINED )
		{
#ifdef OUTPUT_LOG_L2
			cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*) completed.\n\n";
#endif
			return i;
		}
	for(short i = hostIndx - 1; i >= 0; i--)                     // check if there is any unassigned host with index < hostIndx
		if( pHshld->H[i].EstID.ID1 == (unsigned short) UNDEFINED)
		{
#ifdef OUTPUT_LOG_L2
			cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*) completed.\n\n";
#endif
			return i;
		}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*) completed. All hosts from given households have been already assigned.\n\n";
#endif

	return (unsigned short)UNDEFINED;                            // no unassigned hosts left
}



/* gets random unassigned host whose age falls within [ageLB, ageUB) from household */
/* pHshld: pointer to Household object; ageLB and ageUB: age boundaries */
unsigned short Country::HostTbl::GetRndUnassgndHostIndx(Country::Household *pHshld, float ageLB, float ageUB)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*, float, float) started...\n";
#endif
	unsigned short hostIndx = pHshld->NumRecs * ranf_();         // sample host index
	for(unsigned short i = hostIndx; i < pHshld->NumRecs; i++)   // check if there is any unassigned host with index >= hostIndx
	{
		if( pHshld->H[i].EstID.ID1 == (unsigned short)UNDEFINED )
		{
#ifdef OUTPUT_LOG_L2
			cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*, float, float) completed.\n\n";
#endif
			if( (pHshld->H[i].Age >= ageLB) && (pHshld->H[i].Age < ageUB) )
				return i;
			else
				continue;
		}
	}
	for(short i = hostIndx - 1; i >= 0; i--)                     // check if there is any unassigned host with index < hostIndx
	{
		if( pHshld->H[i].EstID.ID1 == (unsigned short)UNDEFINED )
		{
#ifdef OUTPUT_LOG_L2
			cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*, float, float) completed.\n\n";
#endif
			if( (pHshld->H[i].Age >= ageLB) && (pHshld->H[i].Age < ageUB) )
				return i;
			else
				continue;
		}
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTblCl::GetRndUnassignedHost(Country::Household*, float, float) completed. All hosts from given households have been already assigned.\n\n";
#endif

	return (unsigned short)UNDEFINED;                            // no unassigned hosts left
}



/* resets establishment IDs assigned to hosts */
void Country::HostTbl::ResetEstIDs()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTbl::ResetEstIDs() started...\n";
#endif

	#pragma omp parallel for
	for(unsigned int i=0; i < NumRecs; i++)
	{
		Table[i].EstID.ID1 = (unsigned short)UNDEFINED;
		Table[i].EstID.ID2 = (unsigned int)UNDEFINED;
		Table[i].EstID.GroupID = (unsigned short)UNDEFINED;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HostTbl::ResetEstIDs() completed.\n\n";
#endif
}



/* corrects for marginal population age distribution difference between model and real world data */
/* pMADDev: pointer to deviation vector */
void Country::HostTbl::CorrectForPAGDev(MADVec *pMADDev)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::CorrectForPAGDev(MADVec*) started...\n";
#endif

	ADDevVec *pADDevVec;
	try
	{
		pADDevVec = new ADDevVec(pMADDev);
	}
	catch(int ExcCode)
	{
		if( ExcCode == INCORRECT_DISTR_VECTOR )
		{
			cerr << "*** Error: Probability distribution vector does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

	}

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Redundnancy age group cumulative probability distribution vector (age band: " << pADDevVec->AgeBand << "):\n";
	for(unsigned short i=0; i < pADDevVec->RedNumEls; i++)
		cout << pADDevVec->RedV[i].Age << "\t" <<  pADDevVec->RedV[i].CProb << "\n";

	cout << "\nDeficiency age group cumulative probability distribution vector (age band: " << pADDevVec->AgeBand << "):\n";
	for(unsigned short i=0; i < pADDevVec->DefNumEls; i++)
		cout << pADDevVec->DefV[i].Age << "\t" << pADDevVec->DefV[i].CProb << "\n";
#endif

#ifdef OUTPUT_LOG_TEST_L1
	cout << "*** Applying corrections for deviation of model marginal distribution from real world data...\n";
#endif

	try
	{
		for(unsigned int i=0; i < NumRecs; i++)
			pADDevVec->CorrectDev(Table[i]);
	}
	catch(int ExcCode)
	{
		if( ExcCode == INCORRECT_DISTR_VECTOR )
		{
			cerr << "*** Error: Probability distribution vector does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

	}

#ifdef OUTPUT_LOG_TEST_L1
	cout << "*** Corrections applied\n";
#endif

	delete pADDevVec;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::CorrectForPAGDev(MADVec*) completed.\n\n";
#endif
}



/* computes marginal population age distribution */
MADVec* Country::HostTbl::CompMargModelPopulAgeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::CompMargModelPopulAgeDistr() started...\n";
#endif

	unsigned int i, numEls;
	float maxAge;
	for(i=0, maxAge = 0.0f; i < NumRecs; i++)
	{
		if( Table[i].Age > maxAge )
			maxAge = Table[i].Age;
	}
	numEls = (unsigned int)ceil(maxAge/AGE_BAND);
	MADVec* pMMdADVec = new MADVec(numEls);
	for(i=0; i < NumRecs; i++)
	{
		unsigned short indx = (unsigned short)(ceil(Table[i].Age/AGE_BAND))-1;
		pMMdADVec->V[indx].NumHosts++;
	}

	for(i=0; i < pMMdADVec->NumEls; i++)
		pMMdADVec->V[i].Prob = (float)pMMdADVec->V[i].NumHosts/(float)NumRecs;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HostTblCl::CompMargModelPopulAgeDistr() completed.\n\n";
#endif

	return pMMdADVec;
}



/* default constructor; creates undefined household record */
Country::Household::Household() : Lat((double)UNDEFINED), Lon((double)UNDEFINED), LndScnX((unsigned int)UNDEFINED), LndScnY((unsigned int)UNDEFINED), NumRecs(0), H(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::Household() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::Household() completed\n\n";
#endif
}



/* constructor;  creates a record for household and reserves records for household members in Hosts table */
/* hshldLat and hshldLon: lattitude and longitude coordinate; hshldLndScnX and hshldLndScnY: X and Y LandScan indexes of the cell where the household is located; */
/* hshldSize: size of household; pHostTbl: pointer to Hosts table */
Country::Household::Household(double hshldLat, double hshldLon, unsigned int hshldLndScnX, unsigned int hshldLndScnY, unsigned short hshldSize, HostTbl *pHostTbl) : \
	Lat(hshldLat), Lon(hshldLon), LndScnX(hshldLndScnX), LndScnY(hshldLndScnY), NumRecs(hshldSize), H(pHostTbl->ReserveHostRecs(hshldSize))
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::Household(double, double, unsigned short, HostTblCl*) started...\n";
#endif	
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::Household(double, double, unsigned short, HostTblCl*) completed\n\
			Lat: " << Lat << "; Lon: " << Lon << "; Number of hosts in household: " << NumRecs << "\n\n";
#endif
}



/* sets age of househould members using direct sampling */
/* PADVM: pointer to matrix of age distribution vectors */
void Country::Household::SetHshldMembsAgeDir(ADVMtrx *pADVM)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::SetHshldMembsAgeDir(ADVMtrx*) started...\n\n";
#endif

	for(unsigned int i=0; i < NumRecs; i++)
		H[i].Age = pADVM->Sample(NumRecs);

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::SetHshldMembsAgeDir(ADVMtrx*) completed.\n\n";
#endif
}



/* sets Establishment ID (index in Establishment table) */
/* hostIndx: host index within household; estID1, estID2: IDs of establishment */
void Country::Household::SetEstablishmentID(unsigned int hostIndx, unsigned short estID1, unsigned int estID2)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::SetEstablishmentID(unsigned int, unsigned int) started...\n";
#endif

	H[hostIndx].EstID.ID1 = estID1;
	H[hostIndx].EstID.ID2 = estID2;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::SetEstablishmentID(unsigned int, unsigned int) completed.\n\n";
#endif
}



/* returns ID1 - characteristic of establishment type assigned in accordance with host's age group; returns UNDEFINED if no assignment has been done yet */
/* hostIndx: host index within household */
unsigned short Country::Household::GetEstablishmentID1(unsigned int hostIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID1() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID1() completed.\n\n";
#endif
	return H[hostIndx].EstID.ID1;
}



/* returns establishment ID2 if host has been assigned to some establishment or UNDEFINED otherwise */
/* hostIndx: host index within household */
unsigned int Country::Household::GetEstablishmentID2(unsigned int hostIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID2() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID2() completed.\n\n";
#endif
	return H[hostIndx].EstID.ID2;
}



/* returns ID of group given host belongs to */
/* hostIndx: host index within household */
unsigned short Country::Household::GetGroupID(unsigned int hostIndx)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID2() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::Household::CheckEstablishmentID2() completed.\n\n";
#endif
	return H[hostIndx].EstID.GroupID;
}



/* default constructor; creates undefined table */
Country::HouseholdTbl::HouseholdTbl() : ExtCoeff(DEFAULT_EXT_COEFF), NumRecs(0), NumRecsMax(0), Table(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::HouseholdTblCl() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::HouseholdTblCl() completed.\n\n";
#endif
}



/* constructor; creates empty table */
/* TblSize: size of table */
Country::HouseholdTbl::HouseholdTbl(unsigned int tblSize, float extCoeff) : ExtCoeff(extCoeff), NumRecs(0), NumRecsMax(tblSize)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::HouseholdTblCl(unsigned int) started...\n";
#endif

	try
	{
		 Table = new Household[NumRecsMax];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of Country::Household objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::HouseholdTblCl(unsigned int) completed.\n\n";
#endif
}



Country::HouseholdTbl::~HouseholdTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::~HouseholdTblCl() started...\n";
#endif

	if( Table != NULL )
		delete [] Table;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::~HouseholdTblCl() completed.\n";
#endif
}



/* adds record in Household table */
/* pHshld: pointer to a record to be added */
void Country::HouseholdTbl::AddRec(Country::Household *pHshld)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::AddRec(Household*) started...\n";
#endif

	if( NumRecs == NumRecsMax )      // no space left in Table, extend it
	{
		if( NumRecsMax != 0 )
			NumRecsMax *= (1 + ExtCoeff);
		else
			NumRecsMax = DEFAULT_TBL_SIZE;

		Household *TableTemp;
		try
		{
			TableTemp = new Household[NumRecsMax];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (TableTemp) of Country::Household objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

		#pragma omp parallel for
		for(unsigned int i=0; i < NumRecs; i++)
			TableTemp[i] = Table[i];

		if( Table != NULL )
			delete [] Table;
		Table = TableTemp;
	}
	
	Table[NumRecs++] = *pHshld;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::AddRec(Household*) completed\n\n";
#endif
}



/* appends a HouseloldTblCl table to an existing one */
/* pHshldTbl: pointer to household table to be appended */
void Country::HouseholdTbl::AppendTable(Country::HouseholdTbl *pHshldTbl)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::AppendTable(Country::HouseholdTblCl*) started...\n";
#endif

	unsigned int i, j;
	unsigned int numRecsNew = pHshldTbl->NumRecs + NumRecs;
	if( numRecsNew > NumRecsMax )
	{
		Household *TableTemp;
		try
		{
			TableTemp = new Household[numRecsNew];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (TableTemp) of  Country::Household objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(i=0; i < NumRecs; i++)
			TableTemp[i] = Table[i];
		for(j=0; i < numRecsNew; i++, j++)
			TableTemp[i] = pHshldTbl->Table[j];
		if( Table != NULL )
			delete [] Table;
		Table = TableTemp;
		NumRecsMax = numRecsNew;
	}
	else
	{
		for(i = NumRecs, j=0; i < numRecsNew; i++, j++)
			Table[i] = pHshldTbl->Table[j];
	}
	NumRecs = numRecsNew;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::AppendTable(Country::HouseholdTblCl*) completed.\n\n";
#endif
};



/* resets table */
void Country::HouseholdTbl::ResetTable()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::ResetTable() started...\n";
#endif

	NumRecs = 0;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::ResetTable() completed.\n\n";
#endif
}



unsigned int Country::HouseholdTbl::GetNumHouseholds()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetNumHouseholds() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetNumHouseholds() completed.\n\n";
#endif

	return NumRecs;
}



/* gets max size of household */
unsigned short Country::HouseholdTbl::GetMaxHshldSize()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetMaxHshldSize() started...\n";
#endif

	unsigned int i, maxHshldSize;
	for(i=0, maxHshldSize = 0; i < NumRecs; i++)
		if( Table[i].NumRecs > maxHshldSize )
			maxHshldSize = Table[i].NumRecs;

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetMaxHshldSize() completed.\n\n";
#endif
	return maxHshldSize;
}



/* synthesizes household age structure using direct sampling */
/* pADVM: pointer to age distribution matrix */
void Country::HouseholdTbl::SynthesizeHouseholdAgeStructDir(ADVMtrx *pADVM)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::SynthesizeHouseholdAgeStructDir(ADVMtrx*) started...\n";
#endif

	#pragma omp parallel for
	for(unsigned int i=0; i < NumRecs; i++)
	{
#ifdef OUTPUT_LOG_L2
		cout << "Household #" << (i+1) << "\nLat: " << Table[i].Lat << "; Lon: " << Table[i].Lon << "; Number of household members: " << Table[i].NumRecs << "\n";
#endif
		Table[i].SetHshldMembsAgeDir(pADVM);
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::SynthesizeHouseholdAgeStructDir(ADVMtrx*) completed.\n\n";
#endif
}



/* computes model household size distribution */
Country::MdHSDVec* Country::HouseholdTbl::CompModelHshldSizeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::CompModelHshldSizeDistr() started...\n";
#endif

	MdHSDVec *pMdHSDV = new MdHSDVec(GetMaxHshldSize());

	for(unsigned int i=0; i < NumRecs; i++)
		pMdHSDV->V[Table[i].NumRecs-1].NumHshlds++;

	for(unsigned short i=0; i < pMdHSDV->NumEls; i++)
		pMdHSDV->V[i].Prob = (float)pMdHSDV->V[i].NumHshlds/(float)NumRecs;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::CompModelHshldSizeDistr() completed\n\n";
#endif

	return pMdHSDV;
}



/* computes model household-age distribution */
Country::MdADVMtrx* Country::HouseholdTbl::CompModelHshldAgeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::CompModelHshldAgeDistr() started...\n";
#endif

	unsigned int i, maxHshldSize;
	unsigned short j;
	float maxAge;
	for(i=0, maxHshldSize = 0, maxAge = 0; i < NumRecs; i++)
	{
		if( Table[i].NumRecs > maxHshldSize )
			maxHshldSize = Table[i].NumRecs;
		for(j=0; j < Table[i].NumRecs; j++)
			if( Table[i].H[j].Age > maxAge )
				maxAge = Table[i].H[j].Age;
	}

	unsigned short numAgeGroup = ceil(maxAge/AGE_BAND);

	MdADVMtrx* pMdADVMtrx = new MdADVMtrx(maxHshldSize, numAgeGroup);

	for(i=0; i < NumRecs; i++)
	{
		for(j=0; j < Table[i].NumRecs; j++)
		{
			unsigned short indx = (unsigned short)(ceil(Table[i].H[j].Age/AGE_BAND))-1;
			pMdADVMtrx->M[Table[i].NumRecs-1].V[indx].NumHosts++;
		}
	}

	float normFactor;
	for(i=0; i < pMdADVMtrx->NumEls; i++)
	{
		for(j=0, normFactor = 0.0f; j < pMdADVMtrx->M[i].NumEls; j++)
			normFactor += (float)pMdADVMtrx->M[i].V[j].NumHosts;

		for(j=0; j < pMdADVMtrx->M[i].NumEls; j++)
			pMdADVMtrx->M[i].V[j].Prob = (float)pMdADVMtrx->M[i].V[j].NumHosts/normFactor;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::HouseholdTblCl::CompModelHshldAgeDistr() completed.\n\n";
#endif

	return pMdADVMtrx;
}



/* gets household object out of a Table */
/* hshldRecIndx: index of Household object (record); hshld: pointer to Household object thourgh which that record is returned */
void Country::HouseholdTbl::GetHousehold(unsigned int hshldRecIndx, Country::Household *pHshld)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetHousehold(unsigned int, Household*) started...\n";
#endif

	*pHshld = Table[hshldRecIndx];

#ifdef OUTPUT_LOG_L2
	cout << "*** Country::HouseholdTblCl::GetHousehold(unsigned int, Household*) completed.\n\n";
#endif
}



/* corrects for marginal population age distribution difference between model and real world data */
/* pMADDev: pointer to deviation vector; pADVM: pointer to matrix of age distribution vectors */
void Country::HouseholdTbl::CorrectForPAGDev(MADVec *pMADDev, ADVMtrx *pADVM)
{
#ifdef OUTPUT_LOG_L1
	cout << "***  Country::HouseholdTblCl::CorrectForPAGDev(MADVec*, ADVMtrx*) started...\n";
#endif

	ADDevVec *pADDevVec;
	try
	{
		pADDevVec = new ADDevVec(pMADDev);
	}
	catch(int ExcCode)
	{
		if( ExcCode == INCORRECT_DISTR_VECTOR )
		{
			cerr << "*** Error: Probability distribution vector does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

	}

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Redundnancy age group cumulative probability distribution vector (age band: " << pADDevVec->AgeBand << "):\n";
	for(unsigned short i=0; i < pADDevVec->RedNumEls; i++)
		cout << pADDevVec->RedV[i].Age << "\t" <<  pADDevVec->RedV[i].CProb << "\n";

	cout << "\nDeficiency age group cumulative probability distribution vector (age band: " << pADDevVec->AgeBand << "):\n";
	for(unsigned short i=0; i < pADDevVec->DefNumEls; i++)
		cout << pADDevVec->DefV[i].Age << "\t" << pADDevVec->DefV[i].CProb << "\n";
#endif

#ifdef OUTPUT_LOG_TEST_L1
	cout << "\n*** Applying corrections for deviation of model marginal distribution from real world data...\n";
#endif

	try
	{
		#pragma omp parallel for
		for(unsigned int i=0; i < NumRecs; i++)
			pADDevVec->CorrectDev(Table[i], pADVM);
	}
	catch(int ExcCode)
	{
		if( ExcCode == INCORRECT_DISTR_VECTOR )
		{
			cerr << "*** Error: Probability distribution vector does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

	}

#ifdef OUTPUT_LOG_TEST_L1
	cout << "*** Corrections applied\n\n";
#endif

	delete pADDevVec;

#ifdef OUTPUT_LOG_L1
	cout << "***  Country::HouseholdTblCl::CorrectForPAGDev(MADVec*, ADVMtrx*) completed.\n\n";
#endif
}

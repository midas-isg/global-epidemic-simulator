/* Country.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "HouseholdStruct.h"
#include "EstablishmentDistr.h"
#include "EstablishmentStruct.h"
#include "VacancyAndEstablishmentMap.h"
#include "Country.h"
#ifdef _OPENMP
#include "omp.h"
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#endif
#ifndef _WIN32
#include "Itoa.h"
#endif


__inline long seed_randlib(void)
{
	long seed;
	static long aux=0;

	seed = time(NULL);
	seed += aux;
	aux = seed/7;

	return seed;
}



/* constructor; initialises country code, LandScan cell table for the country, household size and household members' age distributions */
/* CC: country code;  pCLSCTbl: pointer to LandScan table for the country; pHSDV: pointer to household size distribution; pADVM: pointer to matrix of age distribution vectors */
/* pESDV: pointer to establishment size distribution; pHETDKernel: pointer to the kernel function object */
Country::Country(unsigned char CC, CntryLndScnCellTbl *pCLSCTbl, HSDVec *pHSDV, ADVMtrx *pADVM, ESDVec *pESDV, HostToEstTravelDistrKernel *pHETDKernel) : \
	CountryCode(CC), PCLSCTbl(pCLSCTbl), PHostTbl(new HostTbl(PCLSCTbl->NumHosts)), PHshldTbl(NULL), PHSDV(pHSDV), PADVM(pADVM), PMMdADVec(NULL), PVacancyGeoMap(NULL), \
	PELDV(NULL), PESDV(pESDV), PEstTbl(NULL), PHstToEstMap(NULL), PHETDKernel(pHETDKernel)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::Country(unsigned char, CntryLndScnCellTblCl*, SDVec*, ADVMtrx*, ESDVec*) started...\n";
#endif

	long seed1 = seed_randlib();
	long seed2 = seed_randlib();
#ifdef _OPENMP
	unsigned int procsNum = omp_get_num_procs();
	set_gen(seed1, seed2, procsNum);
#else
	setall(seed1, seed2);
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::Country(unsigned char, CntryLndScnCellTblCl*, SDVec*, ADVMtrx*, ESDVec*) completed\n\n";
#endif
}



/* constructor; initialises country code, LandScan cell table for the country, household size and household members' age distributions */
/* CC: country code; SclDwnFactor: scale down factor for model population; pCLSCTbl: pointer to LandScan table for the country; pHSDV: pointer to household size distribution; */
/*pADVM: pointer to household members' age distributions; pESDV: pointer to establishment size distribution; pHETDKernel: pointer to the kernel function object */
Country::Country(unsigned char CC, float scaleDownFactor, CntryLndScnCellTbl *pCLSCTbl, HSDVec *pHSDV, ADVMtrx *pADVM, ESDVec *pESDV, HostToEstTravelDistrKernel *pHETDKernel) : \
	CountryCode(CC), PCLSCTbl(pCLSCTbl), PHshldTbl(NULL), PHSDV(pHSDV), PADVM(pADVM), PMMdADVec(NULL), PVacancyGeoMap(NULL), PELDV(NULL), PESDV(pESDV), PEstTbl(NULL), PHstToEstMap(NULL), PHETDKernel(pHETDKernel)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::Country(unsigned char, float, CntryLndScnCellTblCl*, SDVec*, ADVMtrx*, ESDVec*) started...\n";
#endif

	PCLSCTbl->ScaleDownPopul(scaleDownFactor);

#ifdef OUTPUT_LOG_TEST_L2
	cout << "Population Density Map (number of records:" << PCLSCTbl->NumRecs << "):\n";
	for(unsigned int i=0; i < PCLSCTbl->NumRecs; i++)
		cout << PCLSCTbl->Table[i].LndScnX << "\t" << PCLSCTbl->Table[i].LndScnY << "\t" << (float)PCLSCTbl->Table[i].NumHosts/(float)PCLSCTbl->NumHosts << "\n";
	cout << "\n";
#endif

	PHostTbl = new HostTbl(PCLSCTbl->NumHosts);

	long seed1 = seed_randlib();
	long seed2 = seed_randlib();
#ifdef _OPENMP
	unsigned int procsNum = omp_get_num_procs();
	set_gen(seed1, seed2, procsNum);
#else
	setall(seed1, seed2);
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::Country(unsigned char, float, CntryLndScnCellTblCl*, SDVec*, ADVMtrx*, ESDVec*) completed\n\n";
#endif
}



Country::~Country()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::~Country() started...\n";
#endif

#ifdef _OPENMP
	unset_gen();
#endif

	if( PHostTbl != NULL )
	{
		delete PHostTbl;
		PHostTbl = NULL;
	}
	
	if( PHshldTbl != NULL )
	{
		delete PHshldTbl;
		PHshldTbl = NULL;
	}

	if( PMMdADVec != NULL )
	{
		delete PMMdADVec;
		PMMdADVec = NULL;
	}

	if( PVacancyGeoMap != NULL )
	{
		delete PVacancyGeoMap;
		PVacancyGeoMap = NULL;
	}

	if( PELDV != NULL )
	{
		delete PELDV;
		PELDV = NULL;
	}

	if( PEstTbl != NULL )
	{
		delete PEstTbl;
		PEstTbl = NULL;
	}

	if( PHstToEstMap != NULL )
	{
		delete PHstToEstMap;
		PHstToEstMap = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::~Country() completed\n\n";
#endif
}



/* synthesizes household structure by direct sampling */
void Country::SynthesizeHshlds()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeHshlds() started...\n";
#endif

	if( PHshldTbl != NULL )
		delete PHshldTbl;

	PHshldTbl = new HouseholdTbl();

	int numHostsCellNew = 0;
	#pragma omp parallel for firstprivate(numHostsCellNew)
	for(unsigned int i=0; i < PCLSCTbl->NumRecs; i++)
	{
		Household *pHshld;
		double lat, lon;
		unsigned int LndScnX, LndScnY;
		unsigned short hshldSize;
		int numHostsCell;

		numHostsCell = PCLSCTbl->Table[i].NumHosts;
		if( (numHostsCellNew = numHostsCell - ABS(numHostsCellNew)) <= 0 )
			continue;

		LndScnX = PCLSCTbl->Table[i].LndScnX;                                              // LandScan X and Y indexes of the cell containing the household
		LndScnY = PCLSCTbl->Table[i].LndScnY;                                              //

		lat = PCLSCTbl->LndScnCellLat(i) - PCLSCTbl->CellSize * ranf_();                   // Lat and Lon coordinates of the household
		lon = PCLSCTbl->LndScnCellLon(i) + PCLSCTbl->CellSize * ranf_();                   //
		try
		{
			for( ; ; )
			{
				hshldSize = PHSDV->Sample();                                               // sample household size
				pHshld = new Household(lat, lon, LndScnX, LndScnY, hshldSize, PHostTbl);   // create a household object
				#pragma omp critical(AddHouseholdRecord)
				{
				PHshldTbl->AddRec(pHshld);                                                 // create a record in table of households
				}
				delete pHshld;

				if( (numHostsCellNew -= hshldSize) <= 0 )
					break;
			}
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case TABLE_TOO_SMALL:
				cerr << "*** Exception in Country::SynthesizeHshlds(): TABLE_TOO_SMALL (NONCRITICAL) *** \n";
				break;
			
			case TABLE_UNDEFINED:
				cerr << "*** Exception in Country::SynthesizeHshlds(): TABLE_UNDEFINED (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeHshlds() completed\n\n";
#endif
}



/* computes and outputs model household size distribution */
void Country::CompModelHshldSizeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHshldSizeDistr() started...\n";
#endif

	MdHSDVec *pMdHSDV = PHshldTbl->CompModelHshldSizeDistr();

#ifdef CON_OUTPUT
	cout << "Total number of households: " << PHshldTbl->GetNumHouseholds() << "\n";
	cout << "Model household size distribution:\n";
	for(unsigned short i=0; i < pMdHSDV->NumEls; i++)
		cout << pMdHSDV->V[i].Size << "\t" << pMdHSDV->V[i].Prob << "\t" << pMdHSDV->V[i].NumHshlds << "\n";
	cout << "\n";
#endif

	delete pMdHSDV;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHshldSizeDistr() completed\n\n";
#endif
}



/* synthesizes population age structure (on household level) by direct sampling */
void Country::SynthesizePopulAgeStructDir()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizePopulAgeStructDir() started...\n";
#endif

	PHshldTbl->SynthesizeHouseholdAgeStructDir(PADVM);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizePopulAgeStructDir() completed\n\n";
#endif
}



/* computes and outputs model household-age distribution */
void Country::CompModelHshldAgeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHshldAgeDistr() started...\n";
#endif

	MdADVMtrx* pMdADVMtrx = PHshldTbl->CompModelHshldAgeDistr();

#ifdef CON_OUTPUT
	for(unsigned short i=0; i < pMdADVMtrx->NumEls; i++)
	{
		cout << "Age distribution vector, " << (i+1) << "-person household" << ":\n";
		for(unsigned short j=0; j < pMdADVMtrx->M[i].NumEls; j++)
			cout << pMdADVMtrx->M[i].V[j].Age << "\t" << pMdADVMtrx->M[i].V[j].Prob << "\t" << pMdADVMtrx->M[i].V[j].NumHosts << "\n";
		cout << "\n";
	}
#endif

	delete pMdADVMtrx;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompModelHshldAgeDistr() completed\n";
#endif
}



/* computes and outputs marginal model population age distribution */
void Country::CompMargModelPopulAgeDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompMargModelPopulAgeDistr() started...\n";
#endif

	//MADVec* pMMdADVec = PHostTbl->CompMargModelPopulAgeDistr();
	if( PMMdADVec != NULL )
		delete PMMdADVec;

	PMMdADVec = PHostTbl->CompMargModelPopulAgeDistr();

#ifdef CON_OUTPUT
	cout << "Total number of hosts: " << PHostTbl->GetNumHosts() << "\n";
	cout << "Marginal model population age distribution vector:\n";
	for(unsigned short i=0; i < PMMdADVec->NumEls; i++)
		cout << PMMdADVec->V[i].Age << "\t" << PMMdADVec->V[i].Prob << "\t" << PMMdADVec->V[i].NumHosts << "\n";
	cout << "\n";
#endif

	//delete pMMdADVec;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CompMargModelPopulAgeDistr() completed\n\n";
#endif
}



/* outputs synthesized population */
/* fName: name of file for output */
void Country::OutputSynthPopulation(char *fNameTmpl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::OutputSynthPopulation(char*) started...\n";
#endif

	if( !PHETDKernel->GetOutputFlag() )
	{
#ifdef OUTPUT_LOG_L1
	cout << "*** Output flag has not been set for this parameter set, so no output will be produced.\n*** Country::OutputSynPopul(char*) completed.\n*** Country::OutputSynPopul(char*) completed.\n\n";
#endif
		return;
	}

	char *fName = new char[FN_BUF_LNGTH];
	char *fNameSfx = new char[FN_BUF_LNGTH];
	strcpy(fName, fNameTmpl);
	itoa(PHETDKernel->GetCrntParamSetIndx(), fNameSfx, 10);
	
	if( (strlen(fName) + 1 + strlen(fNameSfx)) >= FN_BUF_LNGTH )
	{
		cout << "Synthesized population file name is too long, no output will be produced.\n";		
		delete [] fName;
		delete [] fNameSfx;
#ifdef OUTPUT_LOG_L1
		cout << "*** Country::OutputSynthPopulation(char*) completed.\n\n";
#endif
		return;
	}

	strcat(fName, "_");
	strcat(fName, fNameSfx);

#ifdef OUTPUT_LOG_L1
	cout << "Synthesized population data will be written into the file " << fName << "\n";
#endif

	fstream outF(fName, ios::out | ios::trunc | ios::binary);
	if( !outF )
		throw FILE_OPEN_ERROR;

	Household *pHshld = new Household;
	Host *pHostList = new Host[PHshldTbl->GetMaxHshldSize()];

	unsigned int numHouseholds = PHshldTbl->GetNumHouseholds();
	unsigned int numHosts = PHostTbl->GetNumHosts();
	unsigned int dummy = (unsigned int)UNDEFINED;                 // dummy field (undefined data)

	unsigned short ageGroupNum = PHETDKernel->GetAgeGroupNum();
	outF.write((char*)&ageGroupNum, sizeof(ageGroupNum));
	for(unsigned char i=0; i < ageGroupNum; i++)
	{
		float ageGroupLB = PHETDKernel->GetLowerAgeGroupBoundary(i);
		outF.write((char*)&ageGroupLB, sizeof(ageGroupLB));
	}

	outF.write((char*)&numHouseholds, sizeof(numHouseholds));
	outF.write((char*)&numHosts, sizeof(numHosts));
	for(unsigned int i=0; i < numHouseholds; i++)
	{
		PHshldTbl->GetHousehold(i, pHshld);
		PHostTbl->GetHostList(pHshld, pHostList);

		outF.write((char*)&(pHshld->Lat), sizeof(pHshld->Lat));
		outF.write((char*)&(pHshld->Lon), sizeof(pHshld->Lon));
		outF.write((char*)&(pHshld->NumRecs), sizeof(pHshld->NumRecs));
		outF.write((char*)&dummy, sizeof(dummy));
		for(unsigned short j=0; j < pHshld->NumRecs; j++)
		{
			unsigned char ageGroupIndx = PHETDKernel->GetAgeGroupIndx(pHostList[j].Age);
			outF.write((char*)&ageGroupIndx, sizeof(ageGroupIndx));
			outF.write((char*)&(pHostList[j].Age), sizeof(pHostList[j].Age));
			outF.write((char*)&(pHostList[j].EstID.ID1), sizeof(pHostList[j].EstID.ID1));
			outF.write((char*)&(pHostList[j].EstID.ID2), sizeof(pHostList[j].EstID.ID2));
			outF.write((char*)&(pHostList[j].EstID.GroupID), sizeof(pHostList[j].EstID.GroupID));
		}
	}

	delete pHshld;
	delete [] pHostList;

	delete [] fName;
	delete [] fNameSfx;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::OutputSynthPopulation(char*) completed.\n\n";
#endif
}



/* corrects for marginal population age distribution differences between model and real world data */
/* pRWDataMADVec: pointer to real world marginal age distribution vector; pADVM: pointer to matrix of model age distribution vectors */
void Country::CorrectMADDev(MADVec *pRWDataMADVec, ADVMtrx *pADVM)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CorrectMADDev(MADVec*, ADVMtrx*) started...\n";
#endif

	MADVec* pMMdADVec = PHostTbl->CompMargModelPopulAgeDistr();
	
	MADVec MADDev = *pMMdADVec - *pRWDataMADVec;   // deviation of model marginal probability from real world data
	
	delete pMMdADVec;

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Marginal age distribution deviation:\n";
	for(unsigned short i=0; i < MADDev.NumEls; i++)
		cout << MADDev.V[i].Age << "\t" << MADDev.V[i].Prob << "\t" << MADDev.V[i].NumHosts << "\n";
	cout << "\n";
#endif

	PHshldTbl->CorrectForPAGDev(&MADDev, pADVM);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::CorrectMADDev(MADVec*, ADVMtrx*) completed\n\n";
#endif
}



/* sets the indexes of the age group and the parameter set */
void Country::SetAgeGroupParamSetIndxs(unsigned short ageGroupIndx, unsigned short paramSetIndx)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SetAgeGroupParamSetIndxs(unsigned short, unsigned short) started...\n";
#endif
	PESDV->SetAgeGroupIndx(ageGroupIndx);
	PHETDKernel->SetAgeGroupIndx(ageGroupIndx);
	PHETDKernel->SetParamSetIndx(paramSetIndx);
#ifdef OUTPUT_LOG_L1
	cout << "Parameter index " << paramSetIndx << " set\n";
	cout << "Age group index " << ageGroupIndx << " (" << PHETDKernel->GetCrntLowerAgeGroupBoundary() << "-" << PHETDKernel->GetCrntUpperAgeGroupBoundary() << " years) set\n";
	cout << "*** Country::SetAgeGroupParamSetIndxs(unsigned short, unsigned short) completed.\n\n";
#endif
}



/* gets number of age groups */
unsigned short Country::GetAgeGroupNum()
{
	return PHETDKernel->GetAgeGroupNum();
}



/* gets number of parameter sets */
unsigned short Country::GetParamSetNum()
{
	return PHETDKernel->GetParamSetNum();
}



/* synthesizes vacancy geo map */
void Country::SynthesizeVacancyGeoMap()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancyGeoMap() started...\n";
#endif

	if( PVacancyGeoMap != NULL )
		delete PVacancyGeoMap;
	
	PVacancyGeoMap = new VacancyGeoMap(PCLSCTbl, PHETDKernel);

#ifdef OUTPUT_LOG_TEST_L2
	cout << "Vacancy geomap (number of records: " << PVacancyGeoMap->NumRecs << "):\n";
	for(unsigned int i=0; i < PVacancyGeoMap->NumRecs; i++)
		cout << PVacancyGeoMap->Table[i].LndScnX << "\t" << PVacancyGeoMap->Table[i].LndScnY << "\t" << PVacancyGeoMap->Table[i].Density << "\n";
	cout << "\n";
#endif

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancyGeoMap() completed\n\n";
#endif
}



/* synthesizes distribution of vacancy locations based on the vacancy map */
void Country::SynthesizeVacancyLocDistr()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancyLocDistr() started...\n";
#endif

	if( PELDV != NULL )
		delete PELDV;

	PELDV = new ELDVec(PVacancyGeoMap);

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancyLocDistr() completed\n\n";
#endif
}



/* synthesizes vacancies for a part of population with ages within a given range */
void Country::SynthesizeVacancies()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancies() started...\n";
#endif

	/*
	float ageLB = PHETDKernel->GetCrntLowerAgeGroupBoundary();
	float ageUB = PHETDKernel->GetCrntUpperAgeGroupBoundary();

	float ageGroupFraction = 0.0f;
	Host *pHost = new Host();
	for(unsigned int i=0; i < PHostTbl->GetNumHosts(); i++)
	{
		PHostTbl->GetHostrec(i, pHost);
		if( (pHost->Age >= ageLB) && (pHost->Age < ageUB) )
			ageGroupFraction++;
	}
	delete pHost;
	ageGroupFraction /= (float)PHostTbl->GetNumHosts();

	int numVacancs = (float)PVacancyGeoMap->NumHosts * ageGroupFraction;
	*/

	int numVacancs = (float)PVacancyGeoMap->NumHosts;

	if( PEstTbl != NULL )
		delete PEstTbl;

	PEstTbl = new EstablishmentTbl();

	Establishment *pEst = new Establishment();

	while( numVacancs > 0 )
	{
		ELDVec::Location loc;
		PELDV->SampleLocation(PVacancyGeoMap, &loc);
		pEst->LndScnX = loc.LndScnX;
		pEst->LndScnY = loc.LndScnY;
		pEst->Lat = loc.Lat;
		pEst->Lon = loc.Lon;
		pEst->NumHostVacancs = 1;
		pEst->NumHosts = 0;
		pEst->NumStaff = 0;
		pEst->NumStaffVacancs = 0;

		PEstTbl->AddRec(pEst);

		numVacancs -= pEst->NumHostVacancs;
	}

#ifdef OUTPUT_LOG_TEST_L2
	cout << "Number of synthesized vacancies: " << PEstTbl->GetNumEstablishments() << "\n";

	class Data
	{
	public:
		unsigned int X;
		unsigned int Y;
		unsigned int NumVac;
	};

	int numRecs = PEstTbl->GetNumEstablishments();

	Data *pD = new Data[numRecs];

	for(unsigned int i=0; i < numRecs; i++)
	{
		PEstTbl->GetEstablishment(i, pEst);
		pD[i].X = pEst->LndScnX;
		pD[i].Y = pEst->LndScnY;
		pD[i].NumVac = 1;
	}
	for(unsigned int i=0;  i < numRecs; i++)
	{
		for(unsigned int j = i+1; j < numRecs; )
		{
			if( (pD[i].X == pD[j].X) && (pD[i].Y == pD[j].Y) )
			{
				pD[j] = pD[--numRecs];
				pD[i].NumVac++;
				continue;
			}
			j++;
		}
	}

	cout << "Map of synthesized vacancies (number of records: " << numRecs << "):\n";

	for(unsigned int i=0; i < numRecs; i++)
		cout << pD[i].X << "\t" << pD[i].Y << "\t" << pD[i].NumVac << "\n";
	cout << "\n";

	delete [] pD;
#endif

	delete pEst;

#ifdef OUTPUT_LOG_L1
	cout << "*** Country::SynthesizeVacancies() completed\n\n";
#endif
}

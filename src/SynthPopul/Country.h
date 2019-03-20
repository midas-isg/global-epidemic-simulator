/* Country.h, part of the Global Epidemic Simulation v1.0 BETA
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




/* main class describing the country */

#ifndef COUNTRY
#define COUNTRY



/* table of LandScan cells for a given country */
class CntryLndScnCellTbl;

/* household size distribution vector */
class HSDVec;

/* matrix of household members' age distribution vectors */
class ADVMtrx;

/* marginal population age distribution vector */
class MADVec;

/* host-to-establishment travel distribution kernel function */
class HostToEstTravelDistrKernel;

/* establishment size distribution vector */
class ESDVec;

/* host-to-establishment travel distribution vector */
class HETDVec;



/* country */
class Country
{
private:
	/* matrix of model household members' age distribution vectors */
	class MdADVMtrx;
	
	/* model household size distribution vector */
	class MdHSDVec;
	
	/* characterizes deviation of model age distribution from real world data (probability of redundancy or deficiency of age groups) */
	class ADDevVec;
	
	/* individual host */
	class Host;
	
	/* table of hosts */
	class HostTbl;

	/* individual household */
	class Household;
	
	/* table of households */
	class HouseholdTbl;

	/* map of vacancies in the country */
	class VacancyGeoMap;

	/* vector of distribution of establishment locations */
	class ELDVec;

	/* model establishment size distribution vector */
	class MdESDVec;

	/* class describing generic establishment */
	class Establishment;

	/* table of Establishment objects */
	class EstablishmentTbl;

	/* class that describes mapping hosts to establishments */
	class HostToEstablishmentMap;


	unsigned char CountryCode;                 // country code
	CntryLndScnCellTbl *PCLSCTbl;              // LandScan cell table for the country
	HostTbl *PHostTbl;                         // table of hosts
	HouseholdTbl *PHshldTbl;                   // table of households
	HSDVec *PHSDV;                             // household size distribution vector
	ADVMtrx *PADVM;                            // matrix of household members' age distribution vectors
	MADVec *PMMdADVec;                         // marginal model population age distribution vector
	VacancyGeoMap *PVacancyGeoMap;             // vacancy map
	ELDVec *PELDV;                             // vector of establishment locations distribution
	ESDVec *PESDV;                             // vector of establishment size distribution
	EstablishmentTbl *PEstTbl;                 // table of establishments
	HostToEstTravelDistrKernel *PHETDKernel;   // pointer to the kernel function object containing different kernel function types
	HostToEstablishmentMap *PHstToEstMap;      // pointer to object mapping hosts to establishments



public:
	/* constructor; initialises country code, LandScan cell table for the country, household size and household members' age distributions */
	/* CC: country code;  pCLSCTbl: pointer to LandScan table for the country; pHSDV: pointer to household size distribution; pADVM: pointer to matrix of age distribution vectors */
	/* pESDV: pointer to establishment size distribution; pHETDKernel: pointer to the kernel function object */
	Country(unsigned char CC, CntryLndScnCellTbl *pCLSTbl, HSDVec *pHSDV, ADVMtrx *pADVM, ESDVec *pESDV, HostToEstTravelDistrKernel *pHETDKernel);

	/* constructor; initialises country code, LandScan cell table for the country, household size and household members' age distributions */
	/* CC: country code; SclDwnFactor: scale down factor for model population; pCLSCTbl: pointer to LandScan table for the country; pHSDV: pointer to household size distribution; */
	/*pADVM: pointer to household members' age distributions; pESDV: pointer to establishment size distribution; pHETDKernel: pointer to the kernel function object */
	Country(unsigned char CC, float SclDwnFactor, CntryLndScnCellTbl *pCLSCTbl, HSDVec *pHSDV, ADVMtrx *pADVM, ESDVec *pESDV, HostToEstTravelDistrKernel *pHETDKernel);

	~Country();

	/* synthesizes households */
	void SynthesizeHshlds();

	/* computes and outputs model household size distribution */
	void CompModelHshldSizeDistr();

	/* synthesizes population age structure (on household level) by direct sampling */
	void SynthesizePopulAgeStructDir();

	/* computes and outputs model household-age distribution */
	void CompModelHshldAgeDistr();

	/* computes and outputs marginal model population age distribution */
	void CompMargModelPopulAgeDistr();

	/* outputs synthesized population */
	/* fNameTmpl: file name template for output */
	void OutputSynthPopulation(char *fNameTmpl);

	/* corrects for marginal population age distribution difference between model and real world data */
	/* pRWDataMADVec: pointer to real world marginal age distribution vector; pADVM: pointer to matrix of model age distribution vectors */
	void CorrectMADDev(MADVec *pRWDataMADVec, ADVMtrx *pADVM);

	/* sets the indexes of the age group and the parameter set */
	void SetAgeGroupParamSetIndxs(unsigned short ageGroupIndx, unsigned short paramSetIndx);

	/* gets number of age groups */
	unsigned short GetAgeGroupNum();

	/* gets number of parameter sets */
	unsigned short GetParamSetNum();

	/* synthesizes vacancy geo map */
	void SynthesizeVacancyGeoMap();

	/* synthesizes distribution of vacancy locations based on the vacancy map */
	void SynthesizeVacancyLocDistr();

	/* synthesizes vacancies for a part of population with ages within given range */
	void SynthesizeVacancies();

	/* creates host-to-establishment map */
	void CreateHostToEstablishmentMap();

	/* assigns hosts to establishments */
	void AssignHostsToEstablishments();

	/* creates staff-to-establishment map */
	void CreateStaffToEstablishmentMap();

	/* assigns staff to establishments */
	void AssignStaffToEstablishments();

	/* creates groups of hosts within establishments for current age group */
	void CreateHostGroups();

	/* resets establishment IDs assigned to hosts */
	void ResetHostEstIDs();

	/* computes and outputs model establishment size distribution */
	void CompModelEstablishmentSizeDistr();

	/* computes and outputs model host-to-establishment travel statistics */
	void CompModelHostToEstablishmentTravelStat();

	/* computes and outputs model host-to-establishment travel statistics for current age group */
	void CompModelHostToEstablishmentTravelStatAG();

	/* computes and outputs model host-to-establishment travel statistics for "staff" age group */
	void CompModelStaffToEstablishmentTravelStatAG();

	/* outputs table of synthesized establishments */
	/* fNameTmpl: file name template for output */
	void OutputSynthEstablishments(char *fNameTmpl);
};



#endif
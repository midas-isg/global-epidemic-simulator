/* HouseholdStruct.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* classes to synthesize households */

#ifndef HOUSEHOLD_STRUCT
#define HOUSEHOLD_STRUCT



#include "Country.h"



/* matrix of model household members' age distribution vectors */
class Country::MdADVMtrx
{
public:
	/* household members' age distribution vector */
	class MdADVec
	{
	public:
		/* element of age distribution vector */
		class MdADVEl
		{
		public:
			float Age;               // age
			float Prob;              // corresponding probability distribution vector value
			unsigned int NumHosts;   // number of hosts in the corresponding age band
		};

		unsigned short NumEls;       // number of elements in vector
		MdADVEl *V;                  // vector of MdADVEl elements
	};

	unsigned short NumEls;           // number of elements in array of age distribution vectors
	MdADVec *M;                      // array of age distribution vectors

	/* default constructor; creates undefined matrix */
	MdADVMtrx();

	/* constructor; builds empty matrix */
	/* NumMdADVec: number of age distribution vectors; MdADVecLngth: length of each vector */
	MdADVMtrx(unsigned short numMdADVec, unsigned short MdADVecLngth);

	~MdADVMtrx();
};



/* model household size distribution vector */
class Country::MdHSDVec
{
public:
	/* element of household size distribution vector */
	class MdHSDVEl
	{
	public:
		unsigned short Size;         // household size
		float Prob;                  // corresponding probability distribution vector value
		unsigned int NumHshlds;      // number of households of size Size
	};

	unsigned short NumEls;           // number of elements in vector
	MdHSDVEl *V;                      // vector of MSDVEl elements

	/* default constructor; creates undefined vector */
	MdHSDVec();

	/* constructor; builds empty household size distribution vector */
	/* MSDVecLngth: length of vector */
	MdHSDVec(unsigned short MdSDVecLngth);

	~MdHSDVec();
};



/* characterizes deviation of model age distribution from real world data (probability of redundancy or deficiency of age groups) */
class Country::ADDevVec
{
public:
	/* element of original probability distribution deviation vector - difference between model and real world marginal probability _distribution_ vectors */
	class MADDevEl
	{
	public:
		float Age;                   // age
		float DevProb;               // corresponding vector element value
	};

	/* element of normalized _cumulative_ probability deviation vector ("redundancy" or "deficiency") */
	class ADDevEl
	{
	public:
		float Age;                   // age
		float CProb;                 // corresponding cumulative probability vector value
	};
		
	float AgeBand;                   // age band in probability distribution

	float NormCoeff;                 // normalization coefficient (used to normalize "redundancy" distributions)

	unsigned short MADNumEls;        // number of elements in original probability deviation vector
	MADDevEl *MADV;                  // original probability deviation vector

	unsigned short RedNumEls;        // number of elements in "redundancy" vector
	ADDevEl *RedV;                   // "redundancy" vector of ADDevEl elements

	unsigned short DefNumEls;        // number of elements in "deficiency" vector
	ADDevEl *DefV;                   // "deficiency" vector of ADDevEl elements

	/* constructor; builds redundancy and deficiency age group distribution vectors */
	/* pMADDev: pointer to probability deviation vector */
	ADDevVec(MADVec *pMADDev);

	~ADDevVec();

	/* corrects for marginal age distribution deviation */
	/* hst: reference to host object */
	void CorrectDev(Country::Host &hst);

	/* corrects for marginal age distribution deviation - better than previous version (void CorrectDev(Host &hst)) as some simple restrictions on householdsize-host's age correlation applied */
	/* hshld: reference to household object; pADVM: pointer to matrix of age distribution vectors */
	void CorrectDev(Country::Household &hshld, ADVMtrx *pADVM);
};



/* individual host */
class Country::Host
{
public:
	float Age;                       // host's age

	class EstablishmentID
	{
	public:
		unsigned short ID1;         // 1st part of the establishment ID: indentifies an Establishment table
		unsigned int ID2;           // 2nd part: corresponds to the index within the Establishment table
		unsigned short GroupID;     // ID of a group within the establishment a given host belongs to

		/* default constructor; assigns undefined IDs */
		EstablishmentID();
	};
	EstablishmentID EstID;

	/* default constructor; creates empty Host object */
	Host();
};



/* table of hosts */
class Country::HostTbl
{
private:
	unsigned int NumRecs;            // number of nonempty records in the Table (number of hosts residing in the country)
	unsigned int NumRecsMax;         // max. allowed number of records in the Table
	Country::Host *Table;            // pointer to a table describing hosts residing in the country
	
public:
	/* default constructor; creates undefined table */
	HostTbl();

	/* constructor; creates empty table */
	/* tblSize: size of Table */
	HostTbl(unsigned int tblSize);

	~HostTbl();

	/* reserves undefined host records in Table */
	/* numHosts: number of host records */
	Country::Host* ReserveHostRecs(unsigned int numHosts);

	/* sets host record (just age so far) */
	/* hostRecIndx: index in Table; age: host's age */
	void SetHostRec(unsigned int hostRecIndx, float age);

	/* gets host record */
	/* hostRecIndx: index in Table; pHost: pointer to Host variable through which the record is returned */
	void GetHostRec(unsigned int hostRecIndx, Country::Host *pHost);

	/* sets establishment ID (index in Establishment table) */
	/* hostRecIndx: index in Table; EstID1, EstID2: IDs of establishment */
	void SetEstablishmentID(unsigned int hostRecIndx, unsigned int estID1, unsigned int estID2);

	/* sets host group ID */
	/* hostRecIndx: index in Table; groupID: group ID */
	void SetHostGroupID(unsigned int hostRecIndx, unsigned int groupID);

	/* gets total number of hosts */
	unsigned int GetNumHosts();

	/* gets list of hosts residing in household */
	/* pHshld: pointer to Household object; pHostList: pointer to array of Host objects to be filled in */
	void GetHostList(Country::Household *pHshld, Country::Host *pHostList);

	/* gets random unassigned host from household */
	/* pHshld: pointer to Household object */
	unsigned short GetRndUnassgndHostIndx(Country::Household *pHshld);

	/* gets random unassigned host whose age falls within [ageLB, ageUB) from household */
	/* pHshld: pointer to Household object; ageLB and ageUB: age boundaries */
	unsigned short GetRndUnassgndHostIndx(Country::Household *pHshld, float ageLB, float ageUB);

	/* resets establishment IDs assigned to hosts */
	void ResetEstIDs();

	/* corrects for marginal population age distribution difference between model and real world data */
	/* pMADDev: pointer to deviation vector */
	void CorrectForPAGDev(MADVec *pMADDev);

	/* computes marginal model population age distribution */
	MADVec* CompMargModelPopulAgeDistr();
};



/* individual household */
class Country::Household
{
public:
	double Lat;                // Lat coordinate
	double Lon;                // Lon coordinate
	unsigned int LndScnX;      // LandScan X and Y indexes of the cell where the household is located
	unsigned int LndScnY;
	unsigned short NumRecs;    // number of records in the Table (number of hosts in the household)
	Country::Host *H;          // pointer to the first host in the household in the Hosts table

	/* default constructor; creates undefined household record */
	Household();

	/* constructor;  creates a record for household and reserves records for household members in Hosts table */
	/* hshldLat and hshldLon: lattitude and longitude coordinate; hshldLndScnX and hshldLndScnY: X and Y LandScan indexes of the cell where the household is located; */
	/* hshldSize: size of household; pHostTbl: pointer to Hosts table */
	Household(double hshldLat, double hshldLon, unsigned int hshldLndScnX, unsigned int hshldLndScnY, unsigned short hshldSize, Country::HostTbl *pHostTbl);

	/* sets age of househould members using direct sampling */
	/* pADVM: pointer to matrix of age distribution vectors */
	void SetHshldMembsAgeDir(ADVMtrx *pADVM);

	/* sets establishment ID (index in Establishment table) */
	/* hostIndx: host index within household; estID1, estID2: IDs of establishment */
	void SetEstablishmentID(unsigned int hostIndx, unsigned short estID1, unsigned int estID2);

	/* returns ID1 - characteristic of establishment type assigned in accordance with host's age group; returns UNDEFINED if no assignment has been done yet */
	/* hostIndx: host index within household */
	unsigned short GetEstablishmentID1(unsigned int hostIndx);

	/* returns establishment ID2 if host has been assigned to some establishment or UNDEFINED otherwise */
	/* hostIndx: host index within household */
	unsigned int GetEstablishmentID2(unsigned int hostIndx);

	/* returns ID of group given host belongs to */
	/* hostIndx: host index within household */
	unsigned short GetGroupID(unsigned int hostIndx);

};



/* table of households */
class Country::HouseholdTbl
{
private:
	float ExtCoeff;
	unsigned int NumRecs;        // number of nonempty records in the Table (number of households)
	unsigned int NumRecsMax;     // max. allowed number of records in the Table
	Country::Household *Table;   // pointer to the Table describing households

public:
	/* default constructor; creates undefined table */
	HouseholdTbl();

	/* constructor; creates an empty table */
	/* TblSize: size of table; extCoeff: extension coefficient for Table */
	HouseholdTbl(unsigned int tblSize, float extCoeff);

	~HouseholdTbl();

	/* adds record in Household table */
	/* pHshld: pointer to a record to be added */
	void AddRec(Country::Household *pHshld);

	/* appends a table to an existing one */
	/* pHshldTbl: pointer to household table to be appended */
	void AppendTable(Country::HouseholdTbl *pHshldTbl);

	/* resets table */
	void ResetTable();

	/* gets number of households */
	unsigned int GetNumHouseholds();

	/* gets max size of household */
	unsigned short GetMaxHshldSize();

	/* synthesizes household age structure using direct sampling */
	/* pADVM: pointer to matrix of age distribution vectors */
	void SynthesizeHouseholdAgeStructDir(ADVMtrx *pADVM);

	/* computes model household size distribution */
	Country::MdHSDVec* CompModelHshldSizeDistr();

	/* computes model household-age distribution */
	Country::MdADVMtrx* CompModelHshldAgeDistr();

	/* gets household object out of a Table */
	/* hshldRecIndx: index of Household object (record); hshld: pointer to Household object through which that record is returned */
	void GetHousehold(unsigned int hshldRecIndx, Country::Household *pHshld);

	/* corrects for marginal population age distribution difference between model and real world data */
	/* pMADDev: pointer to deviation vector; pADVM: pointer to matrix of age distribution vectors */
	void CorrectForPAGDev(MADVec *pMADDev, ADVMtrx *pADVM);
};



#endif

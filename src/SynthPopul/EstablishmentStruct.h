/* EstablishmentStruct.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* classes describing generic establishment and used to map hosts to establishments  */

#ifndef ESTABLISHMENT_STRUCT
#define ESTABLISHMENT_STRUCT



#include "Country.h"
#include "EstablishmentDistr.h"
#ifdef _OPENMP
#include "omp.h"
#endif



/* class describing generic establishment */
class Country::Establishment
{
public:
	double Lat;                      // Lat coordinate
	double Lon;                      // Lon coordinate

	unsigned int LndScnX;            // X and Y coordinates of LandScan cell
	unsigned int LndScnY;

	unsigned int NumHosts;           // number of hosts associated with Establishment
	unsigned int NumHostVacancs;     // number of vacancies

	unsigned int NumStaff;           // number of staff - people from different age group - associated with Establishment (e.g. staff at schools or nurseries)
	unsigned int NumStaffVacancs;    // number of staff vacancies

	/* fills in host vacancy; returns the number of remaining host vacancies */
	unsigned int  FillHostVacancy();

	/* fills in staff vacancy; returns the number of remaining staff vacancies */
	unsigned int FillStaffVacancy();
};



/* model establishment size distribution vector */
class Country::MdESDVec
{
public:
	/* element of vector */
	class MdESDVEl
	{
	public:
		unsigned int Size;       // establishment size
		float Prob;              // corresponding probability distribution vector value
		unsigned int NumEst;     // number of establishments (size band value)
	};

	unsigned int NumEls;         // number of elements in vector
	MdESDVEl *V;                 // vector of ESDVEl elements
	
	/* default constructor; creates undefined vector */
	MdESDVec();

	/* constructor; creates empty vector based on input establishment size distribution vector */
	/* ESDV: input establishment size distribution vector */
	MdESDVec(ESDVec *pESDV);

	~MdESDVec();
};



/* table of Establishment objects */
class Country::EstablishmentTbl
{
private:
	float ExtCoeff;
	unsigned int NumRecs;            // number of nonempty records in Table
	unsigned int NumRecsMax;         // max. allowed number of records in Table
	Country::Establishment *Table;   // table of Establishment objects
	
public:
	/* default constructor; creates undefined table */
	EstablishmentTbl();

	/* constructor; creates empty table of Establishment objects */
	/* numEst: total number of establishments; extCoeff: extension coefficient for Table */
	EstablishmentTbl(unsigned int TblSize, float extCoeff);

	~EstablishmentTbl();

	/* adds record in establishment table */
	/* pEst: pointer to the record to be added */
	void AddRec(Establishment *pEst);

	/* appends a table to an existing one */
	void AppendTable(EstablishmentTbl *pEstTbl);

	/* gets the first empty record index in Table */
	unsigned int GetFirstEmptyRecIndex();

	/* resets table */
	void ResetTable();

	/* gets Establishment object out of Table */
	/* estObjIndx: index of Establishment object (record); pEst: pointer to location in memory through which this object is returned */
	void GetEstablishmentRec(unsigned int estObjIndx, Country::Establishment *pEst);

	/* gets establishment coordinates */
	/* estObjIndx: establishment index; *pLat and *pLon: pointers to variables through which Lat and Lon are returned */
	void GetEstablishmentCoords(unsigned int estObjIndx, double *pLat, double *pLon);

	/* gets X and Y indexes of the LandScan cell that contains the establishment */
	/* estObjIndx: establishment index; *pLndScnX and *pLndScnY: pointers to the variables through which the indexes are returned */
	void GetEstablishmentLndScnIndxs(unsigned int estObjIndx, unsigned int *pLndScnX, unsigned int *pLndScnY);

	/* assigns host to establishment; returns the number of remaining host vacancies */
	/* estObjIndx: establishment index (establishment ID) */
	unsigned int AssignHost(unsigned int estObjIndx);

	/* assigns staff member to establishment; returns the number of remaining staff vacancies */
	/* estObjIndx: establishment index (establishment ID) */
	unsigned int AssignStaff(unsigned int estObjIndx);

	/* gets the total number of establishments (number of records in Table) */
	unsigned int GetNumEstablishments();

	/* gets random unoccupied establishment */
	unsigned int GetRndUnoccpdEstablishment();

	/* modifies record in Table */
	/* estObjIndx: establishment index; pEst: pointer to new Establishment object that replaces one with index estObjIndx */
	void ModifyRec(unsigned int estObjIndx, Country::Establishment *pEst);

	/* computes model establishment size distribution */
	Country::MdESDVec* CompModelEstSizeDistr(ESDVec *pESDV);
};



/* class that describes mapping hosts to establishments */
class Country::HostToEstablishmentMap
{
private:
	/* class describing geographical patch - set of LandScan cells; these patches are used in rejection acceptance sampling */
	class GeoPatch
	{
	public:
		/* vector of establishment indexes in the Establishment objects table (EstablishmentTblCl) associated with a given patch  */
		class EstablishmentIndxVec
		{
		public:
			class Indx
			{
			public:
				unsigned int ElIndx;        // index of element in vector V (see below)
				unsigned int EstObjIndx;    // establishment object index
			};

			float ExtCoeff;                 // coefficient of extension (set to 0.5)
			unsigned int NumEls;            // number of nonempty elements in V
			unsigned int NumElsMax;         // max. capacity of V
			unsigned int *V;                // vector of establishment indexes
			
			/* default constructor; creates undefined vector */
			EstablishmentIndxVec();
			
			/* constructor; creates empty vector with initial capacity EstIndxIniVecLngth and extension coefficient extCoeff */
			EstablishmentIndxVec(unsigned int EstIndxIniVecLngth, float extCoeff);

			/* copy constructor */
			EstablishmentIndxVec(const EstablishmentIndxVec& PIV);

			~EstablishmentIndxVec();

			/* assignment operator */
			EstablishmentIndxVec operator=(const EstablishmentIndxVec& PIV2);

			/* adds the establishment index (in the table of establishment objects) into vector V */
			/* returns the index (in V) of the added element */
			/* estObjIndx: index in the table of establishment objects */
			unsigned int AddEstablishmentIndx(unsigned int estObjIndx);

			/* gets establishment object index (can be the last one in V or a random one depending on implementation) */
			void GetEstablishmentIndx(Indx *pIndx);

			/* removes the establishment index from vector V returning the number of remaining indexes */
			/* pIndx: describes index of the element in V to be removed */
			unsigned int RemoveEstablishmentIndx(Indx *pIndx);

			/* resets vector */
			void Reset();
		};


		/* probability distribution vector - auxiliary distribution used in rejection-acceptance sampling */
		class PDVec
		{
		public:
			/* element of probability distribution vector */
			class PDVEl
			{
			public:
				unsigned int GeoPtchIndx;   // geographical patch index
				double CProb;               // corresponding value of the cumulative probability distribution
			};			

			unsigned int NumEls;            // number of elements in vector V
			unsigned int NumElsMax;         // max. capacity of V
			PDVEl *V;                       // vector of PDVEl elements
			
			/* default constructor; creates undefined vector */
			PDVec();

			/* constructor; creates empty vector of length PDVecLngth */
			PDVec(unsigned int PDVecLngth);

			/* copy constructor */
			PDVec(const PDVec& PDV);

			~PDVec();

			/* assignment operator */
			PDVec operator=(const PDVec& PDV2);

			/* adds probability value to the cumulative probability distribution vector */
			void AddProbVal(unsigned int geoPtchIndx, double probVal);

			/* normalizes cumulative probability vector */
			void Normalize();
		};


		/* vector of indexes of LandScan cells filling given geographical patch */
		class LndScnCellVec
		{
		public:
			unsigned int NumEls;            // number of nonempty elements in V
			unsigned int NumElsMax;         // max. capacity of V
			unsigned int *V;                // vector of LandScan cell indexes

			/* constructor; creates empty vector */
			LndScnCellVec();

			/* copy constructor */
			LndScnCellVec(const LndScnCellVec& LSCVec);

			~LndScnCellVec();

			/* assignment operator */
			LndScnCellVec operator=(const LndScnCellVec& LSCVec2);

			/* adds LandScan cell index into vector */
			void AddLandScanCellIndx(unsigned int LndScnCellIndx);
		};
		

		unsigned int GPLndScnX;            // X and Y patch coordinates represented as LandScan indexes of the upper left corner cell
		unsigned int GPLndScnY;

		EstablishmentIndxVec *PEstIndxV;   // vector of establishment indexes in the establishment objects table (EstablishmentTbl) associated with a given patch
		PDVec *PPDV;                       // auxiliary cumulative probability distribution vector
		LndScnCellVec *PLSCVec;            // vector of indexes of LandScan cells filling given geographical patch

		unsigned int NumRepHits;           // number of repeated hits during rejection acceptance sampling
		unsigned int NumHostVacancs;       // number of vacancies in the establishments located in a patch
		unsigned int NumStaffVacancs;      // number of staff vacancies in those establishments

		
		/* default constructor; creates undefined GeoPatch object */
		GeoPatch();

		/* constructor; builds a GeoPatch object */
		/* gpLndScnX and gpLndScnY: X and Y patch coordinates represented as LandScan indexes of the upper left corner cell */
		/* estIndxVecIniLngth: initial length of the vector of establishment indexes EstablishmentIndxVec; estIndxVecExtCoeff: extension coefficient of that vector */
		GeoPatch(unsigned int gpLndScnX, unsigned int gpLndScnY, unsigned int estIndxVecIniLngth, float estIndxVecExtCoeff);

		/* constructor; builds a GeoPatch object */
		/* gpLndScnX and gpLndScnY: X and Y patch coordinates represented as LandScan indexes of the upper left corner cell */
		GeoPatch(unsigned int gpLndScnX, unsigned int gpLndScnY);

		/* copy constructor */
		GeoPatch(const GeoPatch& GP);

		~GeoPatch();

		/* assignment operator */
		GeoPatch operator=(const GeoPatch& GP2);
	};


	/* vector of geographical patches */
	class GeoPatchVec
	{
	public:
		float ExtCoeff;                     // coefficient of extension (default set to 0.5)
		unsigned int NumEls;                // number of elements in V (geographical patches)
		unsigned int NumEstPtchs;           // number of geographical patches that contain establishments (these patches have indexes i = 0 ... (NumEstPtchs-1) in V)
		unsigned int NumAvailEstPtchs;      // number of geographical patches that contain _unoccupied_ establishments (NumAvailEstPtchs < = NumEstPtchs)
		unsigned int NumElsMax;             // max. capacity of V
		GeoPatch *V;                        // vector of GeoPatch elements
		
		/* default constructor; creates undefined vector */
		GeoPatchVec();

		/* constructor; creates empty vector with initial capacity gpPatchVecIniLngth and extension coefficient extCoeff */
		GeoPatchVec(unsigned int gpPatchVecIniLngth, float extCoeff);

		~GeoPatchVec();

		/* adds GeoPatch object to V */
		/* pGPtch: pointer to an object to be added */
		/* returns index of the added geographical patch in V */
		unsigned int AddGeoPatch(GeoPatch *pGPtch);

		/* returns pointer to geographical patch with index geoPatchIndx in vector V */
		GeoPatch* GetGeoPatch(unsigned int geoPatchIndx);

		/* increments the counter of geographical patches with establishments */
		void IncrEstPtchsCntr();

		/* increments the counter of geographical patches with available establishments */
		/* returns new value of counter */
		unsigned int IncrAvailEstPtchsCntr();

		/* decrements the counter of available geographical patches with establishments */
		/* returns new value of counter */
		unsigned int DecrAvailEstPtchsCntr();

		/* resets the counter of geographical patches that contain establishments  */
		void ResetEstPtchsCntr();
	};


	/* matrix that contains indexes of geographical patches in GeoPatchVec vector */
	/* used to map hosts to the corresponding patches */
	class GeoPatchIndxMtrx
	{
	public:
		unsigned int XRef;                  // coordinates of the reference point - the start of the "local" geographical patch grid
		unsigned int YRef;                  // represented as a pair of LandScan indexes of the upper left LandScan cell in the upper left patch
		unsigned int Grnlty;                // granularity of geographical patches (number of LandScan cells in each dimension)
		unsigned int DimX;                  // X and Y (column and row) dimensions of M
		unsigned int DimY;
		unsigned int **M;                   // matrix
		
		/* constructor; builds empty matrix */
		GeoPatchIndxMtrx(unsigned int xRef, unsigned int yRef, unsigned int gpGrnlty, unsigned int dimX, unsigned int dimY);

		~GeoPatchIndxMtrx();

		/* calculates geographical patch index X for a LandScan cell with index LndScnX */
		unsigned int LndScnXToX(unsigned int LndScnX);

		/* calculates geographical patch index Y for a LandScan cell with index LndScnY */
		unsigned int LndScnYToY(unsigned int LndScnY);

		/* calculates X coordinate (LandScan X index of the LandScan cell in the upper left patch corner) of the geographical patch where the LandScan cell with X coordinate LndScnCellX is located */
		unsigned int LndScnCellXToGPLdnScnX(unsigned int LndScnCellX);

		/* calculates Y coordinate (LandScan Y index of the LandScan cell in the upper left patch corner) of the geographical patch where the LandScan cell with Y coordinate LndScnCellY is located */
		unsigned int LndScnCellYToGPLdnScnY(unsigned int LndScnCellY);

		/* returns distance between the geographical patches with indexes (x1, y1) and (x2, y2) */
		/* pCLSCTbl: pointer to the LandScan cell table for the country */
		double PatchDist(CntryLndScnCellTbl *pCLSCTbl, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
	};
	

	/* table setting index reference between Establishment table and Host table */
	class HostToEstIndexTbl
	{
	public:
		/* index entry */
		class IndexEntry
		{
		public:
			unsigned int NumEls;      // number of elements in index array Indx
			unsigned int NumElsMax;   // number of elements in Indx - establishment capacity (number of hosts + staff)
			unsigned int *Indx;       // array of host indexes (in Host table)


			/* default constructor */
			IndexEntry();

			/* constructor; builds empty index array of numElsMax elements */
			IndexEntry(unsigned int numElsMax);

			~IndexEntry();

			/* adds host index into establishment index entry */
			/* hostIndx: index of host in Host table */
			void AddIndex(unsigned int hostIndx);

			/* extracts random host index from establishment index entry */
			unsigned int ExtractRndIndex();
		};

		unsigned int NumElsMax;   // max. capacity of table
		IndexEntry *Table;        // pointer to array of index entries

		/* constructor; builds empty table of numElsMax entries */
		HostToEstIndexTbl(unsigned int numElsMax);

		~HostToEstIndexTbl();

		/* adds empty index entry into table */
		/* estIndx: establishment index in Table; estCapacty: establishment capacity */
		void AddEmptyIndexEntry(unsigned int estIndx, unsigned int estCapacty);

		/* gets index entry */
		/* estIndx: establishment index in Table */
		IndexEntry* GetIndxEntry(unsigned int estIndx);
	};


	GeoPatchVec *PGPVec;                       // vector of geographical patches
	GeoPatchIndxMtrx *PGPIndxMtrx;             // matrix of geographical patch indexes
	HostToEstTravelDistrKernel *PHETDKernel;   // pointer to the kernel function object containing different kernel function types
	PointerToKernFunc F;                       // pointer to the chosen kernel function
	HostToEstIndexTbl *PHEIndxTbl;             // Host->Establishment index table
	
	
	/* (re)builds auxiliary probability distribution vectors, PDVec, for host vacancies from the current age group in each geographical patch */
	void BuildHostPDVs(CntryLndScnCellTbl *pCLSCTbl);

	/* (re)builds auxiliary probability distribution vectors for _staff_ vacancies, PDVec, in each geographical patch */
	void BuildStaffPDVs(CntryLndScnCellTbl *pCLSCTbl);
	
	/* samples a patch; used in the rejection-acceptance establishment sampling */
	/* x and y are coordinates (indexes in GeoPatchIndxMtrx matrix) of geographical patch for which sampling is done */
	unsigned int SamplePatch(unsigned int x, unsigned int y);

	/* creates Establishment object and adds it to Establishment table */
	/* estGPIndx: index of a geographical patch (in the vector GeoPatchVec) where an establishment is to be located; */
	/* pCLSCTbl: pointer to the LandScan cell table for the country; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
	void CreateEstablishment(unsigned int estGPIndx, CntryLndScnCellTbl *pCLSCTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl);

	/* creates table of references between Establishment table and Host table */
	void CreateHostToEstIndexTbl(Country::EstablishmentTbl *pEstTbl, Country::HostTbl *pHostTbl);

	/* deletes table of references created by the previous method */
	void DeleteHostToEstIndexTbl();

public:
	/* constructor; builds geographical patch index matrix and vector of geographical patches for allocating HOSTS from the target age group to establishments */
	/* pCLSCTbl: LandScan cell table for the country; GPGrnlty: granularity of geographical patches (number of LandScan cells in each dimension); pEstTbl: table of Establishment objects */
	/* pHETDKernel: kernel function object */
	HostToEstablishmentMap(CntryLndScnCellTbl *pCLSCTbl, unsigned int GPGrnlty, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel);

	/* constructor; builds geographical patch index matrix and vector of geographical patches for allocating STAFF (hosts from a selected age group, different from the target one) to establishments */
	/* hosts are assumed to have been previously allocated to establishments */
	/* pCLSCTbl: LandScan cell table for the country; GPGrnlty: granularity of geographical patches (number of LandScan cells in each dimension); pESDV: pointer to the establishment size distribution vector */
	/* pEstTbl: table of Establishment objects; pHETDKernel: kernel function object */
	HostToEstablishmentMap(CntryLndScnCellTbl *pCLSCTbl, unsigned int GPGrnlty, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel);

	~HostToEstablishmentMap();
	
	/* assigns hosts to establishments */
	/* pCLSCTbl: pointer to the LandScan cell table for the country; pHostTbl: pointer to the table of hosts; */
	/* pHshldTbl: pointer to the Household table; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
	void AssignHostsToEstablishments(CntryLndScnCellTbl *pCLSCTbl, Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl);

	/* assigns staff to establishments */
	/* pCLSCTbl: pointer to the LandScan cell table for the country; pHostTbl: pointer to the table of hosts; */
	/* pHshldTbl: pointer to the Household table; pESDV: pointer to the establishment size distribution vector; pEstTbl: pointer to the table of establishments */
	void AssignStaffToEstablishments(CntryLndScnCellTbl *pCLSCTbl, Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl);

	/* creates groups of hosts within establishments for current age group */
	/* pHostTbl: pointer to the table of hosts, pESDV: pointer to the establishment size distribution vector, pEstTbl: pointer to the table of establishments */
	void CreateHostGroups(Country::HostTbl *pHostTbl, ESDVec *pESDV, Country::EstablishmentTbl *pEstTbl);

	/* computes model host-to-establishment travel statistics */
	/* pHostTbl: pointer to the table of hosts; pHshldTbl: pointer to the household table; pEstTbl: pointer to the table of establishments */
	HETDVec* CompHostToEstablishmentTravelStat(Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, Country::EstablishmentTbl *pEstTbl);

	/* computes model host-to-establishment travel statistics for specific age group */
	/* pHostTbl: pointer to the table of hosts; pHshldTbl: pointer to the household table; pEstTbl: pointer to the table of establishments; */
	/* pHETDKernel: pointer to host-to-establishment travel distribution kernel; ageGroupIndx: age group index */
	HETDVec* CompHostToEstablishmentTravelStat(Country::HostTbl *pHostTbl, Country::HouseholdTbl *pHshldTbl, Country::EstablishmentTbl *pEstTbl, HostToEstTravelDistrKernel *pHETDKernel, unsigned short ageGroupIndx);
};



#endif

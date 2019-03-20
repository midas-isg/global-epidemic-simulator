/* Main.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "EstablishmentDistr.h"
#include "Country.h"
#include "Globals.h"



int main(int argc, char* argv[])
{
	if( argc != COMM_PARAMS_NUM )
	{
		cout << "Command line syntax: <name of the executable> <LandScan header file name> <LandScan binary dataset file name> \
				<country border raster dataset header file name> <country border raster binary dataset file name> \
				<household size distribution matrix file name> <joint household size-household members' age distribution matrix file name> \
				<marginal population age distribution matrix file name> <host-to-establishment travel distribution parameter file name> <establishment size distribution file name> \
				<country code> <scale-down factor> <output file name for the table of synthesized population> <output file name for the table of establishments>";
		exit(EXIT_FAILURE);
	}

	unsigned int i=1;
	
	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "LandScan header file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(LSHdrName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "LandScan binary dataset file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(LSFileName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Country border raster dataset header file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(CBRHdrName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Country border raster binary dataset file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(CBRFileName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Household size distribution matrix file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(SDVecFName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Household members' age distribution matrix file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(ADMtrxFName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Marginal age distribution file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(MADFName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Host-to-establishment travel distribution parameter file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(HostToEstTDFName, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Establishment size distribution file path is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(ESDFName, argv[i++]);

	unsigned int CountryCode;
	sscanf(argv[i++], "%u", &CountryCode);

	float SclDwnFactor;
	sscanf(argv[i++], "%e", &SclDwnFactor);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Output file path for the table of synthesized population is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(OutpSynthPopulTblFNameTmpl, argv[i++]);

	if( strlen(argv[i]) > _MAX_PATH )
	{
		cerr << "Output file path for the table of establishments is too long.\n";
		exit(EXIT_FAILURE);
	}
	strcpy(OutpSynthEstablshmntTblFNameTmpl, argv[i++]);


	try
	{
		/* Landscan table object */
		LndScnCellTbl *pLSCT;
		try
		{
			pLSCT = new LndScnCellTbl(LSHdrName, LSFileName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << LSHdrName << " or " << LSFileName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in LndScnCellTbl class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
		
		
		/* raster table of country territory */
		RstrCellTbl *pRCT;
		try
		{
			pRCT = new RstrCellTbl(CountryCode, CBRHdrName, CBRFileName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << CBRHdrName << " or " << CBRFileName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in RstrCellTbl class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
		
		
		/* Landsan table of country territory */
		CntryLndScnCellTbl *pCLSCT;
		try
		{
			pCLSCT = new CntryLndScnCellTbl(CountryCode, pLSCT, pRCT);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case NO_COUNTRY_FOUND:
				cerr << "*** No country corresponding to the code " << CountryCode << " found (CRITICAL ERROR). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in LndScnCellTbl class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
	

		delete pLSCT;
		delete pRCT;


		/* object containing household size distribution vector */
		HSDVec *pHSDV;
		try
		{
			pHSDV = new HSDVec(SDVecFName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << SDVecFName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			case INCORRECT_DISTR_VECTOR:
				cerr << "*** Error: Distribution vector in file " << SDVecFName << " does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in HSDVec class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}

		
		/* object containing household members' age distribution vectors */
		ADVMtrx *pADVM;
		try
		{
			pADVM = new ADVMtrx(ADMtrxFName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << ADMtrxFName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			case INCORRECT_DISTR_VECTOR:
				cerr << "*** Error: Distribution vector in file " << ADMtrxFName << " does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in ADVMtrx class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}


		/* object containing establishment size distribution vector */
		ESDVec *pESDV;
		try
		{
			pESDV = new ESDVec(ESDFName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << ESDFName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			case INCORRECT_DISTR_VECTOR:
				cerr << "*** Error: Distribution vector in file " << ESDFName << " does not satisfy key constraints (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in ESDVec class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}


		/* host-to-establishment travel distribution kernel function (or choice kernel function) */
		HostToEstTravelDistrKernel *pHETDKernel;
		try
		{
			pHETDKernel = new HostToEstTravelDistrKernel(HostToEstTDFName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << HostToEstTDFName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in HostToEstTravelDistrKernel class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
	
		
		
		/* country object */
		Country C = Country(CountryCode, SclDwnFactor, pCLSCT, pHSDV, pADVM, pESDV, pHETDKernel);
	
		C.SynthesizeHshlds();                                                    // synthesize household structure by direct sampling
		C.CompModelHshldSizeDistr();                                             // compute and output model household size distribution
		C.SynthesizePopulAgeStructDir();                                         // synthesizes population age structure by direct sampling
		C.CompModelHshldAgeDistr();                                              // computes and outputs model household-age distribution
		C.CompMargModelPopulAgeDistr();                                          // compute and output marginal model population age distribution

		
		/* object containing marginal population age distribution vector */
		MADVec *pRWDataMADVec;
		try
		{
			pRWDataMADVec = new MADVec(MADFName);
		}
		catch(int ExcCode)
		{
			switch(ExcCode)
			{
			case FILE_OPEN_ERROR:
				cerr << "*** Error opening file " << MADFName << " (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			default:
				cerr << "*** Unidentified error in MADVec class constructor (CRITICAL). Program will be terminated. ***\n";
				exit(EXIT_FAILURE);
			}
		}
		
		C.CorrectMADDev(pRWDataMADVec, pADVM);                                   // correct for marginal population age distribution differences between model and real world data
		delete pRWDataMADVec;


		C.CompModelHshldAgeDistr();                                              // compute and output model household-age distribution after correction
		C.CompMargModelPopulAgeDistr();                                          // compute and output marginal model population age distribution after correction

		C.SynthesizeVacancyGeoMap();                                             // synthesizes geo map of vacancies
		C.SynthesizeVacancyLocDistr();                                           // synthesizes geo distribution of vacancy locations based on the vacancy map
		for(unsigned short i=0; i < C.GetParamSetNum(); i++)                     // loop over parameter sets
		{
			for(unsigned short j=0; j < C.GetAgeGroupNum(); j++)                 // loop over age groups
			{
				C.SetAgeGroupParamSetIndxs(j, i);                                // set age group and parameter set indexes
				C.SynthesizeVacancies();                                         // prepare map of vacancies
			
				C.CreateHostToEstablishmentMap();                                // create host-to-establishment map object
				C.AssignHostsToEstablishments();                                 // assign hosts to establishments
				C.CreateHostGroups();                                            // distribute hosts to groups within synthesized establishments
				C.CompModelEstablishmentSizeDistr();                             // compute model establishment size distribution
				C.CompModelHostToEstablishmentTravelStatAG();                    // compute host-to-establishment travel statistics

				C.CreateStaffToEstablishmentMap();                               // create staff-to-establishment map object
				C.AssignStaffToEstablishments();                                 // assign staff to already existing establishments
				C.CompModelStaffToEstablishmentTravelStatAG();                   // compute staff-to-establishment travel statistics

				C.OutputSynthEstablishments(OutpSynthEstablshmntTblFNameTmpl);   // output synthesized table of establishment objects
			}
			C.OutputSynthPopulation(OutpSynthPopulTblFNameTmpl);                 // output synthetic population table
		
			C.ResetHostEstIDs();                                                 // reset IDs mapping hosts to establishments
		}
	
		delete pESDV;
		delete pADVM;
		delete pHSDV;
		delete pCLSCT;
	}
	catch(...)
	{
		cout.flush();   // if an unhandled exception occurs, flush output buffers
		cerr.flush();
		throw;          // and rethrow it
	}
}

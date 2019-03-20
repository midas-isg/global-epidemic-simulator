/* EstablishmentDistr.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "Util.h"
#include "EstablishmentDistr.h"
#ifdef _OPENMP
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#endif




/* constructor; creates empty vector */
/* ageGroupNum: number of age groups */
HostToEstTravelDistrKernel::AgeGroupVec::AgeGroupVec(unsigned short ageGroupNum) : AgeGroupNum(ageGroupNum)
{
	V = new AgeRange[AgeGroupNum];

	for(unsigned int i=0; i < AgeGroupNum; i++)
	{
		V[i].LB = 0.0f;
		V[i].UB = 0.0f;
	}
}



HostToEstTravelDistrKernel::AgeGroupVec::~AgeGroupVec()
{
	if( V != NULL )
		delete [] V;
}



/* constructor; creates empty vector */
/* paramSetNum: number of parameter sets (blocks); ageGroupNum: number of age groups */
HostToEstTravelDistrKernel::ParamSetMtrx::ParamSetMtrx(unsigned short paramSetNum, unsigned short ageGroupNum) : ParamSetNum(paramSetNum), AgeGroupNum(ageGroupNum)
{
	M = new ParamSet*[ParamSetNum];

	for(unsigned int i=0; i < ParamSetNum; i++)
	{
		M[i] = new ParamSet[AgeGroupNum];
	}

	for(unsigned int i=0; i < ParamSetNum; i++)
	{
		for(unsigned int j=0; j < AgeGroupNum; j++)
		{
			M[i][j].FCode = 0;
			M[i][j].a0 = 0.0;
			M[i][j].a1 = 0.0;
			M[i][j].b0 = 0.0;
			M[i][j].b1 = 0.0;
			M[i][j].c1 = 0.0;
		}
	}
}



HostToEstTravelDistrKernel::ParamSetMtrx::~ParamSetMtrx()
{
	if( M != NULL )
	{
		for(unsigned int i=0; i < ParamSetNum; i++)
			delete [] M[i];
		delete [] M;
	}
}



#define GEN_PARAMS_DB_INDX                    0
#define AGE_GROUPS_DB_INDX                    1
#define AGE_GROUP_PARAM_BLOCK_START_DB_INDX   2

/* constructor */
/* fName: name of the file that contains list of kernel function codes and the corresponding parameters */
HostToEstTravelDistrKernel::HostToEstTravelDistrKernel(char *fName)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** HostToEstTravelDistrKernel::HostToEstTravelDistrKernel(char*) started...\n";
#endif

	DataIn Data = DataIn(fName);

	DataBlock *pDB = Data.ReadDataBlock(GEN_PARAMS_DB_INDX);
	unsigned short ageGroupNum = pDB->Mtrx[0][0];                  // element [0][0]: number of age groups
	unsigned short paramSetNum = pDB->Mtrx[0][1];                  // element [0][1]: number of parameter sets (blocks)
	delete pDB;

	PAgeGroupV = new AgeGroupVec(ageGroupNum);
	pDB = Data.ReadDataBlock(AGE_GROUPS_DB_INDX);
	for(unsigned int i=0; i < PAgeGroupV->AgeGroupNum; i++)
	{
		PAgeGroupV->V[i].LB = pDB->Mtrx[0][i];
		PAgeGroupV->V[i].UB = pDB->Mtrx[0][i+1];
	}
	delete pDB;

	PParamSetMRW = new ParamSetMtrx(1, ageGroupNum);
	PParamSetM = new ParamSetMtrx(paramSetNum, ageGroupNum);
	for(unsigned int i=0; i < PParamSetM->AgeGroupNum; i++)
	{
		pDB = Data.ReadDataBlock(AGE_GROUP_PARAM_BLOCK_START_DB_INDX + i);
		PParamSetMRW->M[0][i].FCode = pDB->Mtrx[0][0];
		PParamSetMRW->M[0][i].a0 = pDB->Mtrx[0][1];
		PParamSetMRW->M[0][i].b0 = pDB->Mtrx[0][2];
		PParamSetMRW->M[0][i].a1 = pDB->Mtrx[0][3];
		PParamSetMRW->M[0][i].b1 = pDB->Mtrx[0][4];
		PParamSetMRW->M[0][i].c1 = pDB->Mtrx[0][5];
		PParamSetMRW->M[0][i].OutputFlag = pDB->Mtrx[0][6];
		for(unsigned int j=0; j < PParamSetM->ParamSetNum; j++)
		{
			PParamSetM->M[j][i].FCode = pDB->Mtrx[j+1][0];
			PParamSetM->M[j][i].a0 = pDB->Mtrx[j+1][1];
			PParamSetM->M[j][i].b0 = pDB->Mtrx[j+1][2];
			PParamSetM->M[j][i].a1 = pDB->Mtrx[j+1][3];
			PParamSetM->M[j][i].b1 = pDB->Mtrx[j+1][4];
			PParamSetM->M[j][i].c1 = pDB->Mtrx[j+1][5];
			PParamSetM->M[j][i].OutputFlag = pDB->Mtrx[j+1][6];
		}
		delete pDB;
	}

	CrntAgeGroupIndx = 0;
	CrntParamSetIndx = 0;
	FCode = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].FCode;
	a0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a0;
	b0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b0;
	a1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a1;
	b1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b1;
	c1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].c1;

	PointerToKernFuncArray[0] = &HostToEstTravelDistrKernel::F0;
	PointerToKernFuncArray[1] = &HostToEstTravelDistrKernel::F1;

#ifdef OUTPUT_LOG_L1
	cout << PAgeGroupV->AgeGroupNum << " age groups in the model with " << PParamSetM->ParamSetNum << " parameter sets in each group\n";
	for(unsigned short i=0; i < PAgeGroupV->AgeGroupNum; i++)
	{
		cout << "Age group: " << i << " (" << PAgeGroupV ->V[i].LB << "-" << PAgeGroupV->V[i].UB << " years)\n";
		cout << "Approximation of the real world data: a0 = " << PParamSetMRW->M[0][i].a0 << ", b0 = " << PParamSetMRW->M[0][i].b0 \
			<< ", a1 = " << PParamSetMRW->M[0][i].a1 << ", b1 = " << PParamSetMRW->M[0][i].b1 << ", c1 = " << PParamSetMRW->M[0][i].c1 \
			<< "; kernel function code: " << PParamSetMRW->M[0][i].FCode << "\n";
		for(unsigned short j=0; j < PParamSetM->ParamSetNum; j++)
			cout << "Parameter set: " << j << "; a0 = " << PParamSetM->M[j][i].a0 << ", b0 = " << PParamSetM->M[j][i].b0 \
			<< ", a1 = " << PParamSetM->M[j][i].a1 << ", b1 = " << PParamSetM->M[j][i].b1 << ", c1 = " << PParamSetM->M[j][i].c1 \
			<< "; kernel function code: " << PParamSetM->M[j][i].FCode << "\n";
	}
	cout << "*** HostToEstTravelDistrKernel::HostToEstTravelDistrKernel(char*) completed\n\n";
#endif
}



HostToEstTravelDistrKernel::~HostToEstTravelDistrKernel()
{
	delete PAgeGroupV;
	delete PParamSetMRW;
	delete PParamSetM;
}



/* set of different kernel functions types */
double HostToEstTravelDistrKernel::F0(double r)
{
	double F = 1.0/(1.0 + pow(r/a0, b0)) + c1/(1.0 + pow(r/a1, b1));

	return F;
}


double HostToEstTravelDistrKernel::F1(double r)
{
	double F = 1.0/pow(1.0 + r/a0, b0) + c1/pow(1.0 + r/a1, b1);

	return F;
}



/* returns the current pointer to the kernel function set for a specific age group and a parameter set */
PointerToKernFunc HostToEstTravelDistrKernel::F()
{
	FCode = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].FCode;
	a0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a0;
	b0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b0;
	a1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a1;
	b1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b1;
	c1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].c1; 

	return PointerToKernFuncArray[FCode];
}



/* returns the pointer to the kernel function that approximates the real world travel data (host-to-establishment travel statistics) */
PointerToKernFunc HostToEstTravelDistrKernel::F_rw()
{
	FCode = PParamSetMRW->M[0][CrntAgeGroupIndx].FCode;
	a0 = PParamSetMRW->M[0][CrntAgeGroupIndx].a0;
	b0 = PParamSetMRW->M[0][CrntAgeGroupIndx].b0;
	a1 = PParamSetMRW->M[0][CrntAgeGroupIndx].a1;
	b1 = PParamSetMRW->M[0][CrntAgeGroupIndx].b1;
	c1 = PParamSetMRW->M[0][CrntAgeGroupIndx].c1;

	return PointerToKernFuncArray[FCode];
}



/**/
unsigned short HostToEstTravelDistrKernel::GetAgeGroupNum()
{
	return PParamSetM->AgeGroupNum;
}



/* gets the number of the kernel function parameter sets */
unsigned short HostToEstTravelDistrKernel::GetParamSetNum()
{
	return PParamSetM->ParamSetNum;
}



/* sets current age group index */
void HostToEstTravelDistrKernel::SetAgeGroupIndx(unsigned short ageGroupIndx)
{
	if( ageGroupIndx >= PAgeGroupV->AgeGroupNum )
		throw INDX_OUT_OF_RANGE;

	CrntAgeGroupIndx = ageGroupIndx;

	FCode = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].FCode;
	a0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a0;
	a1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a1;
	b0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b0;
	b1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b1;
	c1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].c1;
}



/* sets current parameter set index */
void HostToEstTravelDistrKernel::SetParamSetIndx(unsigned short paramSetIndx)
{
	if( paramSetIndx >= PParamSetM->ParamSetNum )
		throw INDX_OUT_OF_RANGE;

	CrntParamSetIndx = paramSetIndx;

	FCode = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].FCode;
	a0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a0;
	a1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].a1;
	b0 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b0;
	b1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].b1;
	c1 = PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].c1;
}



/* gets the index of the current age group */
unsigned short HostToEstTravelDistrKernel::GetCrntAgeGroupIndx()
{
	return CrntAgeGroupIndx;
}



/* returns the index of an age group for a specific host's age */
/* age: host's age */
unsigned short HostToEstTravelDistrKernel::GetAgeGroupIndx(float age)
{
	for(unsigned short i=0; i < PAgeGroupV->AgeGroupNum; i++)
	{
		if( (age >= PAgeGroupV->V[i].LB) && (age < PAgeGroupV->V[i].UB) )
			return i;
	}

	throw INDX_OUT_OF_RANGE;
}



/* gets the current lower age group boundary */
float HostToEstTravelDistrKernel::GetCrntLowerAgeGroupBoundary()
{
	return PAgeGroupV->V[CrntAgeGroupIndx].LB;
}



/* returns lower age boundary for specific age group index */
/* ageGroupIndx: age group index */
float HostToEstTravelDistrKernel::GetLowerAgeGroupBoundary(unsigned short ageGroupIndx)
{
	if( ageGroupIndx >= PAgeGroupV->AgeGroupNum )
		throw INDX_OUT_OF_RANGE;

	return PAgeGroupV->V[ageGroupIndx].LB;
}



/* gets the current upper age group boundary */
float HostToEstTravelDistrKernel::GetCrntUpperAgeGroupBoundary()
{
	return PAgeGroupV->V[CrntAgeGroupIndx].UB;
}



/* returns upper age boundary for specific age group index */
/* ageGroupIndx: age group index */
float HostToEstTravelDistrKernel::GetUpperAgeGroupBoundary(unsigned short ageGroupIndx)
{
	if( ageGroupIndx >= PAgeGroupV->AgeGroupNum )
		throw INDX_OUT_OF_RANGE;

	return PAgeGroupV->V[ageGroupIndx].UB;
}



/* gets the index of the kernel function parameter set */
unsigned short HostToEstTravelDistrKernel::GetCrntParamSetIndx()
{
	return CrntParamSetIndx;
}



/* gets the curent kernel function code */
unsigned short HostToEstTravelDistrKernel::GetKernelFunctionCode()
{
	return FCode;
}



/* gets parameter a0 */
double HostToEstTravelDistrKernel::GetParamA0()
{
	return a0;
}



/* gets parameter b0 */
double HostToEstTravelDistrKernel::GetParamB0()
{
	return b0;
}



/* gets parameter a1 */
double HostToEstTravelDistrKernel::GetParamA1()
{
	return a1;
}



/* gets parameter b1 */
double HostToEstTravelDistrKernel::GetParamB1()
{
	return b1;
}



/* gets parameter c1 */
double HostToEstTravelDistrKernel::GetParamC1()
{
	return c1;
}



/* gets the output flag for the current kernel function parameter set */
char HostToEstTravelDistrKernel::GetOutputFlag()
{
	return PParamSetM->M[CrntParamSetIndx][CrntAgeGroupIndx].OutputFlag;
}



/* constructor; builds establishment size distribution vector */
/* fName: name of file that contains vector */
ESDVec::ESDVec(char *fName) : AgeGroupIndx(0)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ESDVec::ESDVec(char*) started...\n";
#endif

	pData = new DataIn(fName);

	DataBlock *pDB = pData->ReadDataBlock(AgeGroupIndx * 2);
	NumEls = pDB->NumRows + 1;
	unsigned int i_ = 0;
	while( pDB->Mtrx[i_][1] == 0 )
	{
		if( i_ == NumEls - 1 )
			break;
		i_++;
	}
	NumEls -= i_;
	try
	{
		V = new ESDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of ESDVec::ESDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	if( i_ == 0)
	{
		V[0].Size = 1;
		V[0].CProb = 0.0f;
	}
	else
	{
		V[0].Size = (unsigned int)pDB->Mtrx[i_-1][0];
		V[0].CProb = pDB->Mtrx[i_-1][1];
	}
	for(unsigned int i=1; i < NumEls; i++, i_++)
	{
		V[i].Size = (unsigned int)pDB->Mtrx[i_][0];
		V[i].CProb = pDB->Mtrx[i_][1];
	}
	if( ABS(V[NumEls-1].CProb - 1.0f) > TOLER_VAL )
		throw INCORRECT_DISTR_VECTOR;
	V[NumEls-1].CProb = 1.0f + TOLER_VAL;
	delete pDB;

	pDB = pData->ReadDataBlock(AgeGroupIndx * 2 + 1);
	EstParams.StaffAgeGroupIndx = pDB->Mtrx[0][0];
	EstParams.StaffRatio = pDB->Mtrx[1][0];
	EstParams.AvGroupSize = pDB->Mtrx[2][0];
	EstParams.FillRatio = pDB->Mtrx[3][0];
	delete pDB;

#ifdef OUTPUT_LOG_L1
	cout << "*** ESDVec::ESDVec(char*) completed.\n\n";
#endif
}



ESDVec::~ESDVec()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ESDVec::~ESDVec() started...\n";
#endif

	if( pData != NULL )
	{
		delete pData;
		pData = NULL;
	}

	if( V != NULL )
	{
		delete [] V;
		V = NULL;
	}

#ifdef OUTPUT_LOG_L1
	cout << "*** ESDVec::~ESDVec() completed.\n\n";
#endif
}



/* sets age group id */
void ESDVec::SetAgeGroupIndx(unsigned short ageGroupIndx)
{
	if( V != NULL )
		delete [] V;

	AgeGroupIndx = ageGroupIndx;

	DataBlock *pDB = pData->ReadDataBlock(AgeGroupIndx * 2);
	NumEls = pDB->NumRows + 1;
	try
	{
		V = new ESDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of ESDVec::ESDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	V[0].Size = 1;
	V[0].CProb = 0.0f;

	for(unsigned int i=1; i < NumEls; i++)
	{
		V[i].Size = (unsigned int)pDB->Mtrx[i-1][0];
		V[i].CProb = pDB->Mtrx[i-1][1];
	}
	if( ABS(V[NumEls-1].CProb - 1.0f) > TOLER_VAL )
		throw INCORRECT_DISTR_VECTOR;
	V[NumEls-1].CProb = 1.0f + TOLER_VAL;
	delete pDB;

	pDB = pData->ReadDataBlock(AgeGroupIndx * 2 + 1);
	EstParams.StaffAgeGroupIndx = pDB->Mtrx[0][0];
	EstParams.StaffRatio = pDB->Mtrx[1][0];
	EstParams.AvGroupSize = pDB->Mtrx[2][0];
	EstParams.FillRatio = pDB->Mtrx[3][0];
	delete pDB;
}



/* samples establishment size */
unsigned int ESDVec::SampleSize()
{
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

	return V[rIndx].Size - (unsigned int)((V[rIndx].Size - V[lIndx].Size) * ranf_());
}



/* gets current age group index */
unsigned short ESDVec::GetAgeGroupIndx()
{
	return AgeGroupIndx;
}



/* gets staff age group index */
float ESDVec::GetStaffAgeGroupIndx()
{
	return EstParams.StaffAgeGroupIndx;
}



/* gets staff ratio */
float ESDVec::GetStaffRatio()
{
	return EstParams.StaffRatio;
}



/* gets average group size */
unsigned short ESDVec::GetAvGroupSize()
{
	return EstParams.AvGroupSize;
}



/* gets fill ratio */
float ESDVec::GetFillRatio()
{
	return EstParams.FillRatio;
}



/* default constructor; creates undefined vector */
HETDVec::HETDVec() : NumHosts(0), NumHostsTotal(0), NumEls(0), V(NULL)
{
}



/* constructor; builds empty vector */
/* HETDVecLngth: length of vector */
HETDVec::HETDVec(unsigned int HETDVecLngth) : NumHosts(0), NumHostsTotal(0), NumEls(HETDVecLngth)
{
	try
	{
		 V = new HETDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of HETDVec::HETDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned int i=0; i < NumEls; i++)
	{
		V[i].Dist = (i+1)*TRAVEL_DIST_BAND;
		V[i].Prob = 0.0;
		V[i].NumHosts = 0;
	}
}



/* constructor; builds vector using supplied kernel function */
/* HETDVecLngth: length of vector; F: pointer to the kernel function */
HETDVec::HETDVec(unsigned int HETDVecLngth, HostToEstTravelDistrKernel *pHETDKernel, PointerToKernFunc F) : NumHosts(0), NumHostsTotal(0), NumEls(HETDVecLngth)
{
	try
	{
		 V = new HETDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of HETDVec::HETDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	double normConst = 0.0f;
	for(unsigned int i=0; i < NumEls; i++)
	{
		V[i].Dist = (i+1)*TRAVEL_DIST_BAND;
		V[i].Prob = (pHETDKernel->*F)(V[i].Dist);
		V[i].NumHosts = 0;

		normConst += V[i].Prob;
	}
	for(unsigned int i=0; i < NumEls; i++)
		V[i].Prob /= normConst;
}



HETDVec::~HETDVec()
{
	if( V != NULL )
		delete [] V;
}

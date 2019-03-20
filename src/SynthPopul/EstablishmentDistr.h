/* EstablishmentDistr.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* establishment distribution classes */

#ifndef ESTABLISHMENT_DISTR
#define ESTABLISHMENT_DISTR


#include "Util.h"



/* host-to-establishment travel distribution kernel function */
class HostToEstTravelDistrKernel;
typedef double(HostToEstTravelDistrKernel::*PointerToKernFunc)(double);

#define NUM_KERNEL_FUNC_FORMS 3       // number of kernel function types in the array (see below)

class HostToEstTravelDistrKernel
{
private:
	class AgeGroupVec                 // class describing vector of age groups
	{
	public:
		class AgeRange
		{
		public:
			float LB;                 // lower age boundary
			float UB;                 // upper age boundary
		};

		unsigned short AgeGroupNum;   // number of age groups
		AgeRange *V;                  // vector of age group ranges

		/* constructor; creates empty vector */
		/* ageGroupNum: number of age groups */
		AgeGroupVec(unsigned short ageGroupNum);

		~AgeGroupVec();
	};
	
	AgeGroupVec *PAgeGroupV;          // vector of age groups
	unsigned short CrntAgeGroupIndx;  // current age group index


	class ParamSetMtrx                // class describing parameter set matrix as Matrix[index_of_parameter_set][index_of_age_group]
	{
	public:
		class ParamSet
		{
		public:
			unsigned short FCode;     // kernel function code
			double a0;                // kernel function parameters
			double b0;
			double a1;
			double b1;
			double c1;
			char OutputFlag;          // output flag: 1 - output produced, 0 - output not produced
		};

		unsigned short ParamSetNum;   // number of parameter sets (blocks)
		unsigned short AgeGroupNum;   // number of age groups
		ParamSet **M;                 // matrix of parameter sets

		/* constructor; creates empty vector */
		/* paramSetNum: number of parameter sets (blocks); ageGroupNum: number of age groups */
		ParamSetMtrx(unsigned short paramSetNum, unsigned short ageGroupNum);

		~ParamSetMtrx();
	};

	ParamSetMtrx *PParamSetMRW;       // matrix of parameter sets approximating real world data
	ParamSetMtrx *PParamSetM;         // matrix of model parameter sets

	unsigned short CrntParamSetIndx;  // current parameter set index
	unsigned short FCode;             // current kernel function code
	double a0;                        // current values of the kernel function parameters
	double b0;
	double a1;
	double b1;
	double c1;

	/* set of different kernel functions types */
	double F0(double r);
	double F1(double r);
	/* to be continued here... */

	PointerToKernFunc PointerToKernFuncArray[NUM_KERNEL_FUNC_FORMS];         // array of pointers to kernel functions

public:
	/* constructor */
	/* fName: name of the file that contains list of kernel function codes and the corresponding parameters */
	HostToEstTravelDistrKernel(char *fName);

	~HostToEstTravelDistrKernel();

	/* returns the current pointer to the kernel function set for a specific age group and a parameter set */
	PointerToKernFunc F();

	/* returns the pointer to the kernel function that approximates the real world travel data (host-to-establishment travel statistics) */
	PointerToKernFunc F_rw();

	/* gets the number of age groups */
	unsigned short GetAgeGroupNum();

	/* gets the number of parameter sets */
	unsigned short GetParamSetNum();

	/* sets current age group index */
	void SetAgeGroupIndx(unsigned short ageGroupIndx);

	/* sets current parameter set index */
	void SetParamSetIndx(unsigned short paramSetIndx);

	/* gets the index of the current age group */
	unsigned short GetCrntAgeGroupIndx();

	/* returns the index of an age group for a specific host's age */
	/* age: host's age */
	unsigned short GetAgeGroupIndx(float age);

	/* gets the current lower age group boundary */
	float GetCrntLowerAgeGroupBoundary();

	/* returns lower age boundary for specific age group index */
	/* ageGroupIndx: age group index */
	float GetLowerAgeGroupBoundary(unsigned short ageGroupIndx);

	/* gets the current upper age group boundary */
	float GetCrntUpperAgeGroupBoundary();

	/* returns upper age boundary for specific age group index */
	/* ageGroupIndx: age group index */
	float GetUpperAgeGroupBoundary(unsigned short ageGroupIndx);

	/* gets the index of the current parameter set */
	unsigned short GetCrntParamSetIndx();

	/* gets the current kernel function code */
	unsigned short GetKernelFunctionCode();

	/* gets parameter a0 */
	double GetParamA0();

	/* gets parameter b0 */
	double GetParamB0();

	/* gets parameter a1 */
	double GetParamA1();

	/* gets parameter b1 */
	double GetParamB1();

	/* gets parameter d */
	double GetParamC1();

	/* gets the output flag for the current kernel function parameter set */
	char GetOutputFlag();
};



/* establishment size distribution vector */
class ESDVec
{
private:
	/* data object holding all data in ascii format */
	DataIn *pData;

	unsigned short AgeGroupIndx;      // current age group index

	/* describes some general establishment parameters that are common for target age group */
	class EstablishmentParams
	{
	public:
		unsigned short StaffAgeGroupIndx;   // age group index of staff
		float StaffRatio;                   // ratio of number of staff to number of other hosts in establishments
		unsigned short AvGroupSize;         // average group size in establishment (e.g. classes in schools)
		float FillRatio;                    // percentage of hosts from given age group that are associated with establishments
	};

	EstablishmentParams EstParams;          // common establishment parameters

public:
	/* element of vector */
	class ESDVEl
	{
	public:
		unsigned int Size;            // establishment size
		float CProb;                  // corresponding cumulative probability value
	};
	
	unsigned int NumEls;              // number of elements in vector
	ESDVEl *V;                        // vector of ESDVEl elements
	
	/* constructor; builds establishment size distribution vector */
	/* fName: name of file that contains vector */
	ESDVec(char *fName);

	~ESDVec();

	/* sets age group index */
	void SetAgeGroupIndx(unsigned short ageGroupIndx);

	/* gets current age group index */
	unsigned short GetAgeGroupIndx();

	/* gets staff age group index */
	float GetStaffAgeGroupIndx();

	/* gets staff ratio */
	float GetStaffRatio();

	/* gets average group size */
	unsigned short GetAvGroupSize();

	/* gets fill ratio */
	float GetFillRatio();

	/* samples establishment size */
	unsigned int SampleSize();
};



/* model host-to-establishment travel distribution vector */
class HETDVec
{
public:
	class HETDVEl
	{
	public:
		double Dist;             // travel distance
		double Prob;             // corresponding probability distribution vector value
		unsigned int NumHosts;   // number of hosts travelling
	};

	unsigned int NumHostsTotal;  // total number of hosts (within specific age group)
	unsigned int NumHosts;       // number of hosts (within specific age group) associated with establishments

	unsigned int NumEls;         // number of elements in vector
	HETDVEl *V;                  // vector of HETDVEl elements

	/* default constructor; creates undefined vector */
	HETDVec();

	/* constructor; builds empty vector */
	/* HETDVecLngth: length of vector */
	HETDVec(unsigned int HETDVecLngth);

	/* constructor; builds vector using supplied kernel function */
	/* HETDVecLngth: length of vector; F: pointer to the kernel function */
	HETDVec(unsigned int HETDVecLngth, HostToEstTravelDistrKernel *pHETDKernel, PointerToKernFunc F);

	~HETDVec();
};



#endif
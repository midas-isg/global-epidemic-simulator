/* PopulDistr.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* classes describing probability distributions used to synthesize population age structure and households */

#ifndef POPUL_DISTR
#define POPUL_DISTR



/* household size distribution vector */
class HSDVec
{
private:
	/* element of household size distribution vector */
	class HSDVEl
	{
	public:
		unsigned short Size;       // household size
		float CProb;               // corresponding cumulative probability vector value
	};

	unsigned short NumEls;         // number of elements in vector
	HSDVEl *V;                      // vector of SDVEl elements

public:
	/* default constructor; creates undefined vector */
	HSDVec();

	/* constructor; builds household size distribution vector */
	/* SDVecLngth: length of vector; pSize: vector of household sizes, pCProb: cumulative distribution vector */
	HSDVec(unsigned short SDVecLngth, unsigned short *pSize, float *pCProb);

	/* constructor; builds household size distribution vector */
	/* fName: name of file that contains vector */
	HSDVec(char *fName);

	~HSDVec();
	
	/* samples household size */
	unsigned short Sample();
};



/* matrix of household members' age distribution vectors */
class ADVMtrx
{
private:
	/* household members' age distribution vector */
	class ADVec
	{
	public:
		/* element of age distribution vector */
		class ADVEl
		{
		public:
			float Age;             // age
			float CProb;           // corresponding cumulative probability vector value
		};

		unsigned short NumEls;     // number of elements in vector
		ADVEl *V;                  // vector of ADVEl elements
	};

	unsigned short NumEls;         // number of elements in array of age distribution vectors
	ADVec *M;                      // array of age distribution vectors

public:
	/* default constructor; creates undefined matrix */
	ADVMtrx();

	/* constructor; builds smatrix of household members' age distribution vectors */
	/* ADNumVec: number of age distribution vectors; pADVecLngth: contains age distribution vectors lengths */ 
	/* ppAge: matrix of age values; ppCProb: matrix of cumulative probability distribution values */
	ADVMtrx(unsigned short ADNumVec, unsigned short *pADVecLngth, float **ppAge, float **ppCProb);

	/* constructor; builds smatrix of household members' age distribution vectors */
	/* fName: name of file that contains matrix */
	ADVMtrx(char *fName);

	~ADVMtrx();

	/* samples a household member's age */
	/* hshldSize: size of household */
	float Sample(unsigned short hshldSize);

	/* gets probability of host living in household of certain size to have age within certain age group */
	/* hshldSize: household size; age: host's age */
	float GetProb(unsigned short hshldSize, float age);
};



/* marginal population age distribution vector */
class MADVec
{
public:
	class MADVEl
	{
	public:
		float Age;               // age
		float Prob;              // corresponding probability distribution vector value
		int NumHosts;            // number of hosts in the corresponding age band
	};

	unsigned short NumEls;       // number of elements in vector
	MADVEl* V;                   // vector of MADVEl elements

	/* default constructor; creates undefined vector */
	MADVec();

	/* constructor; builds empty vector */
	/* MADVecLngth: length of vector */
	MADVec(unsigned short MADVecLngth);

	/* constructor; inputs vector from a file */
	/* fName: file name */
	MADVec(char *fName);

	~MADVec();

	MADVec operator -(MADVec &op2);

	MADVec(const MADVec &op);

	MADVec operator =(MADVec &op2);
};



#endif
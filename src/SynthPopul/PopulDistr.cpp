/* PopulDistr.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "PopulDistr.h"
#ifdef _OPENMP
#include "randlib_par/randlib_par.h"
#else
#include "randlib/randlib.h"
#endif



/* default constructor; creates undefined vector */
HSDVec::HSDVec() : NumEls(0), V(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec() completed.\n\n";
#endif
};



/* constructor; builds household size distribution vector */
/* SDVecLngth: length of vector; pSize: vector of household sizes, pCProb: cumulative distribution vector */
HSDVec::HSDVec(unsigned short SDVecLngth, unsigned short *pSize, float *pCProb) : NumEls(SDVecLngth + 1)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec(unsigned short, unsigned short*, float*) started...\n";
#endif
	try
	{
		V = new HSDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of SDVec::SDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	V[0].Size = 0;
	V[0].CProb = 0.0f;
	for(unsigned short i=1; i < NumEls; i++)
	{
		V[i].Size = pSize[i];
		V[i].CProb = pCProb[i];
	}
	if( ABS(V[NumEls-1].CProb - 1.0f) > TOLER_VAL )
		throw INCORRECT_DISTR_VECTOR;
	V[NumEls-1].CProb = 1.0f + TOLER_VAL;
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec(unsigned short, unsigned short*, float*) completed.\n\n";
#endif
}



/* constructor; builds household size distribution vector */
/* FName: name of file that contains vector */
HSDVec::HSDVec(char *fName)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec(char*) started...\n";
#endif
	InMtrx inM = InMtrx(fName);
#ifdef OUTPUT_LOG_TEST_L1
	cout << "Household size distribution:\n";
	inM.OutCon();
#endif

	NumEls = inM.NumRows + 1;
	try
	{
		V = new HSDVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of SDVec::SDVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	V[0].Size = 0;
	V[0].CProb = 0.0f;
	for(unsigned short i=1; i < NumEls; i++)
	{
		V[i].Size = (unsigned short)inM.Mtrx[i-1][0];
		V[i].CProb = inM.Mtrx[i-1][1];
	}
	if( ABS(V[NumEls-1].CProb - 1.0f) > TOLER_VAL )
		throw INCORRECT_DISTR_VECTOR;
	V[NumEls-1].CProb = 1.0f + TOLER_VAL;

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Household size distribution vector:\n";
	for(unsigned short i=0; i < NumEls; i++)
		cout << V[i].Size << '\t' << V[i].CProb << '\n';
	cout << '\n';
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::SDVec(char*) completed.\n\n";
#endif
}



HSDVec::~HSDVec()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::~SDVec() started...\n";
#endif
	if( V != NULL )
		delete [] V;
#ifdef OUTPUT_LOG_L1
	cout << "*** SDVec::~SDVec() completed.\n\n";
#endif
}



/* samples household size */
unsigned short HSDVec::Sample()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** SDVec::Sample() started...\n";
#endif
	float rnd = ranf_();   // generate a uniformly distributed random number
	for(unsigned short i=1; i < NumEls; i++){
		if( (V[i-1].CProb < rnd) && (V[i].CProb >= rnd) )
		{
#ifdef OUTPUT_LOG_L2
			cout << "*** SDVec::Sample() completed\n";
#endif
			return V[i].Size;
		}
	}
	throw INCORRECT_DISTR_VECTOR;
}



/* default constructor; creates undefined matrix */
ADVMtrx::ADVMtrx() : NumEls(0), M(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx() completed.\n\n";
#endif
}



/* constructor; builds matrix of household members' age distribution vectors */
/* ADNumVec: number of age distribution vectors; pADVecLngth: contains age distribution vectors lengths; */ 
/* ppAge: matrix of age values; ppCProb: matrix of cumulative probability distribution values */
ADVMtrx::ADVMtrx(unsigned short ADNumVec, unsigned short *pADVecLngth, float **ppAge, float **ppCProb) : NumEls(ADNumVec)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx(unsigned short, unsigned short*, float**, float**) started...\n";
#endif
	try
	{
		M = new ADVec[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (M) of ADVMtrx::ADVec objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		M[i].NumEls = pADVecLngth[i] + 1;
		try
		{
			M[i].V = new ADVec::ADVEl[M[i].NumEls];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (M[" << i << "].V) of ADVMtrx::ADVec::ADVEl objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		M[i].V[0].Age = 0.0;
		M[i].V[0].CProb = 0.0;
		for(unsigned short j=1; j < M[i].NumEls; j++)
		{
			M[i].V[j].Age = ppAge[i][j];
			M[i].V[j].CProb = ppCProb[i][j];
		}
		if( ABS(M[i].V[M[i].NumEls-1].CProb - 1.0f) > TOLER_VAL )
			throw INCORRECT_DISTR_VECTOR;
		M[i].V[M[i].NumEls-1].CProb = 1.0f + TOLER_VAL;
	}
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx(unsigned short, unsigned short*, float**, float**) completed.\n\n";
#endif
}



/* constructor; builds matrix of household members' age distribution vectors */
/* fName: name of file that contains matrix */
ADVMtrx::ADVMtrx(char *fName)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx(char*) started...\n";
#endif
	InMtrx inM = InMtrx(fName);
#ifdef OUTPUT_LOG_TEST_L1
	cout << "Matrix of household members' age distribution vectors:\n";
	inM.OutCon();
#endif

	NumEls = inM.NumCols - 1;
	try
	{
		M = new ADVec[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (M) of ADVMtrx::ADVec objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		M[i].NumEls = inM.NumRows + 1;
		try
		{
			M[i].V = new ADVec::ADVEl[M[i].NumEls];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (M[" << i << "].V) of ADVMtrx::ADVec::ADVEl objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		M[i].V[0].Age = 0.0f;
		M[i].V[0].CProb = 0.0f;
		for(unsigned short j=1; j < M[i].NumEls; j++)
		{
			M[i].V[j].Age = inM.Mtrx[j-1][0];
			M[i].V[j].CProb = inM.Mtrx[j-1][i+1];
		}
		if( ABS(M[i].V[M[i].NumEls-1].CProb - 1.0f) > TOLER_VAL )
			throw INCORRECT_DISTR_VECTOR;
		M[i].V[M[i].NumEls-1].CProb = 1.0f + TOLER_VAL;
	}

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Household members' age distribution vectors:\n";
	for(unsigned short i=0; i < NumEls; i++)
	{
		cout << "*** vector #" << i+1 << "\n";
		for(unsigned short j=0; j < M[i].NumEls; j++)
			cout << M[i].V[j].Age << '\t' << M[i].V[j].CProb << '\n';
		cout << '\n';
	}
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::ADVMtrx(char*) completed.\n\n";
#endif
}



ADVMtrx::~ADVMtrx()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::~ADVMtrx() started...\n";
#endif
	if( M != NULL )
	{
		for(unsigned short i=0; i < NumEls; i++)
			delete [] M[i].V;
		delete [] M;
	}
#ifdef OUTPUT_LOG_L1
	cout << "*** ADVMtrx::~ADVMtrx() completed.\n\n";
#endif
}



/* samples a household member's age */
/* hshldSize: size of household */
float ADVMtrx::Sample(unsigned short hshldSize)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** ADVMtrx::Sample(unsigned short) started...\n";
#endif
	if( hshldSize > NumEls )
		throw INCORRECT_DISTR_VECTOR;   // no proper distribution vector

	ADVec *pM = &M[hshldSize-1];
	float rnd, age;
	rnd = ranf_();
	for(unsigned short i=1; i < pM->NumEls; i++)
	{
		if( (pM->V[i-1].CProb < rnd) && (pM->V[i].CProb >= rnd) )
		{
			do
			{
				age = pM->V[i].Age - (pM->V[i].Age - pM->V[i-1].Age) * ranf_();
			}
			while( (age == pM->V[i-1].Age) || (age == pM->V[i].Age) );
#ifdef OUTPUT_LOG_L2
			cout << "Age: " << age << "\n";
			cout << "*** ADVMtrx::Sample(unsigned short) completed.\n\n";
#endif
			return age;
		}
	}
	throw INCORRECT_DISTR_VECTOR;
}



/* gets probability of host living in household of certain size to have age within certain age group */
/* hshldSize: household size; age: host's age */
float ADVMtrx::GetProb(unsigned short hshldSize, float age)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** ADVMtrx::GetProb(unsigned short, float) started...\n";
#endif
	unsigned short indx = (unsigned short)(ceil(age/AGE_BAND))-1;
#ifdef OUTPUT_LOG_L2
	cout << "*** ADVMtrx::GetProb(unsigned short, float) completed\n";
#endif
	return ( M[hshldSize-1].V[indx+1].CProb - M[hshldSize-1].V[indx].CProb );
}



/* default constructor; creates undefined vector */
MADVec::MADVec() : NumEls(0), V(NULL)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec() started...\n";
#endif
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec() completed\n";
#endif
}



/* constructor; builds empty vector */
/* MADVecLngth: length of vector */
MADVec::MADVec(unsigned short MADVecLngth) : NumEls(MADVecLngth)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec(unsigned short) started...\n";
#endif
	try
	{
		 V = new MADVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of MADVec::MADVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		V[i].Age = (i+1)*AGE_BAND;
		V[i].Prob = 0.0f;
		V[i].NumHosts = 0;
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec(unsigned short) completed\n";
#endif
}



/* constructor; inputs vector from a file */
/* fName: file name */
MADVec::MADVec(char *fName)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec(char*) started...\n";
#endif
	InMtrx inM = InMtrx(fName);
#ifdef OUTPUT_LOG_TEST_L1
	cout << "Marginal age distribution:\n";
	inM.OutCon();
#endif

	NumEls = inM.NumRows;
	try{
		V = new MADVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of MADVec::MADVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
	{
		V[i].Age = inM.Mtrx[i][0];
		V[i].Prob = inM.Mtrx[i][1];
		if( inM.NumCols > 2 )
			V[i].NumHosts = (int)inM.Mtrx[i][2];
		else V[i].NumHosts = 0;
	}

#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::MADVec(char*) completed\n";
#endif
}



MADVec::~MADVec()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::~MADVec() started...\n";
#endif
	if( V != NULL )
		delete [] V;
#ifdef OUTPUT_LOG_L2
	cout << "*** MADVec::~MADVec() completed\n";
#endif
}



MADVec MADVec::operator -(MADVec &op2)
{
	if( NumEls != op2.NumEls )
		throw INCORRECT_DISTR_VECTOR;
	
	MADVec temp = MADVec(NumEls);
	for(unsigned short i=0; i < NumEls; i++)
	{
		temp.V[i].Prob = V[i].Prob - op2.V[i].Prob;
		temp.V[i].NumHosts = V[i].NumHosts - op2.V[i].NumHosts;
	}

	return temp;
}



MADVec::MADVec(const MADVec &op)
{
	NumEls = op.NumEls;
	try{
		V = new MADVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of MADVec::MADVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
		V[i] = op.V[i];
}



MADVec MADVec::operator =(MADVec &op2)
{
	NumEls = op2.NumEls;
	try{
		V = new MADVEl[NumEls];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (V) of MADVec::MADVEl objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned short i=0; i < NumEls; i++)
		V[i] = op2.V[i];

	return *this;
}

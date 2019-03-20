/* vector_replacement.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for overiding default vector class with lightweight version
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

#ifndef VECTOR_REPLACEMENT
#define VECTOR_REPLACEMENT

#include "simINT64.h"

#ifdef _OPENMP
#include "omp.h"
#endif

namespace lwv
{
	#include "lw_vector.h"

	/* stub template class for the light weight vector class lw_vector */
	template <class lwvType> class vector : public lw_vector<lwvType>
	{
	public:
		vector() : lw_vector<lwvType>()
		{};

		vector(int num_els) : lw_vector<lwvType>(num_els)
		{};

		vector(int num_els, const lwvType& el) : lw_vector<lwvType>(num_els, el)
		{};

		vector(const vector& v) : lw_vector<lwvType>(v)
		{};

		~vector()
		{};
	};
}

#endif

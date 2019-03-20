/* lw_vector.h, part of the Global Epidemic Simulation v1.0 BETA
/* Lightweight vector class
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

#ifndef LW_VECTOR
#define LW_VECTOR

#include "simINT64.h"

// Recent VS/Intel combinations have incorrectly left _OPENMP undefined. Enable OPENMP_ENABLED below if necessary.
#define OPENMP_ENABLED

#ifdef _OPENMP
#define OPENMP_ENABLED
//#pragma message("Vector class (lw_vector.h) - OpenMP parallelisation enabled")
#endif


/* enables thread safety while inserting or deleting elements from the vector */
/* can be switched off in a thread safe code to make it more efficient */
#ifdef OPENMP_ENABLED
//#define THREAD_SAFE_OPS
#endif
#ifndef THREAD_SAFE_OPS
//#pragma message("Warning: Vector class (lw_vector.h) - inserting or deleting elements from the vector is not thread safe!")
#endif


/* enables parallelisation of loops */
#ifdef OPENMP_ENABLED
#define LOOPS_IN_PARALLEL
#endif


#ifdef LOOPS_IN_PARALLEL
//#define SCHED_TYPE dynamic   // schedule type: dynamic
#define SCHED_TYPE static      // schedule type: static

#define NUM_PROCS_OUT 0        // number of processors excluded from parallelisation
#endif


/* enables index range check useful while debugging and testing */
//#define ENABLE_INDX_RANGE_CHECK
#ifndef ENABLE_INDX_RANGE_CHECK
//#pragma message("Warning: Vector class (lw_vector.h) - index range check disabled!")
#endif

/* exception codes */
#define OUT_OF_RANGE 1



#pragma pack(push,_CRT_PACKING)



template <class lwvType> class lw_vector
{
private:
#ifdef THREAD_SAFE_OPS
	omp_lock_t ob__lock;

	void init_lock();
	void destroy_lock();
	void set_lock();
	void unset_lock();
	bool is_lock_valid();
#endif

#ifdef OPENMP_ENABLED
	int num_procs;
#endif

	SIM_I64 alloc_size;   // allocated vector size
	SIM_I64 indx_flast;   // index of a memory location that follows the last element of a vector ( indx_flast <= alloc_size )
	lwvType *v;           // the vector itself


	/* methods that do bytewise data copying */
	void copy_frwrd(char* dest, char* src, SIM_I64 count);

	void copy_frwrd(char* dest, char* src, SIM_I64 count, int num_procs);

	void copy_bckwrd(char* dest, char* src, SIM_I64 count);


public:
	/* no proper definition of the iterator class */
	typedef lwvType* iterator;

	/* default constructor; allocates empty vector */
	lw_vector();

	/* constructor; allocates memory for a vector but the elements of this vector remain undefined */
	/* num_els: number of elements */
	lw_vector(SIM_I64 num_els);

	/* constructor; allocates memory for a vector and initialises its elements with copies of el */
	/* num_els: number of elements */
	lw_vector(SIM_I64 num_els, const lwvType& el);

	/* copy constructor */
	lw_vector(const lw_vector& lwv);

	/* destructor */
	~lw_vector();

	/* assignment operator */
	lw_vector<lwvType> operator=(const lw_vector& lwv2);

	/* provides access to vector elements via index indx; not thread safe */
	lwvType& operator[](SIM_I64 indx);

	/* returns a reference to the element at the position indx; not thread safe */
	lwvType& at(SIM_I64 indx);

	/* returns a reference to the first element of the vector */
	lwvType& front();

	/* returns a reference to the last element of the vector */
	lwvType& back();

	/* returns random-access iterator to the first element of the vector */
	iterator begin();

	/* returns random-access iterator that points just beyond the end of the vector */
	iterator end();

	/* returns the size of the vector (number of elements) */
	SIM_I64 size();

	/* tests if there are any elements in the vector */
	bool empty();

	/* adds an element el to the end of a vector */
	void push_back(const lwvType& el);

	/* inserts an element el into a vector at the position indx */
	/* returns an index that points to the position of the inserted element */
	SIM_I64 insert(SIM_I64 indx, const lwvType& el);

	/* inserts count copies of el into a vector starting from the position indx_start */
	void insert(SIM_I64 indx_start, SIM_I64 count, const lwvType& el);

	/* inserts an element el into a vector at the position specified by iterator it */
	/* returns an iterator that points to the position of the inserted element */
	iterator insert(iterator it, const lwvType& el);

	/* inserts count copies of el into a vector starting from the position specified by iterator it_start */
	void insert(iterator it_start, SIM_I64 count, const lwvType& el);

	/* deletes the element at the end of the vector without reducing its capacity */
	void pop_back();

	/* erases an element at the position specified by indx */
	/* returns an index pointing at the first element beyond the removed one or to the end of the vector if there are no such elements */
	SIM_I64 erase(SIM_I64 indx);

	/* erases elements in the range starting from index indx_start and finishing just before the position defined by indx_end */
	/* returns an index pointing at the first element beyond those removed or to the end of the vector if there are no such elements */
	SIM_I64 erase(SIM_I64 indx_start, SIM_I64 indx_end);

	/* erases an element at the position specified by iterator it */
	/* returns an iterator pointing at the first element beyond the removed one or to the end of the vector if there are no such elements */
	iterator erase(iterator it);

	/* erases elements in the range starting from iterator it_start and finishing just before the position defined by it_end */
	/* returns an iterator pointing at the first element beyond those removed or to the end of the vector if there are no such elements */
	iterator erase(iterator it_start, iterator it_end);

	/* erases all elements of the vector without reducing its capacity */
	void clear();

	/* specifies a new size for the vector */
	/* if the vector size is less than new size new_size, new (default) elements are added to the end of the vector until it reaches the requested size */
	/* otherwise elements of the vector are deleted starting from its end until until it reaches the requested size */
	void resize(SIM_I64 new_size);

	/* specifies a new size for the vector */
	/* if the vector size is less than new size new_size, new (el) elements are added to the end of the vector until it reaches the requested size */
	/* otherwise elements of the vector are deleted starting from its end until until it reaches the requested size */
	void resize(SIM_I64 new_size, const lwvType& el);

	/* compacts the vector reducing the allocated memory */
	void compact();
};



/* vector size grows exponentially */
#define DEF_INI_SIZE 1       // default initial size
#define EXP_GROWTH_COEFF 3 //3



#ifdef THREAD_SAFE_OPS
template <class lwvType> inline void lw_vector<lwvType>::init_lock()
{
	omp_init_lock(&ob__lock);
}



template <class lwvType> inline void lw_vector<lwvType>::destroy_lock()
{
	omp_destroy_lock(&ob__lock);
	ob__lock = 0;
}



template <class lwvType> inline void lw_vector<lwvType>::set_lock()
{
	omp_set_lock(&ob__lock);
}



template <class lwvType> inline void lw_vector<lwvType>::unset_lock()
{
	omp_unset_lock(&ob__lock);
}



template <class lwvType> inline bool lw_vector<lwvType>::is_lock_valid()
{
	if( ob__lock != 0 )
		return true;
	else
		return false;
}
#endif



template <class lwvType> inline void lw_vector<lwvType>::copy_frwrd(char* dest, char* src, SIM_I64 count)
{
	for(SIM_I64 i=0; i < count; i++)
		*(dest++) = *(src++);
}



template <class lwvType> inline void lw_vector<lwvType>::copy_frwrd(char* dest, char* src, SIM_I64 count, int num_procs)
{
#ifdef LOOPS_IN_PARALLEL
	SIM_I64 chunk_size = count > num_procs  ?  count / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
	for(SIM_I64 i=0; i < count; i++)
		dest[i] = src[i];
}



template <class lwvType> inline void lw_vector<lwvType>::copy_bckwrd(char* dest, char* src, SIM_I64 count)
{
	for(SIM_I64 i = count; i > 0 ; i--)
		*(--dest) = *(--src);
}



/* default constructor; allocates empty vector */
template <class lwvType> lw_vector<lwvType>::lw_vector() : alloc_size(0), indx_flast(0), v(0)
{
#ifdef THREAD_SAFE_OPS
	init_lock();
#endif
#ifdef OPENMP_ENABLED
	num_procs = omp_get_num_procs() - NUM_PROCS_OUT;   // TESTING!
#endif
}



/* constructor; allocates memory for a vector but the elements of this vector remain undefined */
/* num_els: number of elements */
template <class lwvType> lw_vector<lwvType>::lw_vector(SIM_I64 num_els) : alloc_size(num_els), indx_flast(0)
{
#ifdef THREAD_SAFE_OPS
	init_lock();
#endif
#ifdef OPENMP_ENABLED
	num_procs = omp_get_num_procs();
#endif
	if( alloc_size != 0 )
		v = (lwvType*)(new char[alloc_size * (SIM_I64)sizeof(lwvType)]);
	else
		v = 0;
}



/* constructor; allocates memory for a vector and initialises its elements with copies of el */
/* num_els: number of elements */
template <class lwvType> lw_vector<lwvType>::lw_vector(SIM_I64 num_els, const lwvType& el) : alloc_size(num_els), indx_flast(alloc_size)
{
#ifdef THREAD_SAFE_OPS
	init_lock();
#endif
#ifdef OPENMP_ENABLED
	num_procs = omp_get_num_procs();
#endif
	if( alloc_size != 0 )
		v = (lwvType*)(new char[alloc_size * (SIM_I64)sizeof(lwvType)]);
	else
		v = 0;

#ifdef LOOPS_IN_PARALLEL
	SIM_I64 chunk_size = indx_flast > num_procs  ?  indx_flast / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
	for(SIM_I64 i=0; i < indx_flast; i++)
		::new(&v[i]) lwvType(el);
}



/* copy constructor */
template <class lwvType> lw_vector<lwvType>::lw_vector(const lw_vector& lwv)
{
#ifdef THREAD_SAFE_OPS
	init_lock();
#endif
#ifdef OPENMP_ENABLED
	num_procs = lwv.num_procs;
#endif
	alloc_size = lwv.alloc_size;
	indx_flast = lwv.indx_flast;

	if( alloc_size != 0 )
		v = (lwvType*)(new char[alloc_size * (SIM_I64)sizeof(lwvType)]);
	else
		v = 0;

#ifdef LOOPS_IN_PARALLEL
	SIM_I64 chunk_size = indx_flast > num_procs  ?  indx_flast / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
	for(SIM_I64 i=0; i < indx_flast; i++)
		::new(&v[i]) lwvType(lwv.v[i]);
}



/* destructor */
template <class lwvType> lw_vector<lwvType>::~lw_vector()
{
	if( v != 0 )
	{
		for(SIM_I64 i=0; i < indx_flast; i++)
			v[i].~lwvType();

		delete [] (char*)v;

		alloc_size = 0;
		indx_flast = 0;
	}
	v = 0;
#ifdef THREAD_SAFE_OPS
	destroy_lock();
#endif
}



/* assignment operator */
template <class lwvType> lw_vector<lwvType> lw_vector<lwvType>::operator=(const lw_vector& lwv2)
{
#ifdef THREAD_SAFE_OPS
	if( !is_lock_valid() )
		init_lock();
	set_lock();
#endif
#ifdef OPENMP_ENABLED
	num_procs = lwv2.num_procs;
#endif

	if( v != 0 )
	{
#ifdef LOOPS_IN_PARALLEL
		SIM_I64 chunk_size = indx_flast > num_procs  ?  indx_flast / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
		for(SIM_I64 i=0; i < indx_flast; i++)
			v[i].~lwvType();

		delete [] (char*)v;
	}

	alloc_size = lwv2.alloc_size;
	indx_flast = lwv2.indx_flast;

	if( alloc_size != 0 )
		v = (lwvType*)(new char[alloc_size * (SIM_I64)sizeof(lwvType)]);
	else
		v = 0;

#ifdef LOOPS_IN_PARALLEL
	SIM_I64 chunk_size = indx_flast > num_procs  ?  indx_flast / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
	for(SIM_I64 i=0; i < indx_flast; i++)
		v[i] = lwv2.v[i];

#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return *this;
}



/* provides access to vector elements via index indx; not thread safe */
template <class lwvType> inline lwvType& lw_vector<lwvType>::operator[](SIM_I64 indx)
{
#ifdef ENABLE_INDX_RANGE_CHECK
	if( indx < 0 || indx >= indx_flast )
		throw OUT_OF_RANGE;
#endif
	return v[indx];
}



/* returns a reference to the element at the position indx; not thread safe */
template <class lwvType> inline lwvType& lw_vector<lwvType>::at(SIM_I64 indx)
{
#ifdef ENABLE_INDX_RANGE_CHECK
	if( indx < 0 || indx >= indx_flast )
		throw OUT_OF_RANGE;
#endif
	return v[indx];
}



/* returns a reference to the first element of the vector */
template <class lwvType> inline lwvType& lw_vector<lwvType>::front()
{
	return v[0];
}



/* returns a reference to the last element of the vector */
template <class lwvType> inline lwvType& lw_vector<lwvType>::back()
{
	return v[indx_flast - 1];
}



/* returns random-access iterator to the first element of the vector */
template <class lwvType> inline typename lw_vector<lwvType>::iterator lw_vector<lwvType>::begin()
{
	return &v[0];
}



/* returns random-access iterator that points just beyond the end of the vector */
template <class lwvType> inline typename lw_vector<lwvType>::iterator lw_vector<lwvType>::end()
{
	return &v[indx_flast];
}



/* returns the size of the vector (number of elements) */
template <class lwvType> inline SIM_I64 lw_vector<lwvType>::size()
{
	return indx_flast;
}



/* tests if there are any elements in the vector */
template <class lwvType> inline bool lw_vector<lwvType>::empty()
{
	if( indx_flast == 0 )
		return true;
	else
		return false;
}



/* adds an element el to the end of a vector */
template <class lwvType> void lw_vector<lwvType>::push_back(const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx_flast >= alloc_size )   // the vector is full, reallocation needed
	{
		SIM_I64 new_alloc_size = alloc_size != 0  ?  alloc_size * EXP_GROWTH_COEFF  :  DEF_INI_SIZE;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

		if( v != 0 )
		{
#ifndef LOOPS_IN_PARALLEL
			copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType));
#else
			copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType), num_procs);
#endif
			delete [] (char*)v;
		}

		v = v_temp;
		alloc_size = new_alloc_size;
	}

	::new(&v[indx_flast++]) lwvType(el);
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* inserts an element el into a vector at the position indx */
template <class lwvType> SIM_I64 lw_vector<lwvType>::insert(SIM_I64 indx, const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx < 0 || indx > indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( indx_flast < alloc_size )                                  // there is still memory available in the vector
	{
		copy_bckwrd((char*)(&v[indx_flast + 1]), (char*)(&v[indx_flast]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType));
		::new(&v[indx]) lwvType(el);
	}
	else                                                           // the vector is full, reallocation needed
	{
		SIM_I64 new_alloc_size = alloc_size != 0  ?  alloc_size * EXP_GROWTH_COEFF  :  DEF_INI_SIZE;

		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType), num_procs);
#endif
		::new(&v_temp[indx]) lwvType(el);
#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)(&v_temp[indx + 1]), (char*)(&v[indx]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)(&v_temp[indx + 1]), (char*)(&v[indx]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast++;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return indx;
}



/* inserts count copies of el into a vector starting from the position indx_start */
template <class lwvType> void lw_vector<lwvType>::insert(SIM_I64 indx_start, SIM_I64 count, const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx_start < 0 || indx_start > indx_flast || count < 0 )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( (indx_flast + count) <= alloc_size )                                       // allocated size of the vector remains unchanged
	{
		copy_bckwrd((char*)(&v[indx_flast + count]), (char*)(&v[indx_flast + count - 1]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType));

#ifdef LOOPS_IN_PARALLEL
		SIM_I64 chunk_size = count > num_procs  ?  count / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
		for(SIM_I64 i = indx_start; i < indx_start + count; i++)
			::new(&v[i]) lwvType(el);
	}
	else                                                                           // allocated vector size is insufficient; reallocation and expansion needed
	{
		SIM_I64 new_alloc_size = alloc_size != 0  ?  alloc_size * EXP_GROWTH_COEFF  :  DEF_INI_SIZE;

		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType), num_procs);
#endif
		for(SIM_I64 i = indx_start; i < indx_start + count; i++)
			::new(&v_temp[i]) lwvType(el);
#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)(&v_temp[indx_start + count]), (char*)(&v[indx_start]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)(&v_temp[indx_start + count]), (char*)(&v[indx_start]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast += count;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* inserts an element el into a vector at the position specified by iterator it */
template <class lwvType> typename lw_vector<lwvType>::iterator lw_vector<lwvType>::insert(iterator it, const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 delta = (SIM_I64)(it + 1) - (SIM_I64)it;
	SIM_I64 indx = ((SIM_I64)it - (SIM_I64)begin()) / delta;

	if( indx < 0 || indx > indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( indx_flast < alloc_size )                                  // there is still memory available in the vector
	{
		copy_bckwrd((char*)(&v[indx_flast + 1]), (char*)(&v[indx_flast]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType));
		::new(&v[indx]) lwvType(el);
	}
	else                                                           // the vector is full, reallocation needed
	{
		SIM_I64 new_alloc_size = alloc_size != 0  ?  alloc_size * EXP_GROWTH_COEFF  :  DEF_INI_SIZE;

		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType), num_procs);
#endif
		::new(&v_temp[indx]) lwvType(el);
#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)(&v_temp[indx + 1]), (char*)(&v[indx]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)(&v_temp[indx + 1]), (char*)(&v[indx]), (indx_flast - indx) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast++;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return it;
}



/* inserts count copies of el into a vector starting from the position specified by iterator it_start */
template <class lwvType> void lw_vector<lwvType>::insert(iterator it_start, SIM_I64 count, const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 delta = (SIM_I64)(it_start + 1) - (SIM_I64)it_start;
	SIM_I64 indx_start = ((SIM_I64)it_start - (SIM_I64)begin()) / delta;

	if( indx_start < 0 || indx_start > indx_flast || count < 0 )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( (indx_flast + count) <= alloc_size )                                       // allocated size of the vector remains unchanged
	{
		copy_bckwrd((char*)(&v[indx_flast + count]), (char*)(&v[indx_flast + count - 1]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType));

#ifdef LOOPS_IN_PARALLEL
		SIM_I64 chunk_size = count > num_procs  ?  count / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
		for(SIM_I64 i = indx_start; i < indx_start + count; i++)
			::new(&v[i]) lwvType(el);
	}
	else                                                                           // allocated vector size is insufficient; reallocation and expansion needed
	{
		SIM_I64 new_alloc_size = alloc_size != 0  ?  alloc_size * EXP_GROWTH_COEFF  :  DEF_INI_SIZE;

		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType), num_procs);
#endif
		for(SIM_I64 i = indx_start; i < indx_start + count; i++)
			::new(&v_temp[i]) lwvType(el);
#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)(&v_temp[indx_start + count]), (char*)(&v[indx_start]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)(&v_temp[indx_start + count]), (char*)(&v[indx_start]), (indx_flast - indx_start) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast += count;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* deletes the element at the end of the vector without reducing its capacity  */
template <class lwvType> void lw_vector<lwvType>::pop_back()
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx_flast == 0 )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		return;
	}

	v[--indx_flast].~lwvType();   // explicitly call the destructor for the erased object (virtual call)

#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* erases an element at the position specified by indx */
/* returns an index pointing at the first element beyond the removed one or to the end of the vector if there are no such elements */
template <class lwvType> SIM_I64 lw_vector<lwvType>::erase(SIM_I64 indx)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx < 0 || indx >= indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	v[indx].~lwvType();                                                       // explicitly call the destructor for the erased object (virtual call)

	if( EXP_GROWTH_COEFF * (indx_flast - 1) > alloc_size )                    // allocated size of the vector remains unchanged
		copy_frwrd((char*)(&v[indx]), (char*)(&v[indx + 1]), (indx_flast -indx - 1) * (SIM_I64)sizeof(lwvType));
	else                                                                      // reallocate the vector and decrease its size
	{
		SIM_I64 new_alloc_size = alloc_size / EXP_GROWTH_COEFF;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType));
		copy_frwrd((char*)(&v_temp[indx]), (char*)(&v[indx + 1]), (indx_flast - indx - 1) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType), num_procs);
		copy_frwrd((char*)(&v_temp[indx]), (char*)(&v[indx + 1]), (indx_flast - indx - 1) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast--;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return indx;
}



/* erases elements in the range starting from index indx_start and finishing just before the position defined by indx_end */
/* returns an index pointing at the first element beyond those removed or to the end of the vector if there are no such elements */
template <class lwvType> SIM_I64 lw_vector<lwvType>::erase(SIM_I64 indx_start, SIM_I64 indx_end)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx_start > indx_end || indx_start < 0 || indx_start >= indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( indx_end > indx_flast )
		indx_end = indx_flast;

	for(SIM_I64 i = indx_start; i < indx_end; i++)
		v[i].~lwvType();                                                             // explicitly call the destructor for the erased objects (virtual call)

	if( EXP_GROWTH_COEFF * (indx_flast - (indx_end - indx_start)) > alloc_size )     // allocated size of the vector remains unchanged
		copy_frwrd((char*)(&v[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType));
	else                                                                             // reallocate the vector and decrease its size
	{
		SIM_I64 new_alloc_size = alloc_size / EXP_GROWTH_COEFF;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType));
		copy_frwrd((char*)(&v_temp[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType), num_procs);
		copy_frwrd((char*)(&v_temp[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast -= indx_end - indx_start;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return indx_start;
}



/* erases an element at the position specified by iterator it */
/* returns an iterator pointing at the first element beyond the removed one or to the end of the vector if there are no such elements */
template <class lwvType> typename lw_vector<lwvType>::iterator lw_vector<lwvType>::erase(iterator it)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 delta = (SIM_I64)(it + 1) - (SIM_I64)it;
	SIM_I64 indx = ((SIM_I64)it - (SIM_I64)begin()) / delta;

	if( indx < 0 || indx >= indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	v[indx].~lwvType();                                                       // explicitly call the destructor for the erased object (virtual call)

	if( EXP_GROWTH_COEFF * (indx_flast - 1) > alloc_size )                    // allocated size of the vector remains unchanged
		copy_frwrd((char*)(&v[indx]), (char*)(&v[indx + 1]), (indx_flast -indx - 1) * (SIM_I64)sizeof(lwvType));
	else                                                                      // reallocate the vector and decrease its size
	{
		SIM_I64 new_alloc_size = alloc_size / EXP_GROWTH_COEFF;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType));
		copy_frwrd((char*)(&v_temp[indx]), (char*)(&v[indx + 1]), (indx_flast - indx - 1) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx * (SIM_I64)sizeof(lwvType), num_procs);
		copy_frwrd((char*)(&v_temp[indx]), (char*)(&v[indx + 1]), (indx_flast - indx - 1) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast--;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return it;
}



/* erases elements in the range starting from iterator it_start and finishing just before the position defined by it_end */
/* returns an iterator pointing at the first element beyond those removed or to the end of the vector if there are no such elements */
template <class lwvType> typename lw_vector<lwvType>::iterator lw_vector<lwvType>::erase(iterator it_start, iterator it_end)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 delta = (SIM_I64)(it_start + 1) - (SIM_I64)it_start;
	SIM_I64 indx_start = ((SIM_I64)it_start - (SIM_I64)begin()) / delta;
	SIM_I64 indx_end = ((SIM_I64)it_end - (SIM_I64)begin()) / delta;

	if( indx_start > indx_end || indx_start < 0 || indx_start >= indx_flast )
	{
#ifdef THREAD_SAFE_OPS
		unset_lock();
#endif
		throw OUT_OF_RANGE;
	}

	if( indx_end > indx_flast )
		indx_end = indx_flast;

	for(SIM_I64 i = indx_start; i < indx_end; i++)
		v[i].~lwvType();                                                             // explicitly call the destructor for the erased objects (virtual call)

	if( EXP_GROWTH_COEFF * (indx_flast - (indx_end - indx_start)) > alloc_size )     // allocated size of the vector remains unchanged
		copy_frwrd((char*)(&v[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType));
	else                                                                             // reallocate the vector and decrease its size
	{
		SIM_I64 new_alloc_size = alloc_size / EXP_GROWTH_COEFF;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType));
		copy_frwrd((char*)(&v_temp[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_start * (SIM_I64)sizeof(lwvType), num_procs);
		copy_frwrd((char*)(&v_temp[indx_start]), (char*)(&v[indx_end]), (indx_flast - indx_end) * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
		alloc_size = new_alloc_size;
	}
	indx_flast -= indx_end - indx_start;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
	return it_start;
}



/* erases all elements of the vector without reducing its capacity */
template <class lwvType> void lw_vector<lwvType>::clear()
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
#ifdef LOOPS_IN_PARALLEL
	SIM_I64 chunk_size = indx_flast > num_procs  ?  (indx_flast * (SIM_I64)sizeof(lwvType)) / num_procs  :  (SIM_I64)sizeof(lwvType);
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
	for(SIM_I64 i = 0; i < indx_flast; i++)
		v[i].~lwvType();                                      // explicitly call the destructor for the erased objects (virtual call)
	indx_flast = 0;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* specifies a new size for the vector */
/* if the vector size is less than new size new_size, new (default) elements are added to the end of the vector until it reaches the requested size */
/* otherwise elements of the vector are deleted starting from its end until it reaches the requested size */
template <class lwvType> void lw_vector<lwvType>::resize(SIM_I64 new_size)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 new_alloc_size = new_size;
	if( new_alloc_size != alloc_size )
	{
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);
	
		if( indx_flast > new_alloc_size )
			indx_flast = new_alloc_size;

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType), num_procs);
#endif

#ifdef LOOPS_IN_PARALLEL
		SIM_I64 chunk_size = (new_alloc_size - indx_flast) > num_procs  ?  (new_alloc_size - indx_flast) / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
		for(SIM_I64 i = indx_flast; i < new_alloc_size; i++)
			::new(&v_temp[i]) lwvType();

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
	}
	else if( new_alloc_size == alloc_size )
	{
		for(SIM_I64 i = indx_flast; i < new_alloc_size; i++)
			::new(&v[i]) lwvType();
	}

	indx_flast = new_alloc_size;
	alloc_size = new_alloc_size;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* specifies a new size for the vector */
/* if the vector size is less than new size new_size, new (el) elements are added to the end of the vector until it reaches the requested size */
/* otherwise elements of the vector are deleted starting from its end until it reaches the requested size */
template <class lwvType> void lw_vector<lwvType>::resize(SIM_I64 new_size, const lwvType& el)
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	SIM_I64 new_alloc_size = new_size;
	if( new_alloc_size != alloc_size )
	{
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

		if( indx_flast > new_alloc_size )
			indx_flast = new_alloc_size;

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType), num_procs);
#endif

#ifdef LOOPS_IN_PARALLEL
		SIM_I64 chunk_size = (new_alloc_size - indx_flast) > num_procs  ?  (new_alloc_size - indx_flast) / num_procs  :  1;
#pragma omp parallel for schedule(SCHED_TYPE, chunk_size) num_threads(num_procs)
#endif
		for(SIM_I64 i = indx_flast; i < new_alloc_size; i++)
			::new(&v_temp[i]) lwvType(el);

		if( v != 0 )
			delete [] (char*)v;
		v = v_temp;
	}
	else if( new_alloc_size == alloc_size )
	{
		for(SIM_I64 i = indx_flast; i < new_alloc_size; i++)
			::new(&v[i]) lwvType(el);
	}

	indx_flast = new_alloc_size;
	alloc_size = new_alloc_size;
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



/* compacts the vector reducing the allocated memory */
template <class lwvType> void lw_vector<lwvType>::compact()
{
#ifdef THREAD_SAFE_OPS
	set_lock();
#endif
	if( indx_flast < alloc_size )
	{
		SIM_I64 new_alloc_size = indx_flast;
		lwvType *v_temp = (lwvType*)(new char[new_alloc_size * (SIM_I64)sizeof(lwvType)]);

#ifndef LOOPS_IN_PARALLEL
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType));
#else
		copy_frwrd((char*)v_temp, (char*)v, indx_flast * (SIM_I64)sizeof(lwvType), num_procs);
#endif

		if( v != 0 )
			delete [] (char*)v;

		alloc_size = new_alloc_size;
		v = v_temp;
	}
#ifdef THREAD_SAFE_OPS
	unset_lock();
#endif
}



#pragma pack(pop)



#endif

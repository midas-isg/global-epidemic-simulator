/* Util.h, part of the Global Epidemic Simulation v1.0 BETA
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



/* utility class */

#ifndef UTIL
#define UTIL


/* used in the code to read and store matrixes */
class InMtrx
{
public:
	unsigned int NumRows;          // number of rows in matrix
	unsigned int NumCols;          // number of columns in matrix
	float **Mtrx;                  // matrix itself

	/* constructor; reads matrix from file */
	/* fName: file name */
	InMtrx(char *fName);

	/* constructor; reads matrix corresponding to data block id from file */
	/* fName: file name; id: data block id */
	InMtrx(char *fName, unsigned int id);

	~InMtrx();

	/* outputs matrix to console for logging purposes */
	void OutCon();
};



/* block of data represented as matrix */
class DataBlock
{
public:
	unsigned int NumRows;          // number of rows in matrix
	unsigned int NumCols;          // number of columns in matrix
	float **Mtrx;                  // matrix itself

	/* constructor; creates empty matrix */
	DataBlock(unsigned int NumRows, unsigned int NumCols);

	~DataBlock();

	/* outputs matrix to console for logging purposes */
	void OutCon();
};



/* used to read data blocks and represent them as matrixes */
class DataIn
{
private:
	unsigned int RawDataSize;   // buffer size
	char *RawData;              // contents of input file stored in buffer

public:
	/* constructor; reads data from file */
	/* fName: file name */
	DataIn(char *fName);

	~DataIn();

	/* reads and returns block of data */
	/* id: data block id */
	DataBlock* ReadDataBlock(unsigned int id);
};



#endif

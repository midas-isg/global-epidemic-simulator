/* Util.cpp, part of the Global Epidemic Simulation v1.0 BETA
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



/* constructor; reads matrix from file */
/* fName: file name */
InMtrx::InMtrx(char *fName)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::InMtrx(char*) started...\n";
#endif
	fstream inF(fName, ios::in | ios::binary);
	if( !inF )
		throw FILE_OPEN_ERROR;
	inF.seekg(0, ios::end);
	unsigned int bufSize = inF.tellg();
	inF.seekg(0, ios::beg);
	char *buf;
	try
	{
		buf = new char[++bufSize];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (buf) of char objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	inF.read(buf, bufSize);
	buf[bufSize-1] = '\0';
	inF.close();

	NumRows = NumCols = 0;
	unsigned int i, j, row, col;
	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			i = l-1;
		}
		if( (buf[i] == '\n' || buf[i] == '\0') && (col > 0) )
		{
			if( NumCols == 0 )
				NumCols = col;
			else
				if( NumCols != col )
					throw TABLE_UNDEFINED;
			col = 0;
			j=0;
		}
	}
	NumRows = row;

	try
	{
		Mtrx = new float*[NumRows];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Mtrx) of float* objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(i=0; i <  NumRows; i++)
	{
		try
		{
			Mtrx[i] = new float[NumCols];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (Mtrx[" << i << "]) of float objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
	}
	
	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			char ch = buf[l];
			buf[l] = '\0';
			sscanf(&buf[i], "%f", &Mtrx[row-1][col-1]);
			buf[l] = ch;
			i = l-1;
		}
		if( buf[i] == '\n' || buf[i] == '\0' )
		{
			col = 0;
			j=0;
		}
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::InMtrx(char*) completed\n\n";
#endif
}



/* constructor; reads matrix corresponding to data block id from file */
/* fName: file name; id: data block id */
InMtrx::InMtrx(char *fName, unsigned int id)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::InMtrx(char*) started...\n";
#endif
	fstream inF(fName, ios::in | ios::binary);
	if( !inF )
		throw FILE_OPEN_ERROR;
	inF.seekg(0, ios::end);
	unsigned int bufSize = inF.tellg();
	inF.seekg(0, ios::beg);
	char *buf;
	try
	{
		buf = new char[++bufSize];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (buf) of char objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	inF.read(buf, bufSize);
	buf[bufSize-1] = '\0';
	inF.close();

	char *buf_ = buf;
	char id_found = 0;
	unsigned int id_;
	for(unsigned int i=0; (i < bufSize) && !id_found; i++)
	{
		if( buf_[i] == '[' )   // [id] is a separator sequence, id is the data block id
		{
			i++;
			for(unsigned int j = i+1; j < bufSize; j++)
			{
				if( buf_[j] == ']' )
				{
					buf_[j] = '\0';
					sscanf(&buf_[i], "%i", &id_);
					if( id == id_ )
					{
						id_found++;
						buf = &buf_[++j];
					}
					break;
				}
			}
		}
	}

	for(unsigned int i=0; i < bufSize; i++)
	{
		if( buf[i] >= '0' && buf[i] <= '9')
		{
			buf = &buf[i];
			break;
		}
	}


	if( id_found )
	{
		for(unsigned int i=0; ; i++)
		{
			if( buf[i] == '[' )
			{
				buf[i] = '\0';
				bufSize = i;
				break;
			}
			if( buf[i] == '\0' )
			{
				bufSize = i;
				break;
			}
		}
	}
	else
	{
		throw INDX_OUT_OF_RANGE;
	}

	NumRows = NumCols = 0;
	unsigned int i, j, row, col;
	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			i = l-1;
		}
		if( (buf[i] == '\n' || buf[i] == '\0') && (col > 0) )
		{
			if( NumCols == 0 )
				NumCols = col;
			else
				if( NumCols != col )
					throw TABLE_UNDEFINED;
			col = 0;
			j=0;
		}
	}
	NumRows = row;

	try
	{
		Mtrx = new float*[NumRows];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Mtrx) of float* objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(i=0; i <  NumRows; i++)
	{
		try
		{
			Mtrx[i] = new float[NumCols];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (Mtrx[" << i << "]) of float objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
	}
	
	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			char ch = buf[l];
			buf[l] = '\0';
			sscanf(&buf[i], "%f", &Mtrx[row-1][col-1]);
			buf[l] = ch;
			i = l-1;
		}
		if( buf[i] == '\n' || buf[i] == '\0' )
		{
			col = 0;
			j=0;
		}
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::InMtrx(char*) completed\n\n";
#endif
}



InMtrx::~InMtrx()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::~InMtrx() started...\n";
#endif
	if( Mtrx != NULL )
	{
		for(unsigned int i=0; i < NumRows; i++)
			delete [] Mtrx[i];
		delete [] Mtrx;
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::~InMtrx() completed\n\n";
#endif
}



/* outputs matrix to console for logging purposes */
void InMtrx::OutCon()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::OutCon() started...\n\n";
#endif
	cout << "Input matrix:\n";
	for(unsigned int i=0; i <  NumRows; i++)
	{
		for(unsigned int j=0; j <  NumCols; j++)
			cout << Mtrx[i][j] << '\t';
		cout << '\n';
	}
	cout << '\n';
#ifdef OUTPUT_LOG_L2
	cout << "*** InMtrx::OutCon() completed\n\n";
#endif
}






/* constructor; creates empty matrix */
DataBlock::DataBlock(unsigned int numRows, unsigned int numCols) : NumRows(numRows), NumCols(numCols)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::DataBlock(unsigned int, unsigned int) started...\n";
#endif
	try
	{
		Mtrx = new float*[NumRows];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Mtrx) of float* objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(unsigned int i=0; i <  NumRows; i++)
	{
		try
		{
			Mtrx[i] = new float[NumCols];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (Mtrx[" << i << "]) of float objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
	}

	for(unsigned int i=0; i < NumRows; i++)
	{
		for(unsigned int j=0; j <  NumCols; j++)
			Mtrx[i][j] = 0.0f;
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::DataBlock(unsigned int, unsigned int) completed.\n\n";
#endif
}



DataBlock::~DataBlock()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::~DataBlock() started...\n";
#endif
	if( Mtrx != NULL )
	{
		for(unsigned int i=0; i < NumRows; i++)
			delete [] Mtrx[i];
		delete [] Mtrx;
	}
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::~DataBlock() completed\n\n";
#endif
}



/* outputs matrix to console for logging purposes */
void DataBlock::OutCon()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::OutCon() started...\n\n";
#endif
	cout << "Input matrix:\n";
	for(unsigned int i=0; i <  NumRows; i++)
	{
		for(unsigned int j=0; j <  NumCols; j++)
			cout << Mtrx[i][j] << '\t';
		cout << '\n';
	}
	cout << '\n';
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock::OutCon() completed\n\n";
#endif
}



/* constructor; reads data from file */
/* fName: file name */
DataIn::DataIn(char *fName)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataIn::DataIn(char*) started...\n";
#endif
	fstream inF(fName, ios::in | ios::binary);
	if( !inF )
		throw FILE_OPEN_ERROR;
	inF.seekg(0, ios::end);
	RawDataSize = inF.tellg();
	RawDataSize++;
	inF.seekg(0, ios::beg);
	try
	{
		RawData = new char[RawDataSize];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (buf) of char objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	inF.read(RawData, RawDataSize);
	RawData[RawDataSize-1] = '\0';
	inF.close();
#ifdef OUTPUT_LOG_L2
	cout << "*** DataIn::DataIn(char*) completed.\n\n";
#endif
}



DataIn::~DataIn()
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataIn::~DataIn() started...\n";
#endif
	if( RawData != NULL )
		delete RawData;
#ifdef OUTPUT_LOG_L2
	cout << "*** DataIn::~DataIn() completed.\n\n";
#endif
}



/* reads and returns block of data */
/* id: data block id */
DataBlock* DataIn::ReadDataBlock(unsigned int id)
{
#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock* DataIn::ReadDataBlock(unsigned int) started...\n";
#endif
	char *buf, *buf_, id_, symb_aux;
	buf =  buf_ = RawData;
	char id_found = 0;
	for(unsigned int i=0; (i < RawDataSize) && !id_found; i++)
	{
		if( buf_[i] == '[' )   // [id] is a separator sequence, id is the data block id
		{
			i++;
			for(unsigned int j = i+1; j < RawDataSize; j++)
			{
				if( buf_[j] == ']' )
				{
					symb_aux = buf_[j];
					buf_[j] = '\0';
					sscanf(&buf_[i], "%i", &id_);
					if( id == id_ )
					{
						id_found++;
						buf = &buf_[j];
						//buf++;
					}
					buf_[j] = symb_aux;
					break;
				}
			}
		}
	}

	for(unsigned int i=0; i < RawDataSize; i++)
	{
		if( buf[i] >= '0' && buf[i] <= '9')
		{
			buf = &buf[i];
			break;
		}
	}

	unsigned int bufSize;
	if( id_found )
	{
		for(unsigned int i=0; ; i++)
		{
			if( (buf[i] == '[') || (buf[i] == '\0') )
			{
				symb_aux = buf[i];
				buf[i] = '\0';
				bufSize = i;
				break;
			}
		}
	}
	else
	{
		cerr << "*** Data block " << id << " not found. Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	unsigned int numRows = 0, numCols = 0;
	unsigned int i, j, row, col;
	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			i = l-1;
		}
		if( (buf[i] == '\n' || buf[i] == '\0') && (col > 0) )
		{
			if( numCols == 0 )
				numCols = col;
			else
				if( numCols != col )
					throw TABLE_UNDEFINED;
			col = 0;
			j=0;
		}
	}
	numRows = row;

	DataBlock *pDB = new DataBlock(numRows, numCols);

	for(i = j = row = col = 0; i < bufSize; i++)
	{
		for( ; i < bufSize; i++)
		{
			if( (buf[i] < '0' || buf[i] > '9') && buf[i] != '.' && buf[i] != '+' && buf[i] != '-' && buf[i] != 'e' )
				break;
			col++;
			if( j == 0 )
			{
				j++;
				row += j;
			}
			unsigned int l;
			for(l = i+1; l < bufSize; l++)
			{
				if( (buf[l] < '0' || buf[l] > '9') && buf[l] != '.' && buf[l] != '+' && buf[l] != '-' && buf[l] != 'e' )
					break;
			}
			char ch = buf[l];
			buf[l] = '\0';
			sscanf(&buf[i], "%f", &pDB->Mtrx[row-1][col-1]);
			buf[l] = ch;
			i = l-1;
		}
		if( buf[i] == '\n' || buf[i] == '\0' )
		{
			col = 0;
			j=0;
		}
	}

	buf[bufSize] = symb_aux;

#ifdef OUTPUT_LOG_L2
	cout << "*** DataBlock* DataIn::ReadDataBlock(unsigned int) completed.\n\n";
#endif

	return pDB;
}

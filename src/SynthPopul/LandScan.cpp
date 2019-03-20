/* LandScan.cpp, part of the Global Epidemic Simulation v1.0 BETA
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
#include "Keywords.h"



/* default constructor; creates undefined table */
LndScnCellTbl::LndScnCellTbl() : NumCols(0), NumRows(0), NumRecs(0), XLLCorner((float)UNDEFINED), YLLCorner((float)UNDEFINED), CellSize((float)UNDEFINED), Table(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::LndScnCellTblCl() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::LndScnCellTblCl() completed\n\n";
#endif
};



/* finds keyword in buffer and returns pointer to it (index in buffer) */
/* buf: pointer to buffer; bufSize: size of buffer; keyWord: pointer to keyword */
unsigned int FindKeyWord(char* buf, unsigned int bufSize, char *keyWord)
{
	unsigned int i, j, ptr;
	for(i=j=0; i < bufSize; i++)
	{
		if( buf[i] == keyWord[j] )
		{
			for(ptr = i++, j++; i < bufSize; i++, j++)
			{			
				if( buf[i] == ' ' && keyWord[j] == '\0' )
					return ptr;
				if( buf[i] != keyWord[j] )
				{
					j=0;
					break;
				}
			}
		}
	}
	return (unsigned int)UNDEFINED;
}



/* returns parameter value */
/* buf: pointer to buffer; bufSize: size of buffer; keyWordPtr: pointer (index in buffer) to keyword */
float GetParam(char* buf, unsigned int bufSize, unsigned int keyWordPtr)
{
	float val;
	char ch;
	unsigned int i, j;
	for(i = keyWordPtr; i < bufSize; i++)
		if( (buf[i] >= '0' && buf[i] <= '9') || buf[i] == '-'  )
			break;
	for(j = i; j < bufSize; j++)
		if( (buf[j] < '0' || buf[j] > '9') && buf[j] != '.' && buf[j] != '+' && buf[j] != '-' && buf[j] != 'e' )
			break;
	ch = buf[j];
	buf[j] = '\0';
	sscanf(&buf[i], "%f", &val);
	buf[j] = ch;

	return val;
}



/* constructor; creates binary representation of LandScan dataset */
/* hdrFName: pointer to header file name; tblFName: pointer to table file name */
LndScnCellTbl::LndScnCellTbl(char *hdrFName, char *tblFName)
{	
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::LndScnCellTblCl(char*, char*) started...\n";
#endif

	fstream inHdr(hdrFName, ios::in | ios::binary);
	if( !inHdr )
		throw FILE_OPEN_ERROR;
	inHdr.seekg(0, ios::end);
	unsigned int bufSize = inHdr.tellg();
	inHdr.seekg(0, ios::beg);
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
	inHdr.read(buf, bufSize);
	buf[bufSize-1] = '\0';
	inHdr.close();

	unsigned int DataTypeID = UNDEFINED;   // data type ID
#define LS 0                               // LandScan
#define BIL 1                              // BIL

	unsigned int keyWordPtr;

	// check the input data type
	keyWordPtr = FindKeyWord(buf, bufSize, xllcorner);
	if( keyWordPtr != UNDEFINED )                       // LandScan type header
		DataTypeID = LS;
	keyWordPtr = FindKeyWord(buf, bufSize, ulxmap);
	if( keyWordPtr != UNDEFINED )
		DataTypeID = BIL;                               // BIL type header
	if( DataTypeID == UNDEFINED )
	{
		cerr << "*** Uknown population grid header file format (CRITICAL).  Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	if( DataTypeID == LS )
	{
		cout << "LandScan header detected\n";

		keyWordPtr = FindKeyWord(buf, bufSize, ncols);
		NumCols = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, nrows);
		NumRows = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, xllcorner);
		XLLCorner = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, yllcorner);
		YLLCorner = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, cellsize);
		CellSize = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, nodata_value);
		NODATA_value = GetParam(buf, bufSize, keyWordPtr);
	}
	else if( DataTypeID == BIL )
	{
		cout << "BIL header detected\n";

		keyWordPtr = FindKeyWord(buf, bufSize, ncols_BIL);
		NumCols = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, nrows_BIL);
		NumRows = GetParam(buf, bufSize, keyWordPtr);

		keyWordPtr = FindKeyWord(buf, bufSize, xdim);
		CellSize = GetParam(buf, bufSize, keyWordPtr);

		float CellSizeY;
		keyWordPtr = FindKeyWord(buf, bufSize, ydim);
		CellSizeY = GetParam(buf, bufSize, keyWordPtr);
		if( abs(CellSize - CellSizeY) > TOLER_VAL )
		{
			cerr << "*** This type of BIL population grid map format is not supported (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}

		keyWordPtr = FindKeyWord(buf, bufSize, ulxmap);
		XLLCorner = GetParam(buf, bufSize, keyWordPtr) - CellSize/2.0f;

		keyWordPtr = FindKeyWord(buf, bufSize, ulymap);
		YLLCorner = GetParam(buf, bufSize, keyWordPtr) + CellSize/2.0f - (float)NumRows*CellSize;

		keyWordPtr = FindKeyWord(buf, bufSize, nodata);
		NODATA_value = GetParam(buf, bufSize, keyWordPtr);
	};

	delete [] buf;

	ifstream inTbl(tblFName, ios::in | ios::binary);
	if( !inTbl )
		throw FILE_OPEN_ERROR;

	float inVal;
	for(NumRecs = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		//if( inVal != NODATA_value && inVal > 0.0f )
		if( inVal > 0.0f )
			NumRecs++;
	}
	inTbl.clear();
	inTbl.seekg(0, ios::beg);

	try
	{
		Table = new LndScnCell[NumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of LndScnCellTblCl::LndScnCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	unsigned int indx, i, j;
	for(indx = i = j = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		if( j == NumCols )
		{
			j=0;
			i++;
		}
		//if( inVal == NODATA_value || inVal == 0.0f )
		if( inVal <= 0.0f )
		{
			j++;
			continue;
		}

		Table[indx].LndScnX = j;
		Table[indx].LndScnY = i;
		Table[indx].NumHosts = (unsigned int)inVal;
		j++;
		indx++;
	}

	inTbl.close();

#ifdef OUTPUT_LOG_TEST_L1
	cout << "Population grid table:\nNumber of rows: " << NumRows << "; Number of columns: " << NumCols \
		<< "\nNumber of nonempty records read: " << indx \
		<< "\nxllcorner: " << XLLCorner << "; yllcorner: " << YLLCorner << "; cellsize: " << CellSize << "\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::LndScnCellTblCl(char*, char*) completed.\n\n";
#endif
}



LndScnCellTbl::~LndScnCellTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::~LndScnCellTblCl() started...\n";
#endif 
	if( Table != NULL )
	{
		delete [] Table;
		Table = NULL;
	}
#ifdef OUTPUT_LOG_L1
	cout << "*** LndScnCellTblCl::~LndScnCellTblCl() completed\n\n";
#endif
};


	
/* default constructor; creates undefined table */
RstrCellTbl::RstrCellTbl() : NumCols(0), NumRows(0), NumRecs(0), XLLCorner((float)UNDEFINED), YLLCorner((float)UNDEFINED), CellSize((float)UNDEFINED), Table(NULL)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::RstrCellTblCl() started...\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::RstrCellTblCl() completed\n\n";
#endif
}



/* constructor; creates binary representation of country borders dataset */
/* hdrFName: pointer to header file name; tblFName: pointer to table file name */
RstrCellTbl::RstrCellTbl(char *hdrFName, char *tblFName)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::RstrCellTblCl(char*, char*) started...\n";
#endif

	fstream inHdr(hdrFName, ios::in | ios::binary);
	if( !inHdr )
		throw FILE_OPEN_ERROR;
	inHdr.seekg(0, ios::end);
	unsigned int bufSize = inHdr.tellg();
	inHdr.seekg(0, ios::beg);
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
	inHdr.read(buf, bufSize);
	buf[bufSize-1] = '\0';
	inHdr.close();

	unsigned int keyWordPtr;
	keyWordPtr = FindKeyWord(buf, bufSize, ncols);
	NumCols = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, nrows);
	NumRows = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, xllcorner);
	XLLCorner = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, yllcorner);
	YLLCorner = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, cellsize);
	CellSize = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, nodata_value);
	NODATA_value = GetParam(buf, bufSize, keyWordPtr);

	delete [] buf;

	ifstream inTbl(tblFName, ios::in | ios::binary);
	if( !inTbl )
		throw FILE_OPEN_ERROR;

	float inVal;
	for(NumRecs = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		//if( inVal != NODATA_value)
		if( inVal >= 0.0f )
			NumRecs++;
	}
	inTbl.clear();
	inTbl.seekg(0, ios::beg);

	try
	{
		Table = new RstrCell[NumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of RstrCellTblCl::RstrCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	unsigned int indx, i, j;
	for(indx = i = j = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		if( j == NumCols )
		{
			j=0;
			i++;
		}
		//if( inVal == NODATA_value )
		if( inVal < 0.0f )
		{
			j++;
			continue;
		}

		Table[indx].RstrX = j;
		Table[indx].RstrY = i;
		Table[indx].Code = (unsigned short)inVal;
		j++;
		indx++;
	}

	inTbl.close();

#ifdef OUTPUT_LOG_TEST_L1
		cout << "Country border raster cell table:\nNumber of rows: " << NumRows << "; Number of columns: " << NumCols \
			<<"\nNumber of records (raster cells) read: " << indx \
			<< "\nxllcorner: " << XLLCorner	<< "; yllcorner: " << YLLCorner << "; cellsize: " << CellSize << "\n";
#endif
#ifdef OUTPUT_LOG_L1
		cout << "*** RstrCellTblCl::RstrCellTblCl(char*, char*) completed.\n\n";
#endif
}



/* constructor; creates binary representation of country border dataset for a given country */
/* cntryCode: country code; hdrFName: pointer to header file name; tblFName: pointer to table file name */
RstrCellTbl::RstrCellTbl(unsigned short cntryCode, char *hdrFName, char *tblFName)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::RstrCellTblCl(unsigned short, char*, char*) started...\n";
#endif
	fstream inHdr(hdrFName, ios::in | ios::binary);
	if( !inHdr )
		throw FILE_OPEN_ERROR;
	inHdr.seekg(0, ios::end);
	unsigned int bufSize = inHdr.tellg();
	inHdr.seekg(0, ios::beg);
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
	inHdr.read(buf, bufSize);
	buf[bufSize-1] = '\0';
	inHdr.close();

	unsigned int keyWordPtr;
	keyWordPtr = FindKeyWord(buf, bufSize, ncols);
	NumCols = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, nrows);
	NumRows = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, xllcorner);
	XLLCorner = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, yllcorner);
	YLLCorner = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, cellsize);
	CellSize = GetParam(buf, bufSize, keyWordPtr);

	keyWordPtr = FindKeyWord(buf, bufSize, nodata_value);
	NODATA_value = GetParam(buf, bufSize, keyWordPtr);

	delete [] buf;

	ifstream inTbl(tblFName, ios::in | ios::binary);
	if( !inTbl )
		throw FILE_OPEN_ERROR;

	float inVal;
	float cntryCode_aux = (float)cntryCode;
	for(NumRecs = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		if( inVal == cntryCode_aux )
			NumRecs++;
	}
	inTbl.clear();
	inTbl.seekg(0, ios::beg);

	try
	{
		Table = new RstrCell[NumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of RstrCellTblCl::RstrCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	unsigned int indx, i, j;
	for(indx = i = j = 0; !inTbl.eof(); )
	{
		inTbl.read((char*)&inVal, sizeof(inVal));
		if( j == NumCols )
		{
			j=0;
			i++;
		}
		if( inVal != cntryCode_aux )
		{
			j++;
			continue;
		}
		Table[indx].RstrX = j;
		Table[indx].RstrY = i;
		Table[indx].Code = (unsigned short)inVal;
		j++;
		indx++;
	}

	inTbl.close();

#ifdef OUTPUT_LOG_TEST_L1
		cout << "Country border raster cell table:\nNumber of rows: " << NumRows << "; Number of columns: " << NumCols \
			<< "\nNumber of records (raster cells) read: " << indx << "\nxllcorner: " << XLLCorner\
			<< "; yllcorner: " << YLLCorner << "; cellsize: " << CellSize << "\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::RstrCellTblCl(unsigned short, char*, char*) completed.\n\n";
#endif
}



RstrCellTbl::~RstrCellTbl()
{
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::~RstrCellTblCl() started...\n";
#endif
	if( Table != NULL )
	{
		delete [] Table;
		Table = NULL;
	}
#ifdef OUTPUT_LOG_L1
	cout << "*** RstrCellTblCl::~RstrCellTblCl() completed\n\n";
#endif
};



/* constructor; builds a table of LandScan cells for a given country based on the general LandScan population dataset and country borders raster dataset */
/* cntryCode: country code; pLSCTbl: pointer to table of LandScan cells for the whole world; pRCTbl: pointer to country border raster dataset */
CntryLndScnCellTbl::CntryLndScnCellTbl(unsigned short cntryCode, LndScnCellTbl *pLSCTbl, RstrCellTbl *pRCTbl)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** CntryLndScnCellTblCl::CntryLndScnCellTblCl(unsigned short, LndScnCellTblCl&, RstrCellTblCl&, unsigned int&) started...\n";
#endif
	/* index corrections for proper cell sets alignment */
	unsigned int X_corr = floor((pLSCTbl->XLLCorner - pRCTbl->XLLCorner)/pRCTbl->CellSize);   // correction for X index for country border raster dataset
	float yulcorner_LS = pLSCTbl->YLLCorner + pLSCTbl->CellSize * pLSCTbl->NumRows;
	float yulcorner_R = pRCTbl->YLLCorner + pRCTbl->CellSize * pRCTbl->NumRows;
	unsigned int Y_corr = floor((yulcorner_LS - yulcorner_R)/pRCTbl->CellSize);               // correction for Y index for country border raster dataset

	unsigned int cntryNumRecs = 0;
	unsigned int startIndx = (unsigned int)UNDEFINED;
	unsigned int i, j, k;
	for(i=0; i < pRCTbl->NumRecs; i++)
	{
		if( pRCTbl->Table[i].Code == cntryCode )
		{
			pRCTbl->Table[i].RstrX += X_corr;      // correct X index
			pRCTbl->Table[i].RstrY += Y_corr;      // correct Y index
			cntryNumRecs++;                        // count the number of records for the country in the raster table
			if( startIndx == (unsigned int)UNDEFINED )
				startIndx = i;                     // index of the first record corresponding to the country in this table
		}
	}
	if( startIndx == (unsigned int)UNDEFINED )     // no appropriate country found
		throw NO_COUNTRY_FOUND;

	NumCols = pLSCTbl->NumCols;                    // create a table of LandScan cells for the country and copy the data from LSTbl
	NumRows = pLSCTbl->NumRows;
	XLLCorner = pLSCTbl->XLLCorner;
	YLLCorner = pLSCTbl->YLLCorner;
	CellSize = pLSCTbl->CellSize;
	NODATA_value = pLSCTbl->NODATA_value;

	LndScnCell *Table_aux;
	try
	{
		Table_aux = new LndScnCell[cntryNumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table_aux) of LndScnCellTblCl::LndScnCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}

	for(i = startIndx, j=k=0, NumHosts = 0; i < pRCTbl->NumRecs; i++)
	{
		if( pRCTbl->Table[i].Code == cntryCode )
		{
			for( ; (j < pLSCTbl->NumRecs) && (pLSCTbl->Table[j].LndScnY < pRCTbl->Table[i].RstrY); j++);
			for( ; (j < pLSCTbl->NumRecs) && (pLSCTbl->Table[j].LndScnY == pRCTbl->Table[i].RstrY) && (pLSCTbl->Table[j].LndScnX <= pRCTbl->Table[i].RstrX); j++)
			{
				if( pRCTbl->Table[i].RstrX == pLSCTbl->Table[j].LndScnX )
				{
					Table_aux[k++] = pLSCTbl->Table[j];
					NumHosts += pLSCTbl->Table[j].NumHosts;
					j++;
					break;
				}
			}
		}
	}

	/*
	for(i = StartIndx, j=k=0, NumHosts = 0; i < pRCTbl->NumRecs; i++)
	{
		if( pRCTbl->Table[i].Code == cntryCode )
		{
			for( ; j < pLSCTbl->NumRecs; j++)
			{
				if( pRCTbl->Table[i].RstrY == pLSCTbl->Table[j].LndScnY )
				{
					if( pRCTbl->Table[i].RstrX == pLSCTbl->Table[j].LndScnX )
					{
						Table_aux[k++] = pLSCTbl->Table[j];
						NumHosts += pLSCTbl->Table[j].NumHosts;
						j++;
						break;
					}
					else if( pRCTbl->Table[i].RstrX < pLSCTbl->Table[j].LndScnX )
						break;
				}
				else if( pRCTbl->Table[i].RstrY < pLSCTbl->Table[j].LndScnY )
					break;
			}
		}
	}
	*/

	NumRecs = k;
	try
	{
		Table = new LndScnCell[NumRecs];
	}
	catch(...)
	{
		cerr << "*** Error allocating array (Table) of LndScnCellTblCl::LndScnCell objects (CRITICAL). Program will be terminated. ***\n";
		exit(EXIT_FAILURE);
	}
	for(i=0; i < NumRecs; i++)
		Table[i] = Table_aux[i];
	delete Table_aux;
#ifdef OUTPUT_LOG_TEST_L1
	cout << "Country code: " << cntryCode << "; Population: " << NumHosts << "\n" \
		<< "Coordinates of the first cell: Lat = " <<  LndScnCellLat(0) << ", Lon = " << LndScnCellLon(0) << "\n" \
		<< "Coordinates of the last cell: Lat = " <<  LndScnCellLat(NumRecs-1) << ", Lon = " << LndScnCellLon(NumRecs-1) << "\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** CntryLndScnCellTblCl::CntryLndScnCellTblCl(unsigned short, LndScnCellTblCl&, RstrCellTblCl&, unsigned int&) completed\n\n";
#endif
};



/* scales down model population */
/* SclDwnFactor: scale-down factor */
void CntryLndScnCellTbl::ScaleDownPopul(float sclDwnFactor)
{
#ifdef OUTPUT_LOG_L1
	cout << "*** CntryLndScnCellTblCl::ScaleDownPopul(float) started...\n";
#endif
	unsigned int i, j, numHostsSD, numRecsSD;
	for(i=0, NumHosts = 0, numRecsSD = 0; i < NumRecs; i++)
	{
		numHostsSD = floor(((float)Table[i].NumHosts)/sclDwnFactor);   // scale down the population and count the number of unempty records
		Table[i].NumHosts = numHostsSD;
		NumHosts += numHostsSD;
		if( numHostsSD > 0)
			numRecsSD++;
	}
	
	if( NumRecs > numRecsSD)                                           // new table is smaller - rebuild it to remove records containing no hosts
	{
		LndScnCell *TableSD;
		try
		{
			TableSD = new LndScnCell[numRecsSD];
		}
		catch(...)
		{
			cerr << "*** Error allocating array (TableSD) of LndScnCellTblCl::LndScnCell objects (CRITICAL). Program will be terminated. ***\n";
			exit(EXIT_FAILURE);
		}
		for(i=j=0; i < NumRecs; i++)
		{
			if( Table[i].NumHosts > 0 )
				TableSD[j++] = Table[i];
		}
		delete [] Table;
		Table = TableSD;
		NumRecs = numRecsSD;
	};
#ifdef OUTPUT_LOG_TEST_L1
	cout << "Scale down factor: " << sclDwnFactor << "; New popultaion size: " << NumHosts << "\n";
#endif
#ifdef OUTPUT_LOG_L1
	cout << "*** CntryLndScnCellTblCl::ScaleDownPopul(float) completed\n";
#endif
}



/* returns Lat coordinate (upper left corner) of LandScan cell described by table record */
/* indx: index of record */
double CntryLndScnCellTbl::LndScnCellLat(unsigned int indx)
{
	return (YLLCorner + NumRows * CellSize - CellSize * Table[indx].LndScnY);
}



/* returns Lon coordinate (upper left corner) of LandScan cell described by table record */
/* indx: index of record */
double CntryLndScnCellTbl::LndScnCellLon(unsigned int indx)
{
	return (XLLCorner + CellSize * Table[indx].LndScnX);
}


/* returns Lat coordinate (upper left corner) for LandScan cell with index Y */
double CntryLndScnCellTbl::LndScnYToLat(unsigned int y)
{
	return (YLLCorner + NumRows * CellSize - CellSize * y);
}



/* returns cell index Y in LandScan grid for point with latitude lat */
unsigned int CntryLndScnCellTbl::LatToLndScnY(double lat)
{
	return floor(((double)YLLCorner + (double)NumRows * (double)CellSize - lat)/(double)CellSize);
}



/* returns Lon coordinate (upper left corner) for LandScan cell with index X */
double CntryLndScnCellTbl::LndScnXToLon(unsigned int x)
{
	return (XLLCorner + CellSize * x);
}


	
/* returns cell index X in LandScan grid for point with longitude lon */
unsigned int CntryLndScnCellTbl::LonToLndScnX(double lon)
{
	return floor((lon - (double)XLLCorner)/(double)CellSize);
}



/* returns distance between LandScan cells (LndScnX1, LndScnY1) and (LndScnX2, LndScnY2) */
double CntryLndScnCellTbl::LndScnCellDist(unsigned int LndScnX1, unsigned int LndScnY1, unsigned int LndScnX2, unsigned int LndScnY2)
{
	if( (LndScnX1 == LndScnX2) && (LndScnY1 == LndScnY2) )
		return 0.0;

	double latRad1 = PI/180.0 * LndScnYToLat(LndScnY1);
	double latRad2 = PI/180.0 * LndScnYToLat(LndScnY2);
	double latSepRad = latRad1 - latRad2;
	double lonSepRad = PI/180.0 * (LndScnXToLon(LndScnX2) - LndScnXToLon(LndScnX1));

	double var = sin(latSepRad * 0.5)*sin(latSepRad * 0.5) + cos(latRad1) * cos(latRad2) * sin(lonSepRad * 0.5)*sin(lonSepRad * 0.5);

	return 2.0 * GEO_MRADIUS * asin(min(1.0, sqrt(var)));
}



/* returns distance between two points with coordinates (lat1, lon1) and (lat2, lon2) */
double CntryLndScnCellTbl::PointDist(double lat1, double lon1, double lat2, double lon2)
{
	double latRad1 = PI/180.0 * lat1;
	double latRad2 = PI/180.0 * lat2;
	double latSepRad = latRad1 - latRad2;
	double lonSepRad = PI/180.0 * (lon2 - lon1);

	double var = sin(latSepRad * 0.5) * sin(latSepRad * 0.5) + cos(latRad1) * cos(latRad2) * sin(lonSepRad * 0.5) * sin(lonSepRad * 0.5);

	return 2.0 * GEO_MRADIUS * asin(min(1.0, sqrt(var)));
}

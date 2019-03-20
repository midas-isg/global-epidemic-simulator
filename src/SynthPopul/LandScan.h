/* LandScan.h, part of the Global Epidemic Simulation v1.0 BETA
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




/* classes to work with LandScan dataset */

#ifndef LANDSCAN
#define LANDSCAN



#include "Includes_n_Definitions.h"




/* table of LandScan cells */
class LndScnCellTbl
{
public:
	/* LandScan cell */
	class LndScnCell
	{
	public:
		unsigned int LndScnX;      // LandScan X index
		unsigned int LndScnY;      // LandScan Y index
		unsigned int NumHosts;     // number of hosts in a LandScan cell
	};

	unsigned int NumCols;          // number of columns
	unsigned int NumRows;          // number of rows
	unsigned int NumRecs;          // number of records in the table; NumRecs = NumCols*NumRows for the _whole_ LandScan dataset
	float XLLCorner;               // x-coordinate of the lower-left corner of the lower-left cell
	float YLLCorner;               // y-coordinate of the lower-left corner of the lower-left cell
	float CellSize;                // LandScan cell angular resolution (30 arcseconds) in degrees
	float NODATA_value;            // no data attribute
	LndScnCell *Table;             // pointer to the table

	/* default constructor; creates undefined table */
	LndScnCellTbl();

	/* constructor; creates binary representation of LandScan dataset */
	/* hdrFName: pointer to header file name; tblFName: pointer to table file name */
	LndScnCellTbl(char *hdrFName, char *tblFName);

	~LndScnCellTbl();
};



/* table of country border data (raster) */
class RstrCellTbl
{
public:
	/* raster cell */
	class RstrCell
	{
	public:
		unsigned int RstrX;        // raster X index
		unsigned int RstrY;        // raster Y index
		unsigned short Code;       // code of a raster cell
	};

	unsigned int NumCols;          // number of columns
	unsigned int NumRows;          // number of rows
	unsigned int NumRecs;          // number of records in the table; NumRecs = NumCols*NumRows for the _whole_ raster dataset
	float XLLCorner;               // x-coordinate of the lower-left corner of the lower-left cell
	float YLLCorner;               // y-coordinate of the lower-left corner of the lower-left cell
	float CellSize;                // LandScan cell angular resolution (30 arcseconds) in degrees
	float NODATA_value;            // no data attribute
	RstrCell *Table;               // pointer to the table

	/* default constructor; creates undefined table */
	RstrCellTbl();

	/* constructor; creates binary representation of country border dataset */
	/* hdrFName: pointer to header file name; tblFName: pointer to table file name */
	RstrCellTbl(char *hdrFName, char *tblFName);

	/* constructor; creates binary representation of country border dataset for a given country */
	/* cntryCode: country code; hdrFName: pointer to header file name; tblFName: pointer to table file name */
	RstrCellTbl(unsigned short cntryCode, char *hdrFName, char *tblFName);

	~RstrCellTbl();
};



/* table of LandScan cells for a given country */
class CntryLndScnCellTbl : public LndScnCellTbl
{
public:
	unsigned int NumHosts;         // number of hosts in the country

	/* constructor; builds a table of LandScan cells for a given country based on the general LandScan population dataset and country borders raster dataset */
	/* cntryCode: country code; pLSCTbl: pointer to table of LandScan cells for the whole world; pRCTbl: pointer to country border raster dataset */
	CntryLndScnCellTbl(unsigned short cntryCode, LndScnCellTbl *pLSCTbl, RstrCellTbl *pRCTbl);

	/* scales down model population */
	/* sclDwnFactor: scale-down factor */
	void ScaleDownPopul(float sclDwnFactor);		

	/* returns Lat coordinate of LandScan cell described by table record */
	/* indx: index of record */
	double LndScnCellLat(unsigned int indx);

	/* returns Lon coordinate of LandScan cell described by table record */
	/* indx: index of record */
	double LndScnCellLon(unsigned int indx);

	/* returns lattitude for LandScan cell with index Y */
	double LndScnYToLat(unsigned int y);

	/* returns cell index Y in LandScan grid for point with latitude lat */
	unsigned int LatToLndScnY(double lat);

	/* returns longitude for LandScan cell with index X */
	double LndScnXToLon(unsigned int x);

	/* returns cell index X in LandScan grid for point with longitude lon */
	unsigned int LonToLndScnX(double lon);

	/* returns distance between LandScan cells (LndScnX1, LndScny1) and (LndScnX2, LndScny2) */
	double LndScnCellDist(unsigned int LndScnX1, unsigned int LndScnY1, unsigned int LndScnX2, unsigned int LndScnY2);

	/* returns distance between two points with coordinates (lat1, lon1) and (lat2, lon2) */
	static double PointDist(double lat1, double lon1, double lat2, double lon2);
};



#endif

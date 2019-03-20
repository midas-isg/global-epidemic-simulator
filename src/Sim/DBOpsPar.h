/* DBOpsPar.h, part of the Global Epidemic Simulation v1.0 BETA
/* Header for functions involving ODBC database access
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

#include <iostream>
#ifdef _WIN32
  #include <windows.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <sstream>
#include "stdio.h"

  #include <sql.h>
  #include <sqlext.h>

using namespace std;


// exception codes
#define BUFF_LENGTH_TOO_SMALL    1
#define ALLOC_ENV_HNDL_ERROR     2
#define SETTING_ENV_ATTR_ERROR   3
#define ALLOC_DB_CONN_HNDL_ERROR 4
#define DB_CONNECT_ERROR         5
#define SET_CONNECT_ATTR_ERROR   6
#define ALLOC_STMT_HNDL_ERROR    7
#define CREATE_TABLE_ERROR       8
#define CREATE_SCHEMA_ERROR      9
#define OBJ_EXISTS_ERROR         10
#define INSERT_PREP_ERROR        11
#define WRONG_NUM_PARAMS         12
#define PARAM_BIND_ERROR         13
#define TRAN_COMMIT_ERROR        14
#define TRAN_ROLLBACK_ERROR      15
#define INS_REC_ERROR            16


class DBOps
{
private:
	static SQLHENV HEnv;    // environment handle
	SQLHANDLE HDbc;         // connection handle
	SQLHSTMT  HInsStmt;     // SQL INSERT statement handle

	static const char CrSchmStmtPrt1[];

#define DBOPS_MAX_BUFF_LENGTH 256
	char ServerName[SQL_MAX_DSN_LENGTH];    // buffer for a data source name
	char UserName[DBOPS_MAX_BUFF_LENGTH];   // buffer for a user name
	char Passwd[DBOPS_MAX_BUFF_LENGTH];     // buffer for a password

	int num_values;
	int* Val;
	char* create_table;
	char* insert_statement;
	char* drop_table;

public:
	DBOps(char *ServerName, char *UserName, char *Passwd);
	~DBOps();
	void prepareCommands(int no_place_types, char* tblName);
	void Connect();
	void Disconnect();
	void CreateTable();
	void DropTable();
	void CreateSchema(char *ShmName);
	void PrepareSQLInsertStmt();
	void InsertRecord(int *InsVal);
	void DeleteSQLInsertStmt();
	void CommitInsChanges();
	void RollbackInsChanges();
};


/* DBOpsPar.cpp, part of the Global Epidemic Simulation v1.0 BETA
/* Handle ODBC and saving results to DB via SQL
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
#include "DBOpsPar.h"


SQLHENV DBOps::HEnv = NULL;
const char DBOps::CrSchmStmtPrt1[] = "CREATE SCHEMA ";

void DBOps::prepareCommands(int no_place_types, char* tableName) {
  num_values = 6+(2*no_place_types);
  Val = new int[num_values];

  string create = "CREATE TABLE ";
  string insert = "INSERT INTO ";
  string drop = "DROP TABLE ";
  create.append(tableName);
  insert.append(tableName);
  drop.append(tableName);
  create.append(" ([Time_hrs] [int] NOT NULL, [Unit_id] [int] NOT NULL, [Contact_makers] [int] NOT NULL,");
  create.append("[Comm_cases] [int] NOT NULL, ");
  create.append("[Comm_infs] [int] NOT NULL, [HH_cases] [int] NOT NULL, [HH_infs] [int] NOT NULL, ");
  create.append("[Current_sympt] [int] NOT NULL, [Current_nonsympt] [int] NOT NULL");
  insert.append(" (Time_hrs,Unit_id,Contact_makers,Comm_cases,Comm_infs,HH_cases,HH_infs,Current_sympt,Current_nonsympt");

  if (no_place_types==0) {
    create.append(")");
    insert.append(") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

  } else {
    for (int i=0; i<no_place_types; i++) {
      stringstream out;
      out << i;
      create.append(", [P");
      insert.append(",P");
      create.append(out.str());
      insert.append(out.str());
      create.append("_cases] [int] NOT NULL, [P");
      insert.append("_cases,P");
      create.append(out.str());
      insert.append(out.str());
      insert.append("_infs");
      create.append("_infs] [int] NOT NULL");
    }
    create.append(")");
    insert.append(") VALUES (?");
    for (int i=0; i<(no_place_types*2)+9; i++) insert.append(", ?");
    insert.append(")");
  }
  create_table = new char[create.length()+1];
  for (unsigned int i=0; i<create.length(); i++) create_table[i]=create.at(i);
  create_table[create.length()]='\0';

  insert_statement = new char[insert.length()+1];
  for (unsigned int i=0; i<insert.length(); i++) insert_statement[i]=insert.at(i);
  insert_statement[insert.length()]='\0'; 

  drop_table = new char[drop.length()+1];
  for (unsigned int i=0; i<drop.length(); i++) drop_table[i]=drop.at(i);
  drop_table[drop.length()]='\0';

  insert.clear();
  create.clear();


}

DBOps::DBOps(char *ServerName, char *UserName, char *Passwd)
{
	if( strlen(ServerName) > DBOPS_MAX_BUFF_LENGTH )
		throw BUFF_LENGTH_TOO_SMALL;
	strcpy(this->ServerName, ServerName);

	if( strlen(UserName) > DBOPS_MAX_BUFF_LENGTH )
		throw BUFF_LENGTH_TOO_SMALL;
	strcpy(this->UserName, UserName);

	if( strlen(Passwd) > DBOPS_MAX_BUFF_LENGTH )
		throw BUFF_LENGTH_TOO_SMALL;
	strcpy(this->Passwd, Passwd);
	
	SQLRETURN RetCode;      // ODBC return code
	
	// use one global ODBC environment for a node but multiple connections for threads
#pragma omp critical (alloc_env_handle)
{
	if( HEnv == NULL )
	{
		RetCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &HEnv);
		if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
		{
			cerr << "Error allocating environment handle\n";
			throw ALLOC_ENV_HNDL_ERROR;
		}


		RetCode = SQLSetEnvAttr(HEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
		if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
		{
			cerr << "Error setting environment attributes (ODBC version)\n";
			SQLFreeHandle(SQL_HANDLE_ENV, HEnv);
			throw SETTING_ENV_ATTR_ERROR;
		}
	}
}

	RetCode = SQLAllocHandle(SQL_HANDLE_DBC, HEnv, &HDbc);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error allocating database connection handle\n";
		SQLFreeHandle(SQL_HANDLE_ENV, HEnv);
		throw ALLOC_DB_CONN_HNDL_ERROR;
	} 

}


DBOps::~DBOps()
{
#pragma omp critical (free_env_handle)
{
	if( HEnv != NULL )
	{
		SQLFreeHandle(SQL_HANDLE_ENV, HEnv);
		HEnv = NULL;
	}
}
	Disconnect();
}


void DBOps::Connect()
{
	SQLRETURN RetCode;      // ODBC return code

	RetCode = SQLConnect(HDbc, (SQLCHAR*)ServerName, SQL_NTS, (SQLCHAR*)UserName, SQL_NTS, (SQLCHAR*)Passwd, SQL_NTS);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error connecting to a datasource\n";
		SQLFreeHandle(SQL_HANDLE_DBC, HDbc);
		throw DB_CONNECT_ERROR;
	}
	else
		cout << "Connected to a data source " << ServerName << " \n";
	

	// set AUTOCOMMIT_OFF - all transactions are to be committed manually 
	RetCode = SQLSetConnectAttr(HDbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, (SQLINTEGER)NULL);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error setting a connection attribute\n";
		throw SET_CONNECT_ATTR_ERROR;
	}
}


void DBOps::Disconnect()
{
	SQLFreeHandle(SQL_HANDLE_DBC, HDbc);
	cout << "Disconnected from a datasource " << ServerName << " \n";
}


void DBOps::CreateTable()
{

  SQLRETURN RetCode;
	SQLHSTMT HCrTblStmt;

	RetCode = SQLAllocHandle(SQL_HANDLE_STMT, HDbc, &HCrTblStmt);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error allocating a statement handle\n";
		throw ALLOC_STMT_HNDL_ERROR;
	}

	RetCode = SQLExecDirect(HCrTblStmt, (SQLCHAR*)create_table, SQL_NTS);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
#define SQLSTATE_CODE_LNGTH 5
		SQLCHAR SQLState[SQLSTATE_CODE_LNGTH + 1];
		SQLCHAR SQLStateObjExists[] = "42S01"; 
		SQLSMALLINT TextLength;
		RetCode = SQLGetDiagField(SQL_HANDLE_STMT, HCrTblStmt, 1, SQL_DIAG_SQLSTATE, SQLState, SQLSTATE_CODE_LNGTH + 1, &TextLength);
		SQLFreeHandle(SQL_HANDLE_STMT, HCrTblStmt);

		for(int i=0; i < SQLSTATE_CODE_LNGTH; i++)
			if( SQLState[i] != SQLStateObjExists[i] )
			{
				cerr << "Error creating a table\n";
				throw CREATE_TABLE_ERROR;
			}

		throw OBJ_EXISTS_ERROR;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, HCrTblStmt);
	
	RetCode = SQLEndTran(SQL_HANDLE_DBC, HDbc, SQL_COMMIT);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error creating a table\n";
		throw CREATE_TABLE_ERROR;
	}
};

void DBOps::DropTable()
{
  SQLRETURN RetCode;
  SQLHSTMT HCrTblStmt;
  RetCode = SQLAllocHandle(SQL_HANDLE_STMT, HDbc, &HCrTblStmt);
  if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
  {
    cerr << "Error allocating a statement handle\n";
    throw ALLOC_STMT_HNDL_ERROR;
  }

  RetCode = SQLExecDirect(HCrTblStmt, (SQLCHAR*)drop_table, SQL_NTS);
  SQLFreeHandle(SQL_HANDLE_STMT, HCrTblStmt);
  RetCode = SQLEndTran(SQL_HANDLE_DBC, HDbc, SQL_COMMIT);

};



void DBOps::CreateSchema(char *SchmName)
{
	unsigned int StmtLngth = (unsigned int) (strlen(SchmName) + strlen(CrSchmStmtPrt1));
	char *Stmt = new char[StmtLngth+1];
	strcpy(Stmt, CrSchmStmtPrt1);
	strcat(Stmt, SchmName);

	SQLRETURN RetCode;
	SQLHSTMT HCrTblStmt;

	RetCode = SQLAllocHandle(SQL_HANDLE_STMT, HDbc, &HCrTblStmt);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error allocating a statement handle\n";
		throw ALLOC_STMT_HNDL_ERROR;
	}

	RetCode = SQLExecDirect(HCrTblStmt, (SQLCHAR*)Stmt, SQL_NTS);
	delete[] Stmt;
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
#define SQLSTATE_CODE_LNGTH 5
		SQLCHAR SQLState[SQLSTATE_CODE_LNGTH + 1];
		SQLCHAR SQLStateObjExists[] = "42S01"; 
		SQLSMALLINT TextLength;
		RetCode = SQLGetDiagField(SQL_HANDLE_STMT, HCrTblStmt, 1, SQL_DIAG_SQLSTATE, SQLState, SQLSTATE_CODE_LNGTH + 1, &TextLength);
		SQLFreeHandle(SQL_HANDLE_STMT, HCrTblStmt);

		for(int i=0; i < SQLSTATE_CODE_LNGTH; i++)
			if( SQLState[i] != SQLStateObjExists[i] )
			{
				cerr << "Error creating a shema\n";
				throw CREATE_SCHEMA_ERROR;
			}

		throw OBJ_EXISTS_ERROR;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, HCrTblStmt);
	
	RetCode = SQLEndTran(SQL_HANDLE_DBC, HDbc, SQL_COMMIT);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error creating a table\n";
		throw CREATE_TABLE_ERROR;
	}
}


void DBOps::PrepareSQLInsertStmt()
{
	// an appropriate table must already exist in the database!

	SQLRETURN RetCode;
	RetCode = SQLAllocHandle(SQL_HANDLE_STMT, HDbc, &HInsStmt);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error allocating a statement handle\n";
		throw ALLOC_STMT_HNDL_ERROR;
	}

	RetCode = SQLPrepare(HInsStmt, (SQLCHAR*)insert_statement, SQL_NTS);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error preparing an INSERT statement\n";
		SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
		throw INSERT_PREP_ERROR;
	}

	SQLSMALLINT NumParams;
	RetCode = SQLNumParams(HInsStmt, &NumParams);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error preparing an INSERT statement\n";
		SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
		throw INSERT_PREP_ERROR;
	}

	if( NumParams != num_values ){
		cerr << "Wrong number of output values in the INSERT statement\n";
		SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
		throw WRONG_NUM_PARAMS;
	}

	SQLSMALLINT i, DataType, DecimalDigits, Nullable;
	SQLULEN ParamSize;
	for(i=0; i < NumParams; i++)
	{
		RetCode = SQLDescribeParam(HInsStmt, i+1, &DataType, &ParamSize, &DecimalDigits, &Nullable);
		// assume here that all columns have integer type
    if (RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO) {
      SQLCHAR SQLState[SQLSTATE_CODE_LNGTH + 1];
//		  SQLCHAR SQLStateObjExists[] = "42S01";
		  SQLSMALLINT TextLength;
		  RetCode = SQLGetDiagField(SQL_HANDLE_STMT, HInsStmt, 1, SQL_DIAG_SQLSTATE, SQLState, SQLSTATE_CODE_LNGTH + 1, &TextLength);
      printf("Error %s\n",SQLState);
    }
		RetCode = SQLBindParameter(HInsStmt, i+1, SQL_PARAM_INPUT, SQL_C_SLONG, DataType, ParamSize, DecimalDigits, &Val[i], sizeof(int), NULL);
		if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
		{
			cerr << "Error binding a parameter in an SQL statement\n";
			throw PARAM_BIND_ERROR;
		}
	}
}


void DBOps::InsertRecord(int *InsVal)
{
	SQLSMALLINT i;
	for(i=0; i < num_values; i++)
		Val[i] = InsVal[i];

	SQLRETURN RetCode;
	RetCode = SQLExecute(HInsStmt);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error inserting a record in the database\n";
		SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
		throw INS_REC_ERROR;
	}
}


void DBOps::DeleteSQLInsertStmt()
{
	SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
}


void DBOps::CommitInsChanges()
{
	SQLRETURN RetCode;

	RetCode = SQLEndTran(SQL_HANDLE_DBC, HDbc, SQL_COMMIT);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error committing changes to a database\n";
		throw TRAN_COMMIT_ERROR;
	}

	//SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
}


void DBOps::RollbackInsChanges()
{
	SQLRETURN RetCode;

	RetCode = SQLEndTran(SQL_HANDLE_DBC, HDbc, SQL_ROLLBACK);
	if( RetCode != SQL_SUCCESS && RetCode != SQL_SUCCESS_WITH_INFO )
	{
		cerr << "Error rolling back changes from a database\n";
		throw TRAN_ROLLBACK_ERROR;
	}

	SQLFreeHandle(SQL_HANDLE_STMT, HInsStmt);
}

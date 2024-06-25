#include "odbc_test_common.h"
#include <iostream>

using namespace odbc_test;

TEST_CASE("Test Select Statement", "[odbc]") {
	SQLHANDLE env;
	SQLHANDLE dbc;

	HSTMT hstmt = SQL_NULL_HSTMT;

	// Connect to the database using SQLConnect
	CONNECT_TO_DATABASE(env, dbc);

	// Allocate a statement handle
	EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

	// Execute a simple query
	EXECUTE_AND_CHECK("SQLExecDirect (SELECT 1 UNION ALL SELECT 2)", SQLExecDirect, hstmt,
	                  ConvertToSQLCHAR("SELECT 1 UNION ALL SELECT 2"), SQL_NTS);

	// Fetch the first row
	EXECUTE_AND_CHECK("SQLFetch (SELECT 1 UNION ALL SELECT 2)", SQLFetch, hstmt);
	// Check the data
	DATA_CHECK(hstmt, 1, "1");

	// Fetch the second row
	EXECUTE_AND_CHECK("SQLFetch (SELECT 1 UNION ALL SELECT 2)", SQLFetch, hstmt);
	// Check the data
	DATA_CHECK(hstmt, 1, "2");

	EXECUTE_AND_CHECK("SQLFreeStmt (SQL_CLOSE)", SQLFreeStmt, hstmt, SQL_CLOSE);

	// Create a query with 1600 columns
	std::string query = "SELECT ";
	for (int i = 1; i < 1600; i++) {
		query += std::to_string(i);
		if (i < 1599) {
			query += ", ";
		}
	}

	EXECUTE_AND_CHECK("SQLExecDirect (SELECT 1600 columns)", SQLExecDirect, hstmt, ConvertToSQLCHAR(query.c_str()),
	                  SQL_NTS);

	// Fetch the first row
	EXECUTE_AND_CHECK("SQLFetch (SELECT 1600 columns)", SQLFetch, hstmt);

	// Check the data
	for (int i = 1; i < 1600; i++) {
		DATA_CHECK(hstmt, i, std::to_string(i));
	}

	// SELECT $x; should throw error
	SQLRETURN ret = SQLExecDirect(hstmt, ConvertToSQLCHAR("SELECT $x"), SQL_NTS);
	REQUIRE(ret == SQL_ERROR);
	std::string state;
	std::string message;
	ACCESS_DIAGNOSTIC(state, message, hstmt, SQL_HANDLE_STMT);
	REQUIRE(state == "42000");
	REQUIRE(duckdb::StringUtil::Contains(message, "Not all parameters are bound"));

	// Free the statement handle
	EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", SQLFreeStmt, hstmt, SQL_CLOSE);
	EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", SQLFreeHandle, SQL_HANDLE_STMT, hstmt);

	DISCONNECT_FROM_DATABASE(env, dbc);
}

static void PrintWCHAR(SQLWCHAR *content, SQLLEN content_len) {
    std::cout << "COL :";
    for (int i = 0; i < content_len; i++) {
        std::cout << (uint32_t)(((uint16_t*)content)[i]) << "\n";
    }
    std::cout << std::endl;
}

TEST_CASE("Test Select With UTF-16", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;

    HSTMT hstmt = SQL_NULL_HSTMT;

    // Connect to the database using SQLConnect
    DRIVER_CONNECT_TO_DATABASE(env, dbc, "database=/Users/maia/CLionProjects/duckdb-odbc/test.db");

    // Allocate a statement handle
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    // Execute a simple query with an UTF-16 string
    EXECUTE_AND_CHECK("SQLExecDirect (SELECT)", SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("from test_utf8"), SQL_NTS);

    // Check the data
    while (SQLFetch(hstmt) != SQL_NO_DATA) {
        // Check the data
        SQLWCHAR content[256];
        SQLLEN content_len;

        // SQLGetData returns data for a single column in the result set.
        for (idx_t i = 0; i < 6; i++) {
            SQLRETURN ret = SQLGetData(hstmt, i, SQL_C_WCHAR, &content, sizeof(content), &content_len);
            ODBC_CHECK(ret, "SQLGetData");

            PrintWCHAR(content, content_len);
        }
    }

    // Free the statement handle
    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", SQLFreeHandle, SQL_HANDLE_STMT, hstmt);

    DISCONNECT_FROM_DATABASE(env, dbc);
}
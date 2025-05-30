#include "odbc_test_common.h"

using namespace odbc_test;

static void RunDataCheckOnTable(HSTMT &hstmt, int num_rows) {
	for (int i = 1; i <= num_rows; i++) {
		EXECUTE_AND_CHECK("SQLFetch", hstmt, SQLFetch, hstmt);
		DATA_CHECK(hstmt, 1, std::to_string(i));
		DATA_CHECK(hstmt, 2, std::string("foo") + std::to_string(i));
	}
}

// Test Simple With Query
static void SimpleWithTest(HSTMT &hstmt) {
	EXECUTE_AND_CHECK("SQLExectDirect(WITH)", hstmt, SQLExecDirect, hstmt,
	                  ConvertToSQLCHAR("with recursive cte as (select g, 'foo' || g as foocol from "
	                                   "generate_series(1,10) as g(g)) select * from cte;"),
	                  SQL_NTS);

	RunDataCheckOnTable(hstmt, 10);

	EXECUTE_AND_CHECK("SQLFreeStmt(CLOSE)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
}

// Test With Query with Prepare and Execute
static void PreparedWithTest(HSTMT &hstmt) {
	EXECUTE_AND_CHECK("SQLPrepare(WITH)", hstmt, SQLPrepare, hstmt,
	                  ConvertToSQLCHAR("with cte as (select g, 'foo' || g as foocol from generate_series(1,10) as "
	                                   "g(g)) select * from cte WHERE g < ?"),
	                  SQL_NTS);

	SQLINTEGER param = 3;
	SQLLEN param_len = sizeof(param);
	EXECUTE_AND_CHECK("SQLBindParameter", hstmt, SQLBindParameter, hstmt, 1, SQL_PARAM_INPUT, SQL_INTEGER, SQL_INTEGER,
	                  0, 0, &param, sizeof(param), &param_len);

	EXECUTE_AND_CHECK("SQLExecute", hstmt, SQLExecute, hstmt);

	RunDataCheckOnTable(hstmt, 2);

	EXECUTE_AND_CHECK("SQLFreeStmt(CLOSE)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
}

/**
 * Runs two WITH queries and checks the results
 */
TEST_CASE("Test CTE", "[odbc]") {
	SQLHANDLE env;
	SQLHANDLE dbc;

	HSTMT hstmt = SQL_NULL_HSTMT;

	// Connect to the database using SQLDriverConnect
	CONNECT_TO_DATABASE(env, dbc);

	// Allocate a statement handle
	EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

	SimpleWithTest(hstmt);
	PreparedWithTest(hstmt);

	// Free the statement handle
	EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
	EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);

	DISCONNECT_FROM_DATABASE(env, dbc);
}

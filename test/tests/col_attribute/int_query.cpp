#include "utils.h"

using namespace odbc_col_attribute_test;

TEST_CASE("Test SQLColAttribute for a query that returns an int", "[odbc]") {
	SQLHANDLE env;
	SQLHANDLE dbc;

	HSTMT hstmt = SQL_NULL_HSTMT;

	// Connect to the database using SQLConnect
	CONNECT_TO_DATABASE(env, dbc);

	// Allocate a statement handle
	EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", hstmt, SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

	// run a simple query  with ints to get a result set
	EXECUTE_AND_CHECK("SQLExecDirect", hstmt, SQLExecDirect, hstmt, ConvertToSQLCHAR("SELECT 1 AS a, 2 AS b"), SQL_NTS);
	std::map<SQLLEN, ExpectedResult> expected_int;
	expected_int[SQL_DESC_CASE_SENSITIVE] = ExpectedResult(SQL_FALSE);
	expected_int[SQL_DESC_CATALOG_NAME] = ExpectedResult("system");
	expected_int[SQL_DESC_CONCISE_TYPE] = ExpectedResult(SQL_INTEGER);
	expected_int[SQL_DESC_COUNT] = ExpectedResult(2);
	expected_int[SQL_DESC_DISPLAY_SIZE] = ExpectedResult(11);
	expected_int[SQL_DESC_FIXED_PREC_SCALE] = ExpectedResult(SQL_FALSE);
	expected_int[SQL_DESC_LENGTH] = ExpectedResult(10);
	expected_int[SQL_DESC_LITERAL_PREFIX] = ExpectedResult("NULL");
	expected_int[SQL_DESC_LITERAL_SUFFIX] = ExpectedResult("NULL");
	expected_int[SQL_DESC_LOCAL_TYPE_NAME] = ExpectedResult("");
	expected_int[SQL_DESC_NULLABLE] = ExpectedResult(SQL_NULLABLE);
	expected_int[SQL_DESC_NUM_PREC_RADIX] = ExpectedResult(2);
	expected_int[SQL_DESC_PRECISION] = ExpectedResult(10);
	expected_int[SQL_COLUMN_SCALE] = ExpectedResult(0);
	expected_int[SQL_DESC_SCALE] = ExpectedResult(0);
	expected_int[SQL_DESC_SCHEMA_NAME] = ExpectedResult("");
	expected_int[SQL_DESC_SEARCHABLE] = ExpectedResult(SQL_PRED_BASIC);
	expected_int[SQL_DESC_TYPE] = ExpectedResult(SQL_INTEGER);
	expected_int[SQL_DESC_UNNAMED] = ExpectedResult(SQL_NAMED);
	expected_int[SQL_DESC_UNSIGNED] = ExpectedResult(SQL_FALSE);
	expected_int[SQL_DESC_UPDATABLE] = ExpectedResult(SQL_ATTR_READONLY);
	TestAllFields(hstmt, expected_int);

	// Free the statement handle
	EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", hstmt, SQLFreeStmt, hstmt, SQL_CLOSE);
	EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", hstmt, SQLFreeHandle, SQL_HANDLE_STMT, hstmt);

	// Disconnect from the database
	DISCONNECT_FROM_DATABASE(env, dbc);
}

#define CATCH_CONFIG_MAIN
#include "common.h"

namespace odbc_test {

void ODBC_CHECK(SQLRETURN ret, const char *msg) {
	switch (ret) {
	case SQL_SUCCESS:
		REQUIRE(1 == 1);
		return;
	case SQL_SUCCESS_WITH_INFO:
		fprintf(stderr, "%s: Error: Success with info\n", msg);
		break;
	case SQL_ERROR:
		fprintf(stderr, "%s: Error: Error\n", msg);
		break;
	case SQL_NO_DATA:
		fprintf(stderr, "%s: Error: no data\n", msg);
		break;
	case SQL_INVALID_HANDLE:
		fprintf(stderr, "%s: Error: invalid handle\n", msg);
		break;
	default:
		fprintf(stderr, "%s: Unexpected return value\n", msg);
		break;
	}
	REQUIRE(ret == SQL_SUCCESS);
}

void ACCESS_DIAGNOSTIC(string &state, string &message, SQLHANDLE handle, SQLSMALLINT handle_type) {
	SQLCHAR sqlstate[6];
	SQLINTEGER native_error;
	SQLCHAR message_text[256];
	SQLSMALLINT text_length;
	SQLSMALLINT recnum = 0;
	SQLRETURN ret = SQL_SUCCESS;

	while (SQL_SUCCEEDED(ret)) {
		recnum++;
		ret = SQLGetDiagRec(handle_type, handle, recnum, sqlstate, &native_error, message_text, sizeof(message_text),
		                    &text_length);
		if (SQL_SUCCEEDED(ret)) {
			state = ConvertToString(sqlstate);
			message = ConvertToString(message_text);
		}
	}

	if (ret != SQL_NO_DATA) {
		ODBC_CHECK(ret, "SQLGetDiagRec");
	}
}

void DATA_CHECK(HSTMT hstmt, SQLSMALLINT col_num, const char *expected_content) {
	SQLCHAR content[256];
	SQLLEN content_len;

	SQLRETURN ret = SQLGetData(hstmt, col_num, SQL_C_CHAR, content, sizeof(content), &content_len);
	ODBC_CHECK(ret, "SQLGetData");
	if (content_len == SQL_NULL_DATA) {
		REQUIRE(expected_content == nullptr);
		return;
	}
	REQUIRE(!::strcmp(ConvertToCString(content), expected_content));
}

void METADATA_CHECK(HSTMT hstmt, SQLUSMALLINT col_num, const string &expected_col_name,
                    SQLSMALLINT expected_col_name_len, SQLSMALLINT expected_col_data_type, SQLULEN expected_col_size,
                    SQLSMALLINT expected_col_decimal_digits, SQLSMALLINT expected_col_nullable) {
	SQLCHAR col_name[256];
	SQLSMALLINT col_name_len;
	SQLSMALLINT col_type;
	SQLULEN col_size;
	SQLSMALLINT col_decimal_digits;
	SQLSMALLINT col_nullable;

	SQLRETURN ret = SQLDescribeCol(hstmt, col_num, col_name, sizeof(col_name), &col_name_len, &col_type, &col_size,
	                               &col_decimal_digits, &col_nullable);
	ODBC_CHECK(ret, "SQLDescribeCol");

	if (!expected_col_name.empty()) {
		REQUIRE(expected_col_name.compare(ConvertToString(col_name)) == 0);
	}
	if (expected_col_name_len) {
		REQUIRE(col_name_len == expected_col_name_len);
	}
	if (expected_col_data_type) {
		REQUIRE(col_type == expected_col_data_type);
	}
	if (expected_col_size) {
		REQUIRE(col_size == expected_col_size);
	}
	if (expected_col_decimal_digits) {
		REQUIRE(col_decimal_digits == expected_col_decimal_digits);
	}
	if (expected_col_nullable) {
		REQUIRE(col_nullable == expected_col_nullable);
	}
}

void DRIVER_CONNECT_TO_DATABASE(SQLHANDLE &env, SQLHANDLE &dbc, const string &extra_params) {
	string dsn;
	string default_dsn = "duckdbmemory";
	SQLCHAR str[1024];
	SQLSMALLINT strl;
	auto tmp = getenv("COMMON_CONNECTION_STRING_FOR_REGRESSION_TEST");
	string envvar = tmp ? tmp : "";

	if (!envvar.empty()) {
		if (!extra_params.empty()) {
			dsn = "DSN=" + default_dsn + ";" + extra_params + ";" + envvar + ";" + extra_params;
		} else {
			dsn = "DSN=" + default_dsn + ";" + envvar;
		}
	} else {
		if (!extra_params.empty()) {
			dsn = "DSN=" + default_dsn + ";" + extra_params;
		} else {
			dsn = "DSN=" + default_dsn;
		}
	}

	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &env);
	REQUIRE(ret == SQL_SUCCESS);

	ExecuteCmdAndCheckODBC("SQLSetEnvAttr (SQL_ATTR_ODBC_VERSION ODBC3)", SQLSetEnvAttr, env, SQL_ATTR_ODBC_VERSION,
	                       ConvertToSQLPOINTER(SQL_OV_ODBC3), 0);

	ExecuteCmdAndCheckODBC("SQLAllocHandle (DBC)", SQLAllocHandle, SQL_HANDLE_DBC, env, &dbc);

	ExecuteCmdAndCheckODBC("SQLDriverConnect", SQLDriverConnect, dbc, nullptr, ConvertToSQLCHAR(dsn.c_str()), SQL_NTS,
	                       str, sizeof(str), &strl, SQL_DRIVER_COMPLETE);
}

void CONNECT_TO_DATABASE(SQLHANDLE &env, SQLHANDLE &dbc) {
	string dsn = "DuckDB";

	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, nullptr, &env);
	REQUIRE(ret == SQL_SUCCESS);

	ExecuteCmdAndCheckODBC("SQLSetEnvAttr (SQL_ATTR_ODBC_VERSION ODBC3)", SQLSetEnvAttr, env, SQL_ATTR_ODBC_VERSION,
	                       ConvertToSQLPOINTER(SQL_OV_ODBC3), 0);

	ExecuteCmdAndCheckODBC("SQLAllocHandle (DBC)", SQLAllocHandle, SQL_HANDLE_DBC, env, &dbc);

	ExecuteCmdAndCheckODBC("SQLConnect", SQLConnect, dbc, ConvertToSQLCHAR(dsn.c_str()), SQL_NTS, nullptr, 0, nullptr,
	                       0);
}

void DISCONNECT_FROM_DATABASE(SQLHANDLE &env, SQLHANDLE &dbc) {
	ExecuteCmdAndCheckODBC("SQLFreeHandle(SQL_HANDLE_ENV)", SQLFreeHandle, SQL_HANDLE_ENV, env);

	ExecuteCmdAndCheckODBC("SQLDisconnect", SQLDisconnect, dbc);

	ExecuteCmdAndCheckODBC("SQLFreeHandle(SQL_HANDLE_DBC)", SQLFreeHandle, SQL_HANDLE_DBC, dbc);
}

void EXEC_SQL(HSTMT hstmt, const string &query) {
	ExecuteCmdAndCheckODBC("SQLExecDirect", SQLExecDirect, hstmt, ConvertToSQLCHAR(query.c_str()), SQL_NTS);
}

void INITIALIZE_DATABASE(HSTMT hstmt) {
	EXEC_SQL(hstmt, "CREATE TABLE test_table_1 (id integer PRIMARY KEY, t varchar(20));");
	EXEC_SQL(hstmt, "INSERT INTO test_table_1 VALUES (1, 'foo');");
	EXEC_SQL(hstmt, "INSERT INTO test_table_1 VALUES (2, 'bar');");
	EXEC_SQL(hstmt, "INSERT INTO test_table_1 VALUES (3, 'foobar');");

	EXEC_SQL(hstmt, "CREATE TABLE bool_table (id integer, t varchar(5), b boolean);");
	EXEC_SQL(hstmt, "INSERT INTO bool_table VALUES (1, 'yeah', true);");
	EXEC_SQL(hstmt, "INSERT INTO bool_table VALUES (2, 'yes', true);");
	EXEC_SQL(hstmt, "INSERT INTO bool_table VALUES (3, 'true', true);");
	EXEC_SQL(hstmt, "INSERT INTO bool_table VALUES (4, 'false', false)");
	EXEC_SQL(hstmt, "INSERT INTO bool_table VALUES (5, 'not', false);");

	EXEC_SQL(hstmt, "CREATE TABLE byte_table (id integer, t blob);");
	EXEC_SQL(hstmt, "INSERT INTO byte_table VALUES (1, '\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x10'::blob);");
	EXEC_SQL(hstmt, "INSERT INTO byte_table VALUES (2, 'bar');");
	EXEC_SQL(hstmt, "INSERT INTO byte_table VALUES (3, 'foobar');");
	EXEC_SQL(hstmt, "INSERT INTO byte_table VALUES (4, 'foo');");
	EXEC_SQL(hstmt, "INSERT INTO byte_table VALUES (5, 'barf');");

	EXEC_SQL(hstmt, "CREATE TABLE interval_table(id integer, iv interval, d varchar(100));");
	EXEC_SQL(hstmt, "INSERT INTO interval_table VALUES (1, '1 day', 'one day');");
	EXEC_SQL(hstmt, "INSERT INTO interval_table VALUES (2, '10 seconds', 'ten secs');");
	EXEC_SQL(hstmt, "INSERT INTO interval_table VALUES (3, '100 years', 'hundred years');");

	EXEC_SQL(hstmt, "CREATE VIEW test_view AS SELECT * FROM test_table_1;");

	EXEC_SQL(hstmt, "CREATE TABLE lo_test_table (id int4, large_data blob);");
}

map<SQLSMALLINT, SQLULEN> InitializeTypesMap() {
	map<SQLSMALLINT, SQLULEN> types_map;

	types_map[SQL_VARCHAR] = 256;
	types_map[SQL_BIGINT] = 20;
	types_map[SQL_INTEGER] = 11;
	types_map[SQL_SMALLINT] = 5;
	return types_map;
}

SQLCHAR *ConvertToSQLCHAR(const char *str) {
	return reinterpret_cast<SQLCHAR *>(const_cast<char *>(str));
}

string ConvertToString(SQLCHAR *str) {
	return string(reinterpret_cast<char *>(str));
}

const char *ConvertToCString(SQLCHAR *str) {
	return reinterpret_cast<const char *>(str);
}

SQLPOINTER ConvertToSQLPOINTER(uint64_t ptr) {
	return reinterpret_cast<SQLPOINTER>(static_cast<uintptr_t>(ptr));
}

} // namespace odbc_test
#ifndef ODBC_DIAGNOSTIC_HPP
#define ODBC_DIAGNOSTIC_HPP

#include "duckdb_odbc.hpp"

#include <set>
#include <string>
#include <vector>
#include <unordered_map>

namespace duckdb {
struct DiagRecord {
public:
	// Some fields were commented out because they can be extract from other fields or internal data structures
	// std::string sql_diag_class_origin;
	SQLINTEGER sql_diag_column_number = SQL_NO_COLUMN_NUMBER;
	// std::string sql_diag_connection_name;
	std::string sql_diag_message_text;
	SQLINTEGER sql_diag_native = 0;
	SQLLEN sql_diag_row_number = SQL_NO_ROW_NUMBER;
	std::string sql_diag_server_name;
	std::string sql_diag_sqlstate;
	// std::string sql_diag_subclass_origin;
};

struct DiagHeader {
public:
	SQLLEN sql_diag_cursor_row_count;
	// std::string sql_diag_dynamic_function; // this field is extract from map_dynamic_function
	SQLINTEGER sql_diag_dynamic_function_code;
	SQLINTEGER sql_diag_number;
	SQLRETURN sql_diag_return_code;
	SQLLEN sql_diag_row_count;
};

class OdbcDiagnostic {
public:
	DiagHeader header;
	std::vector<DiagRecord> diag_records;
	static const std::unordered_map<SQLINTEGER, std::string> map_dynamic_function;
	static const std::set<std::string> set_odbc3_subclass_origin;

public:
	static bool IsDiagRecordField(SQLSMALLINT diag_identifier);

	std::string GetDiagDynamicFunction();
	bool VerifyRecordIndex(SQLINTEGER rec_idx);
	const DiagRecord &GetDiagRecord(SQLINTEGER rec_idx);
	std::string GetDiagClassOrigin(SQLINTEGER rec_idx);
	std::string GetDiagSubclassOrigin(SQLINTEGER rec_idx);
	// WriteLog
};
} // namespace duckdb
#endif
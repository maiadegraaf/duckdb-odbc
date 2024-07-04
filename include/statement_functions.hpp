#ifndef STATEMENT_FUNCTIONS_HPP
#define STATEMENT_FUNCTIONS_HPP

#pragma once

#include "duckdb_odbc.hpp"

namespace duckdb {

string GetQueryAsString(OdbcHandleStmt *hstmt, SQLCHAR *statement_text, int32_t text_length);
SQLRETURN FinalizeStmt(OdbcHandleStmt *hstmt);

SQLRETURN BatchExecuteStmt(OdbcHandleStmt *hstmt);
SQLRETURN SingleExecuteStmt(OdbcHandleStmt *hstmt);

SQLRETURN FetchStmtResult(OdbcHandleStmt *hstmt, int16_t fetch_orientation = SQL_FETCH_NEXT,
                          int64_t fetch_offset = 0);

SQLRETURN GetDataStmtResult(OdbcHandleStmt *hstmt, u_int16_t col_or_param_num, int16_t target_type,
                            void *target_value_ptr, intmax_t buffer_length, intmax_t *str_len_or_ind_ptr);

SQLRETURN ExecDirectStmt(OdbcHandleStmt *hstmt, const string& query);

SQLRETURN BindParameterStmt(SQLHSTMT statement_handle, SQLUSMALLINT parameter_number, SQLSMALLINT input_output_type,
                            SQLSMALLINT value_type, SQLSMALLINT parameter_type, SQLULEN column_size,
                            SQLSMALLINT decimal_digits, SQLPOINTER parameter_value_ptr, SQLLEN buffer_length,
                            SQLLEN *str_len_or_ind_ptr);

SQLRETURN CloseStmt(OdbcHandleStmt *hstmt);

} // namespace duckdb
#endif

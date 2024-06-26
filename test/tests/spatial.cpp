#include "odbc_test_common.h"

#include <iostream>

using namespace odbc_test;

TEST_CASE("Test Spatial with Binding", "[odbc]") {
    SQLHANDLE env;
    SQLHANDLE dbc;

    HSTMT hstmt = SQL_NULL_HSTMT;

    // Connect to the database using SQLConnect
    CONNECT_TO_DATABASE(env, dbc);

    // Allocate a statement handle
    EXECUTE_AND_CHECK("SQLAllocHandle (HSTMT)", SQLAllocHandle, SQL_HANDLE_STMT, dbc, &hstmt);

    // Install the Spatial Extension
    EXECUTE_AND_CHECK("SQLExecDirect (INSTALL SPATIAL)", SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("INSTALL SPATIAL"), SQL_NTS);
    EXECUTE_AND_CHECK("SQLExecDirect (LOAD SPATIAL)", SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("LOAD SPATIAL"), SQL_NTS);

    // Create a table with spatial data
    EXECUTE_AND_CHECK("SQLExecDirect (CREATE TABLE)", SQLExecDirect, hstmt,
                      ConvertToSQLCHAR("CREATE TABLE d(OBJECTID INT64 NOT NULL, TAG STRING, SHAPE GEOMETRY)"), SQL_NTS);

    // Prepare the insert statement
    auto ret = SQLPrepare(hstmt, ConvertToSQLCHAR("INSERT INTO d ( OBJECTID , TAG , SHAPE ) VALUES ( ? , ? , ST_GEOMFROMTEXT(?))"), SQL_NTS);
    if (ret != SQL_SUCCESS) {
        std::cout << "SQLPrepare failed" << std::endl;
        std::string state;
        std::string message;
        ACCESS_DIAGNOSTIC(state, message, hstmt, SQL_HANDLE_STMT);

        std::cout << "State: " << state << std::endl;
        std::cout << "Message: " << message << std::endl;
        return;
    }

    // Bind the first row of parameters
    SQLINTEGER    c1;
    SQLCHAR       *c2;
    SQLCHAR       *c3;

    c1 = 1;
    c2 = ConvertToSQLCHAR("first row");
    c3 = ConvertToSQLCHAR("POINT (1.1 1.1)");

    EXECUTE_AND_CHECK("SQLBindParameter (Bind OBJECTID)", SQLBindParameter, hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &c1, 0, nullptr);

    EXECUTE_AND_CHECK("SQLBindParameter (Bind TAG)", SQLBindParameter, hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, c2, 0, nullptr);

    EXECUTE_AND_CHECK("SQLBindParameter (Bind SHAPE)", SQLBindParameter, hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, c3, 0, nullptr);

    // Execute the insert statement
    EXECUTE_AND_CHECK("SQLExecute (INSERT INTO)", SQLExecute, hstmt);

//    // Bind the second row of parameters
//    c1 = 2;
//    c2 = ConvertToSQLCHAR("second row");
//    c3 = ConvertToSQLCHAR("POINT (1.2 1.2)");

    // Free the statement handle
    EXECUTE_AND_CHECK("SQLFreeStmt (HSTMT)", SQLFreeStmt, hstmt, SQL_CLOSE);
    EXECUTE_AND_CHECK("SQLFreeHandle (HSTMT)", SQLFreeHandle, SQL_HANDLE_STMT, hstmt);

    DISCONNECT_FROM_DATABASE(env, dbc);
}

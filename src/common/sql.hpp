// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SQL_HPP
#define SQL_HPP

#include <cstdarg>// va_list
#include <stdexcept>

#ifdef WIN32
#include "winapi.hpp"
#endif

#include <mysql.h>

#include "cbasetypes.hpp"
#include "strlib.hpp"

// Return codes
#define SQL_ERROR -1
#define SQL_SUCCESS 0
#define SQL_NO_DATA 100

// macro definition to determine whether the mySQL engine is running on InnoDB (rather than MyISAM)
// uncomment this line if the your mySQL tables have been changed to run on InnoDB
// this macro will adjust how logs are recorded in the database to accommodate the change
//#define SQL_INNODB

/// Data type identifier.
/// String, enum and blob data types need the buffer length specified.
enum SqlDataType
{
	SQLDT_NULL,
	// fixed size
	SQLDT_INT8,
	SQLDT_INT16,
	SQLDT_INT32,
	SQLDT_INT64,
	SQLDT_UINT8,
	SQLDT_UINT16,
	SQLDT_UINT32,
	SQLDT_UINT64,
	// platform dependent size
	SQLDT_CHAR,
	SQLDT_LONG,
	SQLDT_LONGLONG,
	SQLDT_UCHAR,
	SQLDT_ULONG,
	SQLDT_ULONGLONG,
	// floating point
	SQLDT_FLOAT,
	SQLDT_DOUBLE,
	// other
	SQLDT_STRING,
	SQLDT_ENUM,
	// Note: An ENUM is a string with restricted values. When an invalid value
	//       is inserted, it is saved as an empty string (numerical value 0).
	SQLDT_BLOB,
	SQLDT_LASTID
};

struct Sql;// Sql handle (private access)

typedef enum SqlDataType SqlDataType;
typedef struct Sql Sql;


/// Allocates and initializes a new Sql handle.
struct Sql* Sql_Malloc(void);



/// Retrieves the last error number.
uint32 Sql_GetError( Sql* self );



/// Establishes a connection.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_Connect(Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db);




/// Retrieves the timeout of the connection.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_GetTimeout(Sql* self, uint32* out_timeout);




/// Retrieves the name of the columns of a table into out_buf, with the separator after each name.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_GetColumnNames(Sql* self, const char* table, char* out_buf, size_t buf_len, char sep);




/// Changes the encoding of the connection.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_SetEncoding(Sql* self, const char* encoding);



/// Pings the connection.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_Ping(Sql* self);



/// Escapes a string.
/// The output buffer must be at least strlen(from)*2+1 in size.
///
/// @return The size of the escaped string
size_t Sql_EscapeString(Sql* self, char* out_to, const char* from);



/// Escapes a string.
/// The output buffer must be at least from_len*2+1 in size.
///
/// @return The size of the escaped string
size_t Sql_EscapeStringLen(Sql* self, char* out_to, const char* from, size_t from_len);



/// Executes a query.
/// Any previous result is freed.
/// The query is constructed as if it was sprintf.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_Query(Sql* self, const char* query, ...);



/// Executes a query.
/// Any previous result is freed.
/// The query is constructed as if it was svprintf.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_QueryV(Sql* self, const char* query, va_list args);



/// Executes a query.
/// Any previous result is freed.
/// The query is used directly.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_QueryStr(Sql* self, const char* query);



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
///
/// @return Value of the auto-increment column
uint64 Sql_LastInsertId(Sql* self);



/// Returns the number of columns in each row of the result.
///
/// @return Number of columns
uint32 Sql_NumColumns(Sql* self);



/// Returns the number of rows in the result.
///
/// @return Number of rows
uint64 Sql_NumRows(Sql* self);



/// Returns the number of rows affected by the last query
///
/// @return Number of rows
uint64 Sql_NumRowsAffected(Sql* self);



/// Fetches the next row.
/// The data of the previous row is no longer valid.
///
/// @return SQL_SUCCESS, SQL_ERROR or SQL_NO_DATA
int32 Sql_NextRow(Sql* self);



/// Gets the data of a column.
/// The data remains valid until the next row is fetched or the result is freed.
///
/// @return SQL_SUCCESS or SQL_ERROR
int32 Sql_GetData(Sql* self, size_t col, char** out_buf, size_t* out_len);



/// Frees the result of the query.
void Sql_FreeResult(Sql* self);



#if defined(SQL_REMOVE_SHOWDEBUG)
#define Sql_ShowDebug(self) (void)0
#else
#define Sql_ShowDebug(self) Sql_ShowDebug_(self, __FILE__, __LINE__)
#endif
/// Shows debug information (last query).
void Sql_ShowDebug_(Sql* self, const char* debug_file, const unsigned long debug_line);



/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(Sql* self);

// Column length receiver.
// Takes care of the possible size missmatch between uint32 and unsigned long.
struct s_column_length
{
	uint32* out_length;
	unsigned long length;
};
typedef struct s_column_length s_column_length;


///////////////////////////////////////////////////////////////////////////////
// Prepared Statements
///////////////////////////////////////////////////////////////////////////////
// Parameters are placed in the statement by embedding question mark ('?') 
// characters into the query at the appropriate positions.
// The markers are legal only in places where they represent data.
// The markers cannot be inside quotes. Quotes will be added automatically 
// when they are required.
//
// example queries with parameters:
// 1) SELECT col FROM table WHERE id=?
// 2) INSERT INTO table(col1,col2) VALUES(?,?)
class SqlStmt{
private:
	StringBuf buf;
	MYSQL_STMT* stmt;
	MYSQL_BIND* params;
	MYSQL_BIND* columns;
	s_column_length* column_lengths;
	size_t max_params;
	size_t max_columns;
	bool bind_params;
	bool bind_columns;

	void ShowDebugTruncatedColumn( size_t i );

public:
	explicit SqlStmt( Sql& sql ) noexcept(false);
	~SqlStmt();

	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is constructed as if it was sprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 Prepare( const char* query, ... );

	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is constructed as if it was svprintf.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 PrepareV( const char* query, va_list args );

	/// Prepares the statement.
	/// Any previous result is freed and all parameter bindings are removed.
	/// The query is used directly.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 PrepareStr( const char* query );

	/// Returns the number of parameters in the prepared statement.
	///
	/// @return Number or paramenters
	size_t NumParams();

	/// Binds a parameter to a buffer.
	/// The buffer data will be used when the statement is executed.
	/// All parameters should have bindings.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 BindParam( size_t idx, SqlDataType buffer_type, void* buffer, size_t buffer_len );

	/// Executes the prepared statement.
	/// Any previous result is freed and all column bindings are removed.
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 Execute();

	/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
	///
	/// @return Value of the auto-increment column
	uint64 LastInsertId();

	/// Returns the number of columns in each row of the result.
	///
	/// @return Number of columns
	size_t NumColumns();

	/// Binds the result of a column to a buffer.
	/// The buffer will be filled with data when the next row is fetched.
	/// For string/enum buffer types there has to be enough space for the data 
	/// and the nul-terminator (an extra byte).
	///
	/// @return SQL_SUCCESS or SQL_ERROR
	int32 BindColumn( size_t idx, SqlDataType buffer_type, void* buffer, size_t buffer_len = 0, uint32* out_length = nullptr, int8* out_is_null = nullptr );

	/// Returns the number of rows in the result.
	///
	/// @return Number of rows
	uint64 NumRows();

	/// Fetches the next row.
	/// All column bindings will be filled with data.
	///
	/// @return SQL_SUCCESS, SQL_ERROR or SQL_NO_DATA
	int32 NextRow();

	/// Frees the result of the statement execution.
	void FreeResult();

	void ShowDebug_( const char* file, const unsigned long line );
};

#if defined(SQL_REMOVE_SHOWDEBUG)
#define SqlStmt_ShowDebug(self) (void)0
#else
// TODO: we have to keep this until C++20 and std::source_location is available [Lemongrass]
#define SqlStmt_ShowDebug(self) (self).ShowDebug_( __FILE__, __LINE__ )
#endif

void Sql_Init(void);

#endif /* SQL_HPP */

// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sql.hpp"

#include <cstdlib>// strtoul

#include "cbasetypes.hpp"
#include "cli.hpp"
#include "malloc.hpp"
#include "showmsg.hpp"
#include "timer.hpp"

// MySQL 8.0 or later removed my_bool typedef.
// Reintroduce it as a bandaid fix.
// See https://bugs.mysql.com/?id=87337
#if !defined(MARIADB_BASE_VERSION) && !defined(MARIADB_VERSION_ID) && MYSQL_VERSION_ID >= 80001 && MYSQL_VERSION_ID != 80002
#define my_bool bool
#endif

void ra_mysql_error_handler(uint32 ecode);

int32 mysql_reconnect_type;
uint32 mysql_reconnect_count;

/// Sql handle
struct Sql
{
	StringBuf buf;
	MYSQL handle;
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long* lengths;
	int32 keepalive;
};

///////////////////////////////////////////////////////////////////////////////
// Sql Handle
///////////////////////////////////////////////////////////////////////////////



/// Allocates and initializes a new Sql handle.
Sql* Sql_Malloc(void)
{
	Sql* self;

	CREATE(self, Sql, 1);
	mysql_init(&self->handle);
	StringBuf_Init(&self->buf);
	self->lengths = nullptr;
	self->result = nullptr;
	self->keepalive = INVALID_TIMER;
	my_bool reconnect = 1;
	mysql_options(&self->handle, MYSQL_OPT_RECONNECT, &reconnect);
	return self;
}


/**
 * Retrieves the last error number.
 * @param self : sql handle
 * @return last error number
 */
uint32 Sql_GetError( Sql* self ){
	return mysql_errno( &self->handle );
}

static int32 Sql_P_Keepalive(Sql* self);

/**
 * Establishes a connection to schema
 * @param self : sql handle
 * @param user : username to access
 * @param passwd : password
 * @param host : hostname
 * @param port : port
 * @param db : schema name
 * @return 
 */
int32 Sql_Connect(Sql* self, const char* user, const char* passwd, const char* host, uint16 port, const char* db)
{
	if( self == nullptr )
		return SQL_ERROR;

	StringBuf_Clear(&self->buf);

#if !defined(MARIADB_BASE_VERSION) && !defined(MARIADB_VERSION_ID) && MYSQL_VERSION_ID >= 50710
	uint32 md = SSL_MODE_DISABLED;

	if( mysql_options( &self->handle, MYSQL_OPT_SSL_MODE, &md ) ){
		ShowSQL( "Your MySQL version does not understand \"MYSQL_OPT_SSL_MODE\" yet. Please consider upgrading - especially if you encounter SSL related error messages from your MySQL server.\n" );
	}
#endif

	if( !mysql_real_connect(&self->handle, host, user, passwd, db, (uint32)port, nullptr/*unix_socket*/, 0/*clientflag*/) )
	{
		ShowSQL("%s\n", mysql_error(&self->handle));
		return SQL_ERROR;
	}

	self->keepalive = Sql_P_Keepalive(self);
	if( self->keepalive == INVALID_TIMER )
	{
		ShowSQL("Failed to establish keepalive for DB connection!\n");
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}



/// Retrieves the timeout of the connection.
int32 Sql_GetTimeout(Sql* self, uint32* out_timeout)
{
	if( self && out_timeout && SQL_SUCCESS == Sql_Query(self, "SHOW VARIABLES LIKE 'wait_timeout'") )
	{
		char* data;
		size_t len;
		if( SQL_SUCCESS == Sql_NextRow(self) &&
			SQL_SUCCESS == Sql_GetData(self, 1, &data, &len) )
		{
			*out_timeout = (uint32)strtoul(data, nullptr, 10);
			Sql_FreeResult(self);
			return SQL_SUCCESS;
		}
		Sql_FreeResult(self);
	}
	return SQL_ERROR;
}



/// Retrieves the name of the columns of a table into out_buf, with the separator after each name.
int32 Sql_GetColumnNames(Sql* self, const char* table, char* out_buf, size_t buf_len, char sep)
{
	char* data;
	size_t len;
	size_t off = 0;

	if( self == nullptr || SQL_ERROR == Sql_Query(self, "EXPLAIN `%s`", table) )
		return SQL_ERROR;

	out_buf[off] = '\0';
	while( SQL_SUCCESS == Sql_NextRow(self) && SQL_SUCCESS == Sql_GetData(self, 0, &data, &len) )
	{
		len = strnlen(data, len);
		if( off + len + 2 > buf_len )
		{
			ShowDebug("Sql_GetColumns: output buffer is too small\n");
			*out_buf = '\0';
			return SQL_ERROR;
		}
		memcpy(out_buf+off, data, len);
		off += len;
		out_buf[off++] = sep;
	}
	out_buf[off] = '\0';
	Sql_FreeResult(self);
	return SQL_SUCCESS;
}



/// Changes the encoding of the connection.
int32 Sql_SetEncoding(Sql* self, const char* encoding)
{
	if( self && Sql_Query(self, "SET NAMES %s", encoding) == 0 )
		return SQL_SUCCESS;
	return SQL_ERROR;
}



/// Pings the connection.
int32 Sql_Ping(Sql* self)
{
	if( self && mysql_ping(&self->handle) == 0 )
		return SQL_SUCCESS;
	return SQL_ERROR;
}



/// Wrapper function for Sql_Ping.
///
/// @private
static TIMER_FUNC(Sql_P_KeepaliveTimer){
	Sql* self = (Sql*)data;
	ShowInfo("Pinging SQL server to keep connection alive...\n");
	Sql_Ping(self);
	return 0;
}



/// Establishes keepalive (periodic ping) on the connection.
///
/// @return the keepalive timer id, or INVALID_TIMER
/// @private
static int32 Sql_P_Keepalive(Sql* self)
{
	uint32 timeout, ping_interval;

	// set a default value first
	timeout = 28800; // 8 hours

	// request the timeout value from the mysql server
	Sql_GetTimeout(self, &timeout);

	if( timeout < 60 )
		timeout = 60;

	// establish keepalive
	ping_interval = timeout - 30; // 30-second reserve
	//add_timer_func_list(Sql_P_KeepaliveTimer, "Sql_P_KeepaliveTimer");
	return add_timer_interval(gettick() + ping_interval*1000, Sql_P_KeepaliveTimer, 0, (intptr_t)self, ping_interval*1000);
}



/// Escapes a string.
size_t Sql_EscapeString(Sql* self, char *out_to, const char *from)
{
	if( self )
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)strlen(from));
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)strlen(from));
}



/// Escapes a string.
size_t Sql_EscapeStringLen(Sql* self, char *out_to, const char *from, size_t from_len)
{
	if( self )
		return (size_t)mysql_real_escape_string(&self->handle, out_to, from, (unsigned long)from_len);
	else
		return (size_t)mysql_escape_string(out_to, from, (unsigned long)from_len);
}



/// Executes a query.
int32 Sql_Query(Sql* self, const char* query, ...)
{
	int32 res;
	va_list args;

	va_start(args, query);
	res = Sql_QueryV(self, query, args);
	va_end(args);

	return res;
}



/// Executes a query.
int32 Sql_QueryV(Sql* self, const char* query, va_list args)
{
	if( self == nullptr )
		return SQL_ERROR;

	Sql_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_Vprintf(&self->buf, query, args);
	if( mysql_real_query(&self->handle, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		ra_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	self->result = mysql_store_result(&self->handle);
	if( mysql_errno(&self->handle) != 0 )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		ra_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Executes a query.
int32 Sql_QueryStr(Sql* self, const char* query)
{
	if( self == nullptr )
		return SQL_ERROR;

	Sql_FreeResult(self);
	StringBuf_Clear(&self->buf);
	StringBuf_AppendStr(&self->buf, query);
	if( mysql_real_query(&self->handle, StringBuf_Value(&self->buf), (unsigned long)StringBuf_Length(&self->buf)) )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		ra_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	self->result = mysql_store_result(&self->handle);
	if( mysql_errno(&self->handle) != 0 )
	{
		ShowSQL("DB error - %s\n", mysql_error(&self->handle));
		ra_mysql_error_handler(mysql_errno(&self->handle));
		return SQL_ERROR;
	}
	return SQL_SUCCESS;
}



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE query.
uint64 Sql_LastInsertId(Sql* self)
{
	if( self )
		return (uint64)mysql_insert_id(&self->handle);
	else
		return 0;
}



/// Returns the number of columns in each row of the result.
uint32 Sql_NumColumns(Sql* self)
{
	if( self && self->result )
		return (uint32)mysql_num_fields(self->result);
	return 0;
}



/// Returns the number of rows in the result.
uint64 Sql_NumRows(Sql* self)
{
	if( self && self->result )
		return (uint64)mysql_num_rows(self->result);
	return 0;
}



/// Returns the number of rows affected by the last query
uint64 Sql_NumRowsAffected(Sql* self)
{
	if( self )
		return (uint64)mysql_affected_rows(&self->handle);
	return 0;
}



/// Fetches the next row.
int32 Sql_NextRow(Sql* self)
{
	if( self && self->result )
	{
		self->row = mysql_fetch_row(self->result);
		if( self->row )
		{
			self->lengths = mysql_fetch_lengths(self->result);
			return SQL_SUCCESS;
		}
		self->lengths = nullptr;
		if( mysql_errno(&self->handle) == 0 )
			return SQL_NO_DATA;
	}
	return SQL_ERROR;
}



/// Gets the data of a column.
int32 Sql_GetData(Sql* self, size_t col, char** out_buf, size_t* out_len)
{
	if( self && self->row )
	{
		if( col < Sql_NumColumns(self) )
		{
			if( out_buf ) *out_buf = self->row[col];
			if( out_len ) *out_len = (size_t)self->lengths[col];
		}
		else
		{// out of range - ignore
			if( out_buf ) *out_buf = nullptr;
			if( out_len ) *out_len = 0;
		}
		return SQL_SUCCESS;
	}
	return SQL_ERROR;
}



/// Frees the result of the query.
void Sql_FreeResult(Sql* self)
{
	if( self && self->result )
	{
		mysql_free_result(self->result);
		self->result = nullptr;
		self->row = nullptr;
		self->lengths = nullptr;
	}
}

/// Closes the handle
void Sql_Close(Sql* self) {
	if (self) {
		mysql_close(&self->handle);
	}
}


/// Shows debug information (last query).
void Sql_ShowDebug_(Sql* self, const char* debug_file, const unsigned long debug_line)
{
	if( self == nullptr )
		ShowDebug("at %s:%lu - self is nullptr\n", debug_file, debug_line);
	else if( StringBuf_Length(&self->buf) > 0 )
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&self->buf));
	else
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
}



/// Frees a Sql handle returned by Sql_Malloc.
void Sql_Free(Sql* self)
{
	if( self )
	{
		Sql_FreeResult(self);
		StringBuf_Destroy(&self->buf);
		if( self->keepalive != INVALID_TIMER ) delete_timer(self->keepalive, Sql_P_KeepaliveTimer);
		Sql_Close(self);
		aFree(self);
	}
}



///////////////////////////////////////////////////////////////////////////////
// Prepared Statements
///////////////////////////////////////////////////////////////////////////////



/// Returns the mysql integer type for the target size.
///
/// @private
static enum enum_field_types Sql_P_SizeToMysqlIntType(int32 sz)
{
	switch( sz )
	{
	case 1: return MYSQL_TYPE_TINY;
	case 2: return MYSQL_TYPE_SHORT;
	case 4: return MYSQL_TYPE_LONG;
	case 8: return MYSQL_TYPE_LONGLONG;
	default:
		ShowDebug("SizeToMysqlIntType: unsupported size (%d)\n", sz);
		return MYSQL_TYPE_NULL;
	}
}



/// Binds a parameter/result.
///
/// @private
static int32 Sql_P_BindSqlDataType(MYSQL_BIND* bind, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, unsigned long* out_length, int8* out_is_null)
{
	memset(bind, 0, sizeof(MYSQL_BIND));
	switch( buffer_type )
	{
	case SQLDT_NULL:
		bind->buffer_type = MYSQL_TYPE_NULL;
		buffer_len = 0;// FIXME length = ? [FlavioJS]
		break;
	// fixed size
	case SQLDT_UINT8:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_INT8:
		bind->buffer_type = MYSQL_TYPE_TINY;
		buffer_len = 1;
		break;
	case SQLDT_UINT16:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_INT16:
		bind->buffer_type = MYSQL_TYPE_SHORT;
		buffer_len = 2;
		break;
	case SQLDT_UINT32:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_INT32:
		bind->buffer_type = MYSQL_TYPE_LONG;
		buffer_len = 4;
		break;
	case SQLDT_UINT64:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_INT64:
		bind->buffer_type = MYSQL_TYPE_LONGLONG;
		buffer_len = 8;
		break;
	// platform dependent size
	case SQLDT_UCHAR:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_CHAR:
		bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(char));
		buffer_len = sizeof(char);
		break;
	case SQLDT_ULONG:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_LONG:
		bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(long));
		buffer_len = sizeof(long);
		break;
	case SQLDT_ULONGLONG:
		bind->is_unsigned = 1;
		[[fallthrough]];
	case SQLDT_LONGLONG:
		bind->buffer_type = Sql_P_SizeToMysqlIntType(sizeof(int64));
		buffer_len = sizeof(int64);
		break;
	// floating point
	case SQLDT_FLOAT:
		bind->buffer_type = MYSQL_TYPE_FLOAT;
		buffer_len = 4;
		break;
	case SQLDT_DOUBLE:
		bind->buffer_type = MYSQL_TYPE_DOUBLE;
		buffer_len = 8;
		break;
	// other
	case SQLDT_STRING:
	case SQLDT_ENUM:
		bind->buffer_type = MYSQL_TYPE_STRING;
		break;
	case SQLDT_BLOB:
		bind->buffer_type = MYSQL_TYPE_BLOB;
		break;
	default:
		ShowDebug("Sql_P_BindSqlDataType: unsupported buffer type (%d)\n", buffer_type);
		return SQL_ERROR;
	}
	bind->buffer = buffer;
	bind->buffer_length = (unsigned long)buffer_len;
	bind->length = out_length;
	bind->is_null = (my_bool*)out_is_null;
	return SQL_SUCCESS;
}



/// Prints debug information about a field (type and length).
///
/// @private
static void Sql_P_ShowDebugMysqlFieldInfo(const char* prefix, enum enum_field_types type, int32 is_unsigned, unsigned long length, const char* length_postfix)
{
	const char* sign = (is_unsigned ? "UNSIGNED " : "");
	const char* type_string;
	switch( type )
	{
	default:
		ShowDebug("%stype=%s%u, length=%lu\n", prefix, sign, type, length);
		return;
#define SHOW_DEBUG_OF(x) case x: type_string = #x; break
	SHOW_DEBUG_OF(MYSQL_TYPE_TINY);
	SHOW_DEBUG_OF(MYSQL_TYPE_SHORT);
	SHOW_DEBUG_OF(MYSQL_TYPE_LONG);
	SHOW_DEBUG_OF(MYSQL_TYPE_INT24);
	SHOW_DEBUG_OF(MYSQL_TYPE_LONGLONG);
	SHOW_DEBUG_OF(MYSQL_TYPE_DECIMAL);
	SHOW_DEBUG_OF(MYSQL_TYPE_FLOAT);
	SHOW_DEBUG_OF(MYSQL_TYPE_DOUBLE);
	SHOW_DEBUG_OF(MYSQL_TYPE_TIMESTAMP);
	SHOW_DEBUG_OF(MYSQL_TYPE_DATE);
	SHOW_DEBUG_OF(MYSQL_TYPE_TIME);
	SHOW_DEBUG_OF(MYSQL_TYPE_DATETIME);
	SHOW_DEBUG_OF(MYSQL_TYPE_YEAR);
	SHOW_DEBUG_OF(MYSQL_TYPE_STRING);
	SHOW_DEBUG_OF(MYSQL_TYPE_VAR_STRING);
	SHOW_DEBUG_OF(MYSQL_TYPE_BLOB);
	SHOW_DEBUG_OF(MYSQL_TYPE_SET);
	SHOW_DEBUG_OF(MYSQL_TYPE_ENUM);
	SHOW_DEBUG_OF(MYSQL_TYPE_NULL);
#undef SHOW_DEBUG_TYPE_OF
	}
	ShowDebug("%stype=%s%s, length=%lu%s\n", prefix, sign, type_string, length, length_postfix);
}



/// Reports debug information about a truncated column.
///
/// @private
void SqlStmt::ShowDebugTruncatedColumn( size_t i ){
	MYSQL_RES* meta;
	MYSQL_FIELD* field;
	MYSQL_BIND* column;

	meta = mysql_stmt_result_metadata(this->stmt);
	field = mysql_fetch_field_direct(meta, (uint32)i);
	ShowSQL("DB error - data of field '%s' was truncated.\n", field->name);
	ShowDebug("column - %lu\n", (unsigned long)i);
	Sql_P_ShowDebugMysqlFieldInfo("data   - ", field->type, field->flags&UNSIGNED_FLAG, this->column_lengths[i].length, "");
	column = &this->columns[i];
	if( column->buffer_type == MYSQL_TYPE_STRING )
		Sql_P_ShowDebugMysqlFieldInfo("buffer - ", column->buffer_type, column->is_unsigned, column->buffer_length, "+1(nul-terminator)");
	else
		Sql_P_ShowDebugMysqlFieldInfo("buffer - ", column->buffer_type, column->is_unsigned, column->buffer_length, "");
	mysql_free_result(meta);
}



/// Allocates and initializes a new SqlStmt handle.
SqlStmt::SqlStmt( Sql& sql ){
	this->stmt = mysql_stmt_init( &sql.handle );

	if( this->stmt == nullptr ){
		ShowSQL( "DB error - %s\n", mysql_error( &sql.handle ) );
		throw std::runtime_error( "Out of memory" );
	}

	StringBuf_Init( &this->buf );
	this->params = nullptr;
	this->columns = nullptr;
	this->column_lengths = nullptr;
	this->max_params = 0;
	this->max_columns = 0;
	this->bind_params = false;
	this->bind_columns = false;
}



/// Prepares the statement.
int32 SqlStmt::Prepare(const char* query, ...){
	int32 res;
	va_list args;

	va_start(args, query);
	res = this->PrepareV(query, args);
	va_end(args);

	return res;
}



/// Prepares the statement.
int32 SqlStmt::PrepareV(const char* query, va_list args){
	this->FreeResult();
	StringBuf_Clear( &this->buf );
	StringBuf_Vprintf( &this->buf, query, args );

	if( mysql_stmt_prepare( this->stmt, StringBuf_Value( &this->buf ), (unsigned long)StringBuf_Length( &this->buf ) ) ){
		ShowSQL( "DB error - %s\n", mysql_stmt_error( this->stmt ) );
		ra_mysql_error_handler( mysql_stmt_errno( this->stmt ) );
		return SQL_ERROR;
	}

	this->bind_params = false;

	return SQL_SUCCESS;
}



/// Prepares the statement.
int32 SqlStmt::PrepareStr(const char* query){
	this->FreeResult();
	StringBuf_Clear( &this->buf );
	StringBuf_AppendStr( &this->buf, query );

	if( mysql_stmt_prepare( this->stmt, StringBuf_Value( &this->buf ), (unsigned long)StringBuf_Length( &this->buf ) ) ){
		ShowSQL( "DB error - %s\n", mysql_stmt_error( this->stmt ) );
		ra_mysql_error_handler( mysql_stmt_errno( this->stmt ) );
		return SQL_ERROR;
	}

	this->bind_params = false;

	return SQL_SUCCESS;
}



/// Returns the number of parameters in the prepared statement.
size_t SqlStmt::NumParams(){
	return (size_t)mysql_stmt_param_count( this->stmt );
}



/// Binds a parameter to a buffer.
int32 SqlStmt::BindParam(size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len){
	if( !this->bind_params ){
		// initialize the bindings
		size_t count = this->NumParams();

		if( this->max_params < count ){
			this->max_params = count;
			RECREATE( this->params, MYSQL_BIND, count );
		}

		memset( this->params, 0, count * sizeof( MYSQL_BIND ) );

		for( size_t i = 0; i < count; ++i ){
			this->params[i].buffer_type = MYSQL_TYPE_NULL;
		}

		this->bind_params = true;
	}

	if( idx < this->max_params ){
		return Sql_P_BindSqlDataType( this->params + idx, buffer_type, buffer, buffer_len, nullptr, nullptr );
	}else{
		// TODO: for real...? Check this! [Lemongrass]
		return SQL_SUCCESS;// out of range - ignore
	}
}



/// Executes the prepared statement.
int32 SqlStmt::Execute(){
	this->FreeResult();

	if( ( this->bind_params && mysql_stmt_bind_param( this->stmt, this->params ) ) ||
		mysql_stmt_execute( this->stmt ) )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(this->stmt));
		ra_mysql_error_handler(mysql_stmt_errno(this->stmt));
		return SQL_ERROR;
	}

	this->bind_columns = false;

	// store all the data
	if( mysql_stmt_store_result( this->stmt ) ){
		ShowSQL( "DB error - %s\n", mysql_stmt_error( this->stmt ) );
		ra_mysql_error_handler( mysql_stmt_errno( this->stmt ) );
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}



/// Returns the number of the AUTO_INCREMENT column of the last INSERT/UPDATE statement.
uint64 SqlStmt::LastInsertId(){
	return (uint64)mysql_stmt_insert_id( this->stmt );
}



/// Returns the number of columns in each row of the result.
size_t SqlStmt::NumColumns(){
	return (size_t)mysql_stmt_field_count( this->stmt );
}



/// Binds the result of a column to a buffer.
int32 SqlStmt::BindColumn(size_t idx, enum SqlDataType buffer_type, void* buffer, size_t buffer_len, uint32* out_length, int8* out_is_null){
	if( buffer_type == SQLDT_STRING || buffer_type == SQLDT_ENUM )
	{
		if( buffer_len < 1 )
		{
			ShowDebug("SqlStmt::BindColumn: buffer_len(%" PRIuPTR ") is too small, no room for the nul-terminator\n", buffer_len);
			return SQL_ERROR;
		}
		--buffer_len;// nul-terminator
	}

	if( !this->bind_columns ){
		// initialize the bindings
		size_t cols = this->NumColumns();

		if( this->max_columns < cols ){
			this->max_columns = cols;
			RECREATE( this->columns, MYSQL_BIND, cols );
			RECREATE( this->column_lengths, s_column_length, cols );
		}
		memset( this->columns, 0, cols * sizeof( MYSQL_BIND ) );
		memset( this->column_lengths, 0, cols * sizeof( s_column_length ) );

		for( size_t i = 0; i < cols; ++i ){
			this->columns[i].buffer_type = MYSQL_TYPE_NULL;
		}

		this->bind_columns = true;
	}

	if( idx < this->max_columns ){
		this->column_lengths[idx].out_length = out_length;
		return Sql_P_BindSqlDataType( this->columns + idx, buffer_type, buffer, buffer_len, &this->column_lengths[idx].length, out_is_null );
	}else{
		// TODO: for real...? Check this! [Lemongrass]
		return SQL_SUCCESS;// out of range - ignore
	}
}



/// Returns the number of rows in the result.
uint64 SqlStmt::NumRows(){
	return (uint64)mysql_stmt_num_rows( this->stmt );
}



/// Fetches the next row.
int32 SqlStmt::NextRow(){
	int32 err;

	// bind columns
	if( this->bind_columns && mysql_stmt_bind_result(this->stmt, this->columns) ){
		err = 1;// error binding columns
	}else{
		// fetch row
		err = mysql_stmt_fetch( this->stmt );
	}

	// check for errors
	if( err == MYSQL_NO_DATA )
		return SQL_NO_DATA;
#if defined(MYSQL_DATA_TRUNCATED)
	// MySQL 5.0/5.1 defines and returns MYSQL_DATA_TRUNCATED [FlavioJS]
	if( err == MYSQL_DATA_TRUNCATED )
	{
		my_bool truncated;

		if( !this->bind_columns ){
			ShowSQL("DB error - data truncated (unknown source, columns are not bound)\n");
			return SQL_ERROR;
		}

		// find truncated column
		size_t cols = this->NumColumns();

		for( size_t i = 0; i < cols; ++i ){
			MYSQL_BIND* column = &this->columns[i];
			column->error = &truncated;
			mysql_stmt_fetch_column( this->stmt, column, (uint32)i, 0 );
			column->error = nullptr;
			if( truncated )
			{// report truncated column
				this->ShowDebugTruncatedColumn( i );
				return SQL_ERROR;
			}
		}
		ShowSQL("DB error - data truncated (unknown source)\n");
		return SQL_ERROR;
	}
#endif
	if( err )
	{
		ShowSQL("DB error - %s\n", mysql_stmt_error(this->stmt));
		ra_mysql_error_handler(mysql_stmt_errno(this->stmt));
		return SQL_ERROR;
	}

	// propagate column lengths and clear unused parts of string/enum/blob buffers
	size_t cols = this->NumColumns();

	for( size_t i = 0; i < cols; ++i ){
		unsigned long length = this->column_lengths[i].length;
		MYSQL_BIND* column = &this->columns[i];

#if !defined(MYSQL_DATA_TRUNCATED)
		// MySQL 4.1/(below?) returns success even if data is truncated, so we test truncation manually [FlavioJS]
		if( column->buffer_length < length )
		{// report truncated column
			if( column->buffer_type == MYSQL_TYPE_STRING || column->buffer_type == MYSQL_TYPE_BLOB )
			{// string/enum/blob column
				this->ShowDebugTruncatedColumn( i );
				return SQL_ERROR;
			}
			// FIXME numeric types and null [FlavioJS]
		}
#endif

		if( this->column_lengths[i].out_length ){
			*this->column_lengths[i].out_length = (uint32)length;
		}

		if( column->buffer_type == MYSQL_TYPE_STRING )
		{// clear unused part of the string/enum buffer (and nul-terminate)
			memset((char*)column->buffer + length, 0, column->buffer_length - length + 1);
		}
		else if( column->buffer_type == MYSQL_TYPE_BLOB && length < column->buffer_length )
		{// clear unused part of the blob buffer
			memset((char*)column->buffer + length, 0, column->buffer_length - length);
		}
	}

	return SQL_SUCCESS;
}



/// Frees the result of the statement execution.
void SqlStmt::FreeResult(){
	mysql_stmt_free_result( this->stmt );
}



/// Shows debug information (with statement).
void SqlStmt::ShowDebug_(const char* debug_file, const unsigned long debug_line){
#if !defined(SQL_REMOVE_SHOWDEBUG)
	if( StringBuf_Length( &this->buf ) > 0 ){
		ShowDebug("at %s:%lu - %s\n", debug_file, debug_line, StringBuf_Value(&this->buf));
	}else{
		ShowDebug("at %s:%lu\n", debug_file, debug_line);
	}
#endif
}



/// Frees a SqlStmt.
SqlStmt::~SqlStmt(){
	this->FreeResult();
	StringBuf_Destroy( &this->buf );
	if( this->stmt != nullptr ){
		mysql_stmt_close( this->stmt );
		this->stmt = nullptr;
	}

	if( this->params != nullptr ){
		aFree( this->params );
		this->params = nullptr;
	}

	if( this->columns != nullptr ){
		aFree( this->columns );
		this->columns = nullptr;
	}

	if( this->column_lengths != nullptr ){
		aFree( this->column_lengths );
		this->column_lengths = nullptr;
	}
}



/// Receives MySQL error codes during runtime (not on first-time-connects).
void ra_mysql_error_handler(uint32 ecode) {
	switch( ecode ) {
		case 2003:// Can't connect to MySQL (this error only happens here when failing to reconnect)
			if( mysql_reconnect_type == 1 ) {
				static uint32 retry = 1;
				if( ++retry > mysql_reconnect_count ) {
					ShowFatalError("MySQL has been unreachable for too long, %d reconnects were attempted. Shutting Down\n", retry);
					exit(EXIT_FAILURE);
				}
			}
			break;
	}
}

void Sql_inter_server_read(const char* cfgName, bool first) {
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	fp = fopen(cfgName, "r");
	if(fp == nullptr) {
		if( first ) {
			ShowFatalError("File not found: %s\n", cfgName);
			exit(EXIT_FAILURE);
		} else
			ShowError("File not found: %s\n", cfgName);
		return;
	}

	while(fgets(line, sizeof(line), fp)) {
		int32 i = sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2);
		if(i != 2)
			continue;

		if(!strcmpi(w1,"mysql_reconnect_type")) {
			mysql_reconnect_type = atoi(w2);
			switch( mysql_reconnect_type ) {
				case 1:
				case 2:
					break;
				default:
					ShowError("%s::mysql_reconnect_type is set to %d which is not valid, defaulting to 1...\n", cfgName, mysql_reconnect_type);
					mysql_reconnect_type = 1;
					break;
			}
		} else if(!strcmpi(w1,"mysql_reconnect_count")) {
			mysql_reconnect_count = atoi(w2);
			if( mysql_reconnect_count < 1 )
				mysql_reconnect_count = 1;
		} else if(!strcmpi(w1,"import"))
			Sql_inter_server_read(w2,false);
	}
	fclose(fp);

	return;
}

void Sql_Init(void) {
	Sql_inter_server_read(INTER_CONF_NAME,true);
}

#ifdef my_bool
#undef my_bool
#endif

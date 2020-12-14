#include "asyncquery.hpp"
#include "map.hpp"
#include "log.hpp"

#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/future.hpp"

#include <thread>

using namespace std;

extern int map_server_port;
extern char map_server_ip[64];
extern char map_server_id[32];
extern char map_server_pw[32];
extern char map_server_db[32];

extern int log_db_port;
extern char log_db_ip[64];
extern char log_db_id[32];
extern char log_db_pw[32];
extern char log_db_db[32];

size_t DBResultData::Index(int Row, int Column) {
	return Row * ColumnNum + Column;
}

const char* DBResultData::GetData(int Row, int Column) {
	return data.at(Index(Row, Column)).c_str();
}

void DBResultData::SetData(int Row, int Column, Sql* handle) {
	char* _data;
	Sql_GetData(handle, Column, &_data, NULL);
	data.at(Index(Row, Column)) = _data;
}

int8 DBResultData::GetInt8(int Row, int Column) {
	return atoi(GetData(Row, Column));
}

uint8 DBResultData::GetUInt8(int Row, int Column) {
	return atoi(GetData(Row, Column));
}

int16 DBResultData::GetInt16(int Row, int Column) {
	return atoi(GetData(Row, Column));
}

uint16 DBResultData::GetUInt16(int Row, int Column) {
	return atoi(GetData(Row, Column));
}

int32 DBResultData::GetInt32(int Row, int Column) {
	return atoi(GetData(Row, Column));
}

uint32 DBResultData::GetUInt32(int Row, int Column) {
	return strtoul(GetData(Row, Column), nullptr, 10);
}

int64 DBResultData::GetInt64(int Row, int Column) {
	return strtoll(GetData(Row, Column), NULL, 10);
}

uint64 DBResultData::GetUInt64(int Row, int Column) {
	return strtoull(GetData(Row, Column), NULL, 10);
}

thread* db_thread;
Sql* MainDBHandle = NULL, * LogDBHandle = NULL;

JobQueue<dbJob> dbJobs;

Sql* getHandle(dbType type) {
	switch (type) {
	default:
	case dbType::MAIN_DB: return MainDBHandle;
	case dbType::LOG_DB: return LogDBHandle;
	}
}

void addDBJob(dbType dType, string query, futureJobFunc resultFunc) {
	dbJobs.push_back({ dType, query, resultFunc });
}

void addDBJob(dbType dType, string query) {
	dbJobs.push_back({ dType, query, NULL });
}

void doQuery(dbJob& job) {
	Sql* handle = getHandle(job.dType);
	int sql_result_value;

	if (SQL_ERROR == (sql_result_value = Sql_QueryStr(handle, job.query.c_str())))
		Sql_ShowDebug(handle);
	else if (job.resultFunc) {
		DBResultData* r = new DBResultData(
			(size_t)Sql_NumRows(handle),
			(size_t)Sql_NumColumns(handle),
			(size_t)Sql_NumRowsAffected(handle)
		);

		r->sql_result_value = sql_result_value;

		for (size_t Row = 0; Row < r->RowNum && SQL_SUCCESS == Sql_NextRow(handle); Row++)
			for (size_t ColumnNum = 0; ColumnNum < r->ColumnNum; ColumnNum++)
				r->SetData(Row, ColumnNum, handle);

		add_future(job.resultFunc, (FutureData)r);
	}

	Sql_FreeResult(handle);
}

void db_runtime(void) {
	while (runflag != CORE_ST_STOP) {
		this_thread::sleep_for(chrono::milliseconds(50));

		dbJobs.Run([](dbJob& job) {
			doQuery(job);
			});
	}

	ShowStatus("Close DB Server(async thread) Connection....\n");
	Sql_Free(MainDBHandle);
	MainDBHandle = NULL;
	if (log_config.sql_logs)
		Sql_Free(LogDBHandle);
	LogDBHandle = NULL;
}

void asyncquery_init(void) {
	MainDBHandle = Sql_Malloc();

	ShowInfo("Connecting to the DB Server(async thread)....\n");
	if (SQL_ERROR == Sql_Connect(MainDBHandle, map_server_id, map_server_pw, map_server_ip, map_server_port, map_server_db))
	{
		ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
			map_server_id, map_server_pw, map_server_ip, map_server_port, map_server_db);
		Sql_ShowDebug(MainDBHandle);
		Sql_Free(MainDBHandle);
		exit(EXIT_FAILURE);
	}
	ShowStatus("Connect success! (DB Server(async thread) Connection)\n");

	if (log_config.sql_logs) {
		LogDBHandle = Sql_Malloc();

		ShowInfo("Connecting to the Log DB Server(async thread)....\n");
		if (SQL_ERROR == Sql_Connect(LogDBHandle, log_db_id, log_db_pw, log_db_ip, log_db_port, log_db_db))
		{
			ShowError("Couldn't connect with uname='%s',passwd='%s',host='%s',port='%d',database='%s'\n",
				log_db_id, log_db_pw, log_db_ip, log_db_port, log_db_db);
			Sql_ShowDebug(LogDBHandle);
			Sql_Free(LogDBHandle);
			exit(EXIT_FAILURE);
		}

		ShowStatus("Connect success! (Log DB Server(async thread) Connection)\n");
	}

	db_thread = new thread(db_runtime);
}

void asyncquery_final(void) {
	db_thread->join();
}

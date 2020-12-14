#pragma once

#include "../common/sql.hpp"
#include "../common/future.hpp"
#include <vector>
#include <string>
#include <functional>

struct DBResultData {
private:
	size_t Index(int Row, int Column);

public:
	std::vector<std::string> data;
	size_t ColumnNum = 0;
	size_t RowNum = 0;
	size_t NumRowsAffected = 0;
	int sql_result_value = 0;

	DBResultData(size_t rowNum, size_t columnNum, size_t numRowsAffected) : data(rowNum* columnNum) {
		ColumnNum = columnNum;
		RowNum = rowNum;
		NumRowsAffected = numRowsAffected;
	};

	DBResultData(const DBResultData* src) : DBResultData(src->RowNum, src->ColumnNum, src->NumRowsAffected) {
		sql_result_value = src->sql_result_value;
		data.assign(src->data.begin(), src->data.end());
	};

	const char* GetData(int Row, int Column);
	void SetData(int Row, int Column, Sql* handle);
	int8 GetInt8(int Row, int Column);
	uint8 GetUInt8(int Row, int Column);
	int16 GetInt16(int Row, int Column);
	uint16 GetUInt16(int Row, int Column);
	int32 GetInt32(int Row, int Column);
	uint32 GetUInt32(int Row, int Column);
	int64 GetInt64(int Row, int Column);
	uint64 GetUInt64(int Row, int Column);
};

enum class dbType {
	MAIN_DB,
	LOG_DB,
};

typedef std::function<void(DBResultData& result)> dbJobFunc;

struct dbJob {
	dbType dType;
	std::string query;
	futureJobFunc resultFunc;
};

void addDBJob(dbType dType, std::string query, futureJobFunc resultFunc);
void addDBJob(dbType dType, std::string query);
void asyncquery_init(void);
void asyncquery_final(void);

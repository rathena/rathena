// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SQLLOCK_HPP
#define SQLLOCK_HPP

#include <mutex>

#include <common/sql.hpp>

enum locktype {
	LOGIN_SQL_LOCK,
	CHAR_SQL_LOCK,
	MAP_SQL_LOCK,
	WEB_SQL_LOCK
};

class SQLLock {
private:
	std::unique_lock<std::mutex> ulock;
	Sql * handle;
	locktype lt;

public:
	SQLLock(locktype);
	~SQLLock();
	void lock();
	void unlock();
	Sql * getHandle();
};


#endif

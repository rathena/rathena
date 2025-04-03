// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "sqllock.hpp"

#include <mutex>

#include <common/showmsg.hpp>

std::mutex dbmutex;

extern Sql * login_handle;
extern Sql * char_handle;
extern Sql * map_handle;
extern Sql * web_handle;


SQLLock::SQLLock(locktype lt) : ulock(dbmutex, std::defer_lock), lt(lt) {
	switch(lt) {
		case LOGIN_SQL_LOCK:
			handle = login_handle;
			break;
		case CHAR_SQL_LOCK:
			handle = char_handle;
			break;
		case MAP_SQL_LOCK:
			handle = map_handle;
			break;
		case WEB_SQL_LOCK:
			handle = web_handle;
			break;
	}
}

void SQLLock::lock() {
	// switch(lt) {
	//     case LOGIN_SQL_LOCK:
	//         ShowDebug("Locking login sql\n");
	//         break;
	//     case CHAR_SQL_LOCK:
	//         ShowDebug("Locking char sql\n");
	//         break;
	//     case MAP_SQL_LOCK:
	//         ShowDebug("Locking map sql\n");
	//         break;
	//     case WEB_SQL_LOCK:
	//         ShowDebug("Locking web sql\n");
	//         break;
	// }
	ulock.lock();
}

void SQLLock::unlock() {
	ulock.unlock();
	// switch(lt) {
	//     case LOGIN_SQL_LOCK:
	//         ShowDebug("Unlocked login sql\n");
	//         break;
	//     case CHAR_SQL_LOCK:
	//         ShowDebug("Unlocked char sql\n");
	//         break;
	//     case MAP_SQL_LOCK:
	//         ShowDebug("Unlocked map sql\n");
	//         break;
	//     case WEB_SQL_LOCK:
	//         ShowDebug("Unlocked web sql\n");
	//         break;
	// }

}


// can only get handle if locked
Sql * SQLLock::getHandle() {
	if (!ulock.owns_lock())
		return nullptr;
	return handle;
}

SQLLock::~SQLLock() {
}

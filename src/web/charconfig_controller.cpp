// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "charconfig_controller.hpp"

#include <string>
#include <nlohmann/json.hpp>

#include <common/showmsg.hpp>
#include <common/sql.hpp>

#include "auth.hpp"
#include "http.hpp"
#include "sqllock.hpp"
#include "webutils.hpp"
#include "web.hpp"

HANDLER_FUNC(charconfig_save) {
	if (!isAuthorized(req, false)) {
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}
	
	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto char_id = std::stoi(req.get_file_value("GID").content);
	auto world_name = req.get_file_value("WorldName").content;
	auto data = nlohmann::json::object();

	if (req.has_file("data")) {
		data = nlohmann::json::parse(req.get_file_value("data").content);
	}

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt stmt{ *handle };
	if (SQL_SUCCESS != stmt.Prepare(
			"SELECT `data` FROM `%s` WHERE (`account_id` = ? AND `char_id` = ? AND `world_name` = ?) LIMIT 1",
			char_configs_table)
		|| SQL_SUCCESS != stmt.BindParam( 0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt.BindParam( 1, SQLDT_INT32, &char_id, sizeof(char_id))
		|| SQL_SUCCESS != stmt.BindParam( 2, SQLDT_STRING, (void *)world_name.c_str(), world_name.length())
		|| SQL_SUCCESS != stmt.Execute()
	) {
		SqlStmt_ShowDebug(stmt);
		sl.unlock();
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}

	if (stmt.NumRows() > 0) {
		char databuf[SQL_BUFFER_SIZE];
		if (SQL_SUCCESS != stmt.BindColumn( 0, SQLDT_STRING, &databuf, sizeof(databuf), nullptr, nullptr)
			|| SQL_SUCCESS != stmt.NextRow()
		) {
			SqlStmt_ShowDebug(stmt);
			sl.unlock();
			res.status = HTTP_BAD_REQUEST;
			res.set_content("Error", "text/plain");
			return;
		}

		auto db_data = nlohmann::json::parse(databuf);
		mergeData(db_data, data, false);
		data = std::move(db_data);
	}

	auto data_str = data.dump();

	if (SQL_SUCCESS != stmt.Prepare(
			"REPLACE INTO `%s` (`account_id`, `char_id`, `world_name`, `data`) VALUES (?, ?, ?, ?)",
			char_configs_table)
		|| SQL_SUCCESS != stmt.BindParam( 0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt.BindParam( 1, SQLDT_INT32, &char_id, sizeof(char_id))
		|| SQL_SUCCESS != stmt.BindParam( 2, SQLDT_STRING, (void *)world_name.c_str(), world_name.length())
		|| SQL_SUCCESS != stmt.BindParam( 3, SQLDT_STRING, (void *)data_str.c_str(), data_str.length())
		|| SQL_SUCCESS != stmt.Execute()
	) {
		SqlStmt_ShowDebug(stmt);
		sl.unlock();
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}

	sl.unlock();
	res.set_content(data_str, "application/json");
}

HANDLER_FUNC(charconfig_load) {
	if (!req.has_file("AID") || !req.has_file("WorldName")) {
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}

	// TODO: Figure out when client sends AuthToken for this path, then add packetver check
	// if (!isAuthorized(req)) {
		// ShowError("Not authorized!\n");
		// message.reply(web::http::status_codes::Forbidden);
		// return;
	// }

	auto account_id = std::stoi(req.get_file_value("AID").content);
	auto char_id = std::stoi(req.get_file_value("GID").content);
	auto world_name_str = req.get_file_value("WorldName").content;
	auto world_name = world_name_str.c_str();

	SQLLock sl(WEB_SQL_LOCK);
	sl.lock();
	auto handle = sl.getHandle();
	SqlStmt stmt{ *handle };
	if (SQL_SUCCESS != stmt.Prepare(
			"SELECT `data` FROM `%s` WHERE (`account_id` = ? AND `char_id` = ? AND `world_name` = ?) LIMIT 1",
			char_configs_table)
		|| SQL_SUCCESS != stmt.BindParam(0, SQLDT_INT32, &account_id, sizeof(account_id))
		|| SQL_SUCCESS != stmt.BindParam(1, SQLDT_INT32, &char_id, sizeof(char_id))
		|| SQL_SUCCESS != stmt.BindParam(2, SQLDT_STRING, (void *)world_name, strlen(world_name))
		|| SQL_SUCCESS != stmt.Execute()
	) {
		SqlStmt_ShowDebug(stmt);
		sl.unlock();
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}

	if (stmt.NumRows() <= 0) {
		std::string data = "{\"Type\": 1}";

		if( SQL_SUCCESS != stmt.Prepare( "INSERT INTO `%s` (`account_id`, `char_id`, `world_name`, `data`) VALUES (?, ?, ?, ?)", char_configs_table ) ||
			SQL_SUCCESS != stmt.BindParam( 0, SQLDT_INT32, &account_id, sizeof( account_id ) ) ||
			SQL_SUCCESS != stmt.BindParam( 1, SQLDT_INT32, &char_id, sizeof( char_id ) ) ||
			SQL_SUCCESS != stmt.BindParam( 2, SQLDT_STRING, (void*)world_name, strlen( world_name ) ) ||
			SQL_SUCCESS != stmt.BindParam( 3, SQLDT_STRING, (void*)data.c_str(), strlen( data.c_str() ) ) ||
			SQL_SUCCESS != stmt.Execute() ){
			SqlStmt_ShowDebug( stmt );
			sl.unlock();
			res.status = HTTP_BAD_REQUEST;
			res.set_content( "Error", "text/plain" );
			return;
		}

		sl.unlock();
		res.set_content( data, "application/json" );
		return;
	}

	char databuf[SQL_BUFFER_SIZE];

	if (SQL_SUCCESS != stmt.BindColumn(0, SQLDT_STRING, &databuf, sizeof(databuf), nullptr, nullptr)
		|| SQL_SUCCESS != stmt.NextRow()
	) {
		SqlStmt_ShowDebug(stmt);
		sl.unlock();
		res.status = HTTP_BAD_REQUEST;
		res.set_content("Error", "text/plain");
		return;
	}

	sl.unlock();

	databuf[sizeof(databuf) - 1] = 0;

	auto response = nlohmann::json::parse(databuf);
	response["Type"] = 1;
	res.set_content(response.dump(), "application/json");
}

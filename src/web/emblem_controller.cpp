// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "emblem_controller.hpp"

#include <iostream>
#include <ostream>
#include <fstream>

#include "../common/showmsg.hpp"
#include "auth.hpp"
#include "web.hpp"
#include "http.hpp"
#include "sqllock.hpp"

// Max size is 50kb for gif
#define MAX_EMBLEM_SIZE 50000
#define START_VERSION 1

std::string getUniqueFileName(const int guild_id, const std::string& world_name, const int version) {
    auto stream = std::ostringstream();
    stream << world_name << "_" << guild_id << "_" << version;
    return stream.str();
}

HANDLER_FUNC(emblem_download) {
    if (!isAuthorized(req, false)) {
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    if (!req.has_file("GDID") || !req.has_file("WorldName")) {
        ShowDebug("Missing required fields for emblem download!\n");
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    auto world_name_str = req.get_file_value("WorldName").content;
    auto world_name = world_name_str.c_str();
    auto guild_id = std::stoi(req.get_file_value("GDID").content);

    SQLLock sl(WEB_SQL_LOCK);
    sl.lock();
    auto handle = sl.getHandle();
    SqlStmt * stmt = SqlStmt_Malloc(handle);
    if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
            "SELECT `version`, `file_name`, `file_type` FROM `%s` WHERE (`guild_id` = ? AND `world_name` = ?)",
            guild_emblems_table)
        || SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_INT, &guild_id, sizeof(guild_id))
        || SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void *)world_name, strlen(world_name))
        || SQL_SUCCESS != SqlStmt_Execute(stmt)
    ) {
        SqlStmt_ShowDebug(stmt);
        SqlStmt_Free(stmt);
        sl.unlock();
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    uint32 version = 0;
    std::string filename;
    char tmp[256];
    char filetype[256];

    if (SqlStmt_NumRows(stmt) <= 0) {
        SqlStmt_Free(stmt);
        ShowError("[%d, \"%s\"] Not found in table\n", guild_id, world_name);
        sl.unlock();
        res.status = 404;
        res.set_content("Error", "text/plain");
        return;
    }

    if (SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_UINT32, &version, sizeof(version), NULL, NULL)
        || SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &tmp, sizeof(tmp), NULL, NULL)
        || SQL_SUCCESS != SqlStmt_BindColumn(stmt, 2, SQLDT_STRING, &filetype, sizeof(filetype), NULL, NULL)
        || SQL_SUCCESS != SqlStmt_NextRow(stmt)
    ) {
        SqlStmt_ShowDebug(stmt);
        SqlStmt_Free(stmt);
        sl.unlock();
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    SqlStmt_Free(stmt);
    sl.unlock();

    const char * content_type;

    if (!strcmp(filetype, "BMP"))
        content_type = "image/bmp";
    else if (!strcmp(filetype, "GIF"))
        content_type = "image/gif";

    filename = web_config.guild_emblem_dir + PATHSEP_STR + tmp;
    ShowDebug("Opening file [%s] for reading\n", filename.c_str());
    std::ifstream emblemFile(filename, std::ios_base::binary);
    emblemFile.seekg(0, std::ios::end);
    auto length = emblemFile.tellg();
    emblemFile.seekg(0, std::ios::beg);
    if (length > MAX_EMBLEM_SIZE) {
        ShowDebug("Emblem is too big, size [%d] max length is %d\n", length, MAX_EMBLEM_SIZE);
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }
    res.body.resize(static_cast<size_t>(length));
    emblemFile.read(&res.body[0], static_cast<std::streamsize>(length));
    res.set_header("Content-Type", content_type);
}


HANDLER_FUNC(emblem_upload) {
    if (!isAuthorized(req, true)) {
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    if (!req.has_file("GDID") || !req.has_file("WorldName") || !req.has_file("Img")|| !req.has_file("ImgType")) {
        ShowDebug("Missing required fields for emblem download!\n");
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    auto world_name_str = req.get_file_value("WorldName").content;
    auto world_name = world_name_str.c_str();
    auto guild_id = std::stoi(req.get_file_value("GDID").content);
    auto imgtype_str = req.get_file_value("ImgType").content;
    auto imgtype = imgtype_str.c_str();
    auto img = req.get_file_value("Img").content;
    
    if (imgtype_str != "BMP" && imgtype_str != "GIF") {
        ShowError("Invalid image type found [%s], rejecting!\n", imgtype);
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    auto length = img.length();
    if (length > MAX_EMBLEM_SIZE) {
        ShowDebug("Emblem is too big, size [%d] max length is %d\n", length, MAX_EMBLEM_SIZE);
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    SQLLock sl(WEB_SQL_LOCK);
    sl.lock();
    auto handle = sl.getHandle();
    SqlStmt * stmt = SqlStmt_Malloc(handle);
    if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
            "SELECT `version`, `file_name` FROM `%s` WHERE (`guild_id` = ? AND `world_name` = ?)",
            guild_emblems_table)
        || SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_INT, &guild_id, sizeof(guild_id))
        || SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void *)world_name, strlen(world_name))
        || SQL_SUCCESS != SqlStmt_Execute(stmt)
    ) {
        SqlStmt_ShowDebug(stmt);
        SqlStmt_Free(stmt);
        sl.unlock();
        res.status = 400;
        res.set_content("Error", "text/plain");
        return;
    }

    uint32 version = START_VERSION;
    std::string filename;
    char tmp[256];

    if (SqlStmt_NumRows(stmt) > 0) {
        if (SQL_SUCCESS != SqlStmt_BindColumn(stmt, 0, SQLDT_UINT32, &version, sizeof(version), NULL, NULL)
            || SQL_SUCCESS != SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &tmp, sizeof(tmp), NULL, NULL)
            || SQL_SUCCESS != SqlStmt_NextRow(stmt)
        ) {
            SqlStmt_ShowDebug(stmt);
            SqlStmt_Free(stmt);
            sl.unlock();
            res.status = 400;
            res.set_content("Error", "text/plain");
            return;
        }
        version += 1;
        tmp[255] = 0;
        filename = web_config.guild_emblem_dir + PATHSEP_STR + tmp;
        std::remove(filename.c_str());
    }

    filename = getUniqueFileName(guild_id, world_name_str, version);

    if (version > START_VERSION) {
        // update existing
        if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
            "UPDATE `%s` SET `version` = ?, `file_name` = ?, `file_type` = ? WHERE (`guild_id` = ? AND `world_name` = ?)",
            guild_emblems_table)
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_UINT32, &version, sizeof(version))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void *)filename.c_str(), strlen(filename.c_str()))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void *)imgtype, strlen(imgtype))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_INT, &guild_id, sizeof(guild_id))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_STRING, (void *)world_name, strlen(world_name))
            || SQL_SUCCESS != SqlStmt_Execute(stmt)
        ) {
            SqlStmt_ShowDebug(stmt);
            SqlStmt_Free(stmt);
            sl.unlock();
            res.status = 400;
            res.set_content("Error", "text/plain");
            return;
        }
    
    } else {
        // insert new
        if (SQL_SUCCESS != SqlStmt_Prepare(stmt,
            "INSERT INTO `%s` (`version`, `file_name`, `file_type`, `guild_id`, `world_name`) VALUES (?, ?, ?, ?, ?)",
            guild_emblems_table)
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_UINT32, &version, sizeof(version))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, (void *)filename.c_str(), strlen(filename.c_str()))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, (void *)imgtype, strlen(imgtype))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 3, SQLDT_INT, &guild_id, sizeof(guild_id))
            || SQL_SUCCESS != SqlStmt_BindParam(stmt, 4, SQLDT_STRING, (void *)world_name, strlen(world_name))
            || SQL_SUCCESS != SqlStmt_Execute(stmt)
        ) {
            SqlStmt_ShowDebug(stmt);
            SqlStmt_Free(stmt);
            sl.unlock();
            res.status = 400;
            res.set_content("Error", "text/plain");
            return;
        }
    }

    SqlStmt_Free(stmt);
    sl.unlock();

    filename = web_config.guild_emblem_dir + PATHSEP_STR + filename;
    ShowDebug("Opening file [%s] for writing\n", filename.c_str());
    std::ofstream emblemfile(filename, std::ofstream::binary);
    emblemfile.write(img.c_str(), img.length());
    emblemfile.close();

    std::ostringstream stream;
    stream << "{\"Type\":1,\"version\":"<< version << "}";
    res.set_content(stream.str(), "application/json");
}

// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ACCOUNTDB_HPP
#define ACCOUNTDB_HPP

#include <memory>
#include <string>

#include <common/cbasetypes.hpp>
#include <config/core.hpp>
#include "mmo_account.hpp"

class AccountDB {
   public:
	virtual ~AccountDB(){};

	virtual bool init() = 0;

	/// Sets a property in this database.
	///
	/// @param key Property name
	/// @param value Property value
	/// @return true if successful
	virtual bool set_property(const char* key, const char* value) = 0;

	/// Creates a new account in this database.
	/// If acc->account_id is not -1, the provided value will be used.
	/// Otherwise the account_id will be auto-generated and written to acc->account_id.
	///
	/// @param acc Account data
	/// @return true if successful
	virtual bool create(struct mmo_account& acc) = 0;

	/// Removes an account from this database.
	///
	/// @param account_id Account id
	/// @return true if successful
	virtual bool remove(const uint32 account_id) = 0;

	/// Enables the web auth token for the given account id
	virtual bool enable_webtoken(const uint32 account_id) = 0;

	/// Starts timer for disabling the web auth token for the given account id
	virtual bool disable_webtoken_timer(const uint32 account_id) = 0;

	// Actually disable the web auth token for the given account id
	virtual int disable_webtoken(const uint32 account_id) = 0;

	/// Removes the web auth token for all accounts
	virtual bool remove_webtokens() = 0;

	/// Modifies the data of an existing account.
	/// Uses acc->account_id to identify the account.
	///
	/// @param acc Account data
	/// @param refresh_token Whether or not to refresh the web auth token
	/// @return true if successful
	virtual bool save(const struct mmo_account& acc, bool refresh_token) = 0;

	/// Finds an account with account_id and copies it to acc.
	///
	/// @param acc Pointer that receives the account data
	/// @param account_id Target account id
	/// @return true if successful
	virtual bool load_num(struct mmo_account& acc, const uint32 account_id) = 0;

	/// Finds an account with userid and copies it to acc.
	///
	/// @param acc Pointer that receives the account data
	/// @param userid Target username
	/// @return true if successful
	virtual bool load_str(struct mmo_account& acc, const char* userid) = 0;

	virtual void send_global_accreg(int fd, uint32 account_id, uint32 char_id) = 0;
	virtual void save_global_accreg(int fd, uint32 account_id, uint32 char_id) = 0;
};

#endif /* ACCOUNTDB_HPP */

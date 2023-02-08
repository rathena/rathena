// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef ACCOUNTDBSQL_HPP
#define ACCOUNTDBSQL_HPP

#include <memory>
#include <string>

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp" // ACCOUNT_REG2_NUM
#include "../common/sql.hpp"
#include "../config/core.hpp"

#include "accountdb.hpp"
#include "mmo_account.hpp"

class AccountDBSql : public AccountDB {
public:
	/// Constructor
	AccountDBSql() {};

	/// Destroys this database, releasing all allocated memory (including itself).
	~AccountDBSql();

	bool init() override;

	/// Gets a property from this database.
	/// These read-only properties must be implemented:
	///
	/// @param key Property name
	/// @param buf Buffer for the value
	/// @param buflen Buffer length
	/// @return true if successful
	bool get_property(const char* key, char* buf, size_t buflen) override;

	/// Sets a property in this database.
	///
	/// @param key Property name
	/// @param value Property value
	/// @return true if successful
	bool set_property(const char* key, const char* value) override;

	/// Creates a new account in this database.
	/// If acc->account_id is not -1, the provided value will be used.
	/// Otherwise the account_id will be auto-generated and written to acc->account_id.
	///
	/// @param acc Account data
	/// @return true if successful
	bool create(struct mmo_account& acc) override;

	/// Removes an account from this database.
	///
	/// @param account_id Account id
	/// @return true if successful
	bool remove(const uint32 account_id) override;

	/// Enables the web auth token for the given account id
	bool enable_webtoken(const uint32 account_id) override;

	/// Starts timer for disabling the web auth token for the given account id
	bool disable_webtoken_timer(const uint32 account_id) override;

	// Actually disable the web auth token for the given account id
	int disable_webtoken(const uint32 account_id) override;

	/// Removes the web auth token for all accounts
	bool remove_webtokens() override;

	/// Modifies the data of an existing account.
	/// Uses acc->account_id to identify the account.
	///
	/// @param acc Account data
	/// @param refresh_token Whether or not to refresh the web auth token
	/// @return true if successful
	bool save(const struct mmo_account& acc, bool refresh_token) override;

	/// Finds an account with account_id and copies it to acc.
	///
	/// @param acc Pointer that receives the account data
	/// @param account_id Target account id
	/// @return true if successful
	bool load_num(struct mmo_account& acc, const uint32 account_id) override;

	/// Finds an account with userid and copies it to acc.
	///
	/// @param acc Pointer that receives the account data
	/// @param userid Target username
	/// @return true if successful
	bool load_str(struct mmo_account& acc, const char* userid) override;

	void send_global_accreg(int fd, uint32 account_id, uint32 char_id) override;
	void save_global_accreg(int fd, uint32 account_id, uint32 char_id) override;


private:
	Sql * accounts_{nullptr};       // SQL handle accounts storage
	std::string db_hostname_{"127.0.0.1"}; // Doubled for long hostnames (bugreport:8003)
	uint16 db_port_{3306};
	std::string db_username_{"ragnarok"};
	std::string db_password_{""};
	std::string db_database_{"ragnarok"};
	std::string codepage_{""};
	// other settings
	bool case_sensitive_{false};
	//table name
	std::string account_db_{"login"};
	std::string global_acc_reg_num_table_{"global_acc_reg_num"};
	std::string global_acc_reg_str_table_{"global_acc_reg_str"};

	bool from_sql(struct mmo_account& acc, uint32 account_id);
	bool to_sql(const struct mmo_account& acc, bool is_new, bool refresh_token);

	Sql * get_handle();
};


#endif /* ACCOUNTDBSQL_HPP */

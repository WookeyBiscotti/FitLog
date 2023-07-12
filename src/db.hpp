#pragma once

#include "info.hpp"
#include "user_info.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>

#include <ctime>
#include <memory>
#include <unordered_map>

class Db {
  public:
	Db() : _db("fit_log.db3", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
		if (!_db.tableExists("fit_log_info")) {
			createIfEmpty();
		}

		auto version = _db.execAndGet("SELECT version FROM fit_log_info").getInt();
		if (version != fit_log_version) {
			SQLite::Transaction tr(_db);
			migrateFrom(version);
			_db.exec("DELETE FROM fit_log_info");
			_db.exec(fmt::format("INSERT INTO fit_log_info VALUES({})", fit_log_version));
			tr.commit();
		}
	}

	void createIfEmpty() {
		SQLite::Transaction tr(_db);

		_db.exec("CREATE TABLE fit_log_info (version INTEGER)");
		_db.exec(fmt::format("INSERT INTO fit_log_info VALUES({})", fit_log_version));

		_db.exec("CREATE TABLE user_info (id INTEGER KEY, name TEXT, chatId INTEGER)");

		_db.exec("CREATE TABLE body_value (userId INTEGER KEY, ts INTEGER, name TEXT, value REAL)");

		tr.commit();
	}

	void migrateFrom(int version) {
		if (version == 1) {
			migrateFrom1to2();
		}
	}

	void migrateFrom1to2() {
		_db.exec("CREATE TABLE food_entry (userId INTEGER KEY, ts INTEGER, name TEXT, kcallories REAL, isLiquid "
				 "INTEGER)");
	}

	bool userExist(int64_t id) {
		return _db.execAndGet(fmt::format("SELECT count(*) FROM user_info WHERE id='{}'", id)).getInt() != 0;
	}

	std::shared_ptr<UserInfo> createUser(int64_t id, const std::string name, int64_t chatId) {
		_db.exec(fmt::format("INSERT INTO user_info VALUES({}, '{}', {})", id, name, chatId));

		_cachedUserInfo[id] = std::make_shared<UserInfo>(id, name, chatId);

		return _cachedUserInfo.at(id);
	}

	std::shared_ptr<UserInfo> getUser(int64_t id) {
		if (_cachedUserInfo.count(id)) {
			return _cachedUserInfo.at(id);
		}
		SQLite::Statement queryUserInfo(_db, fmt::format("SELECT id, name, chatId FROM user_info WHERE id={}", id));
		queryUserInfo.executeStep();
		if (!queryUserInfo.hasRow()) {
			return nullptr;
		}

		_cachedUserInfo[id] =
			std::make_shared<UserInfo>(queryUserInfo.getColumn(0).getInt64(), queryUserInfo.getColumn(1).getString(),
									   queryUserInfo.getColumn(2).getInt64());

		return _cachedUserInfo.at(id);
	}

	void addBodyValue(int64_t id, const std::string& name, float value) {
		_db.exec(fmt::format("INSERT INTO body_value VALUES({}, {}, '{}', {})", id, std::time(nullptr), name, value));
	}

	std::vector<std::pair<uint64_t, float>> getBodyValues(int64_t id, const std::string& name, uint64_t fromTs,
														  uint64_t toTs) {
		SQLite::Statement q(
			_db, fmt::format("SELECT ts, value FROM body_value WHERE userId={} AND name='{}' AND ts >= {} AND ts <= {}",
							 id, name, fromTs, toTs));
		std::vector<std::pair<uint64_t, float>> res;
		while (q.executeStep()) {
			res.emplace_back(std::pair<uint64_t, float>{q.getColumn(0).getInt64(), q.getColumn(1).getDouble()});
		}

		return res;
	}

	void addFoodEntry(int64_t id, const std::string& name, float kCallories, bool isLiquid) {
		_db.exec(fmt::format("INSERT INTO food_entry VALUES({}, {}, '{}', {}, {})", id, std::time(nullptr), name,
							 kCallories, isLiquid ? 1 : 0));
	}

	std::vector<std::pair<uint64_t, float>> getFoodEntries(int64_t id, const std::string& name, bool isLiquid,
														   uint64_t fromTs, uint64_t toTs) {
		SQLite::Statement q(_db,
							fmt::format("SELECT ts, kcallories FROM food_entry WHERE userId={} AND name='{}' AND ts >= "
										"{} AND ts <= {} AND isLiquid == {}",
										id, name, fromTs, toTs, isLiquid ? 1 : 0));
		std::vector<std::pair<uint64_t, float>> res;
		while (q.executeStep()) {
			res.emplace_back(std::pair<uint64_t, float>{q.getColumn(0).getInt64(), q.getColumn(1).getDouble()});
		}

		return res;
	}

	std::vector<std::pair<uint64_t, float>> getFoodEntriesAll(int64_t id, bool isLiquid, uint64_t fromTs,
															  uint64_t toTs) {
		SQLite::Statement q(_db, fmt::format("SELECT ts, kcallories FROM food_entry WHERE userId={} AND ts >= "
											 "{} AND ts <= {} AND isLiquid == {}",
											 id, fromTs, toTs, isLiquid ? 1 : 0));
		std::vector<std::pair<uint64_t, float>> res;
		while (q.executeStep()) {
			res.emplace_back(std::pair<uint64_t, float>{q.getColumn(0).getInt64(), q.getColumn(1).getDouble()});
		}

		return res;
	}

  private:
	std::unordered_map<int64_t, std::shared_ptr<UserInfo>> _cachedUserInfo;
	SQLite::Database _db;
};

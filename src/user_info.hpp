#pragma once

#include "body_value.hpp"

#include <nlohmann/json.hpp>

#include <cinttypes>
#include <memory>
#include <string>

struct UserInfo {
	UserInfo(int64_t id, std::string name, int64_t chatId) : id(id), name(name), chatId(chatId) {}

	int64_t id;

	std::string name;

	int64_t chatId;
};

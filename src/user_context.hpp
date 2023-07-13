#pragma once

#include "user_info.hpp"

#include <memory>

class Command;

struct UserContext {
	UserContext() = default;
	UserContext(const UserContext&) = delete;

	std::shared_ptr<UserInfo> userInfo;

	struct StackEntry {
		Command* command;
		nlohmann::json state;
	};
	std::vector<StackEntry> stack;
};

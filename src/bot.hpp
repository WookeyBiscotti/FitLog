#pragma once

#include "commands.hpp"
#include "commands/command.hpp"
#include "db.hpp"
#include "render.hpp"
#include "token.hpp"
#include "user_context.hpp"

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <sciplot/sciplot.hpp>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>

#include <algorithm>
#include <csignal>

class Bot {
  public:
	Bot();

	void run();

	void addOnCommand(const std::string& name, Command& command);

	UserContext& getOrCreateUserContext(int64_t id, const std::string& name, int64_t chatId);

	const TgBot::Api& api() { return _bot.getApi(); }

  private:
	template <class C> void addOnCommand() {
		auto it = _commands.emplace(C::name(), std::make_unique<C>(_db, *this));
		addOnCommand(it.first->first, *it.first->second);
	}

  private:
	TgBot::Bot _bot;
	Db _db;
	std::unordered_map<int64_t, UserContext> _userContexts;
	std::unordered_map<std::string, std::unique_ptr<Command>> _commands;
};

#include "bot.hpp"

#include "commands/add_body_stats.hpp"
#include "commands/add_food_stats.hpp"
#include "commands/start.hpp"
#include "commands/stats_menu.hpp"

Bot::Bot() : _bot(findToken()) {
	signal(SIGINT, [](int s) {
		printf("SIGINT got\n");
		exit(0);
	});
}

void Bot::addOnCommand(const std::string& name, Command& command) {
	_bot.getEvents().onCommand(name, [name, &command, this](TgBot::Message::Ptr msg) {
		auto& uc = getOrCreateUserContext(msg->from->id, msg->from->username, msg->chat->id);

		try {
			command.onCommand(uc, name);
		} catch (...) {
			uc.stack.clear();
			throw;
		}
	});
}

UserContext& Bot::getOrCreateUserContext(int64_t id, const std::string& name, int64_t chatId) {
	if (auto found = _userContexts.find(id); found != _userContexts.end()) {
		return found->second;
	} else {
		auto user = _db.getUser(id);
		if (!user) {
			user = _db.createUser(id, name, chatId);
		}
		_userContexts[user->id].userInfo = user;

		return _userContexts[user->id];
	}
}

void Bot::run() {
	using namespace TgBot;
	addOnCommand<StartCommand>();
	addOnCommand<AddBodyStats>();
	addOnCommand<StatsMenu>();
	addOnCommand<AddFoodStats>();

	_bot.getEvents().onNonCommandMessage([this](Message::Ptr msg) {
		auto& uc = getOrCreateUserContext(msg->from->id, msg->from->username, msg->chat->id);

		if (uc.stack.empty()) {
			return;
		}
		auto cmd = uc.stack.back().command;

		try {
			cmd->onNonCommand(uc, msg->text);
		} catch (...) {
			uc.stack.clear();
			throw;
		}
	});

	_bot.getEvents().onCallbackQuery([this](CallbackQuery::Ptr query) {
		if (!query->message && !query->data.empty()) {
			return;
		}

		auto& uc = getOrCreateUserContext(query->from->id, query->from->username, query->message->chat->id);
		if (query->data.front() == '/') {
			auto cmd = query->data.substr(1);
			if (auto found = _commands.find(cmd); found != _commands.end()) {
				found->second->onCommand(uc, cmd);
			}
			return;
		}

		if (uc.stack.empty()) {
			return;
		}
		auto cmd = uc.stack.back().command;
		try {
			cmd->onQuery(uc, query->data);
		} catch (...) {
			uc.stack.clear();
			throw;
		}
	});

	printf("Bot username: %s\n", _bot.getApi().getMe()->username.c_str());
	_bot.getApi().deleteWebhook();

	TgLongPoll longPoll(_bot);
	while (true) {
		try {
			printf("Long poll started\n");
			longPoll.start();
		} catch (std::exception& e) {
			printf("error: %s\n", e.what());
		}
	}
}

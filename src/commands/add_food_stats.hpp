#pragma once

#include "bot.hpp"
#include "command.hpp"
#include "db.hpp"
#include "render.hpp"

class Db;
class Bot;

class AddFoodStats : public Command {
  public:
	AddFoodStats(Db& db, Bot& bot) : _db(db), _bot(bot) {}

	static std::string name() { return "add_food_stats"; }

	void onCommand(UserContext& context, const std::string& command) override {
		context.stack.push_back({this, {}});
		_bot.api().sendMessage(context.userInfo->chatId, "➕Введите кКалории:");
	}

	void onNonCommand(UserContext& context, const std::string& nonCommand) override {
		try {
			float value = std::stof(nonCommand);
			_db.addFoodEntry(context.userInfo->id, "food", value, false);
			_bot.api().sendMessage(context.userInfo->chatId, "🆗Значение добавленно");
		} catch (const std::exception& e) {
			_bot.api().sendMessage(context.userInfo->chatId, "⚠️Не удалось прочитать калории");
		}

		assert(!context.stack.empty() && context.stack.back().command == this);
		context.stack.pop_back();

		sendMainMenu(_bot.api(), context);
	}

  private:
	Db& _db;
	Bot& _bot;
};

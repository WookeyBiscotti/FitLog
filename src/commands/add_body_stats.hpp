#pragma once

#include "command.hpp"

#include "bot.hpp"
#include "commands/start.hpp"
#include "render.hpp"

class AddBodyStats : public Command {
	static inline const std::string ADD_WEIGHT = "add_weight";
	static inline const std::string ADD_BICEP_CIRCUMFERENCE = "add_bicep_circumference";

	enum Stage {
		WEIGHT_STAGE = 0,
		BICEP_CIRCUMFERENCE_STAGE = 1,
	};

  public:
	AddBodyStats(Db& db, Bot& bot) : _db(db), _bot(bot) {}

	static std::string name() { return "add_body_stats"; }

	void onCommand(UserContext& context, const std::string& command) override {
		auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
		setButton(keyboard, 0, 0, makeButon("➕⚖️Вес", ADD_WEIGHT));
		setButton(keyboard, 0, 1, makeButon("➕💪Обхват бицепса", ADD_BICEP_CIRCUMFERENCE));
		setButton(keyboard, 0, 2, makeButon("⬅️Отмена", "/" + StartCommand::name()));

		context.stack.push_back({this, {}});
		_bot.api().sendMessage(context.userInfo->chatId, "Что добавить❓", false, 0, keyboard);
	}

	void onQuery(UserContext& context, const std::string& query) override {
		auto& state = context.stack.back().state;
		auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
		setButton(keyboard, 0, 0, makeButon("⬅️Отмена", "/" + StartCommand::name()));
		if (query == ADD_WEIGHT) {
			state["stage"] = WEIGHT_STAGE;
			_bot.api().sendMessage(context.userInfo->chatId, "Введите вес:", false, 0, keyboard);
		} else if (query == ADD_BICEP_CIRCUMFERENCE) {
			state["stage"] = BICEP_CIRCUMFERENCE_STAGE;
			_bot.api().sendMessage(context.userInfo->chatId, "Обхват бицепса:", false, 0, keyboard);
		}
	}

	void onNonCommand(UserContext& context, const std::string& nonCommand) override {
		auto& state = context.stack.back().state;
		int stage = state["stage"];
		if (stage == WEIGHT_STAGE) {
			try {
				float value = std::stof(nonCommand);
				_db.addBodyValue(context.userInfo->id, "weight", value);
				_bot.api().sendMessage(context.userInfo->chatId, "🆗Значение добавленно");
			} catch (const std::exception& e) {
				_bot.api().sendMessage(context.userInfo->chatId, "⚠️Не удалось прочитать вес");
			}
		} else if (stage == BICEP_CIRCUMFERENCE_STAGE) {
			try {
				float value = std::stof(nonCommand);
				_db.addBodyValue(context.userInfo->id, "bicep_circumference", value);
				_bot.api().sendMessage(context.userInfo->chatId, "🆗Значение добавленно");
			} catch (const std::exception& e) {
				_bot.api().sendMessage(context.userInfo->chatId, "⚠️Не удалось прочитать вес");
			}
		}

		assert(!context.stack.empty() && context.stack.back().command == this);
		context.stack.pop_back();

		sendMainMenu(_bot.api(), context);
	}

  private:
	Db& _db;
	Bot& _bot;
};

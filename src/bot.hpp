#pragma once

#include "commands.hpp"
#include "db.hpp"
#include "render.hpp"
#include "token.hpp"

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <sciplot/sciplot.hpp>
#include <tgbot/Bot.h>
#include <tgbot/net/TgLongPoll.h>

#include <algorithm>
#include <csignal>

class Bot {
  public:
	Bot() : _bot(findToken()) {}

	void run() {
		using namespace TgBot;

		_bot.getEvents().onCommand("start", [this](Message::Ptr msg) {
			if (!msg->from) {
				_bot.getApi().sendMessage(msg->chat->id, fmt::format("Не можем установить отправителя."));
			}
			if (!_db.userExist(msg->from->id)) {
				auto user = _db.createUser(msg->from->id, msg->from->username, msg->chat->id);

				_bot.getApi().sendMessage(
					msg->chat->id, fmt::format("Привет, {}! Мы зарегестировали твой аккаунт!", msg->from->username));

				renderMainMenu(*user);
			} else {
				_bot.getApi().sendMessage(
					msg->chat->id, fmt::format("Привет, {}! Твой аккаунт уже зарегестирован!", msg->from->username));

				auto user = _db.getUser(msg->from->id);
				renderMainMenu(*user);
			}
		});

		_bot.getEvents().onAnyMessage([this](Message::Ptr msg) {
			if (StringTools::startsWith(msg->text, "/")) {
				return;
			}
			if (!msg->from) {
				_bot.getApi().sendMessage(msg->chat->id, fmt::format("Не можем установить отправителя."));
			}
			auto user = _db.getUser(msg->from->id);

			if (user->state == UserInfo::NONE) {
				renderMainMenu(*user);
			} else if (user->state == UserInfo::ADDING_WEIGHT_VALUE) {
				try {
					float value = std::stof(msg->text);
					_db.addBodyValue(user->id, "weight", value);
					renderBodyAddWeightAccept(*user);
				} catch (const std::exception& e) {
					renderBodyAddWeightError(*user);
				}
			} else if (user->state == UserInfo::ADDING_FOOD_CALLORIES) {
				try {
					float value = std::stof(msg->text);
					_db.addFoodEntry(user->id, "food", value, false);
					renderAddFoodAccept(*user);
				} catch (const std::exception& e) {
					renderAddFoodError(*user);
				}
			} else if (user->state == UserInfo::ADDING_LIQUID) {
				try {
					float value = std::stof(msg->text);
					_db.addFoodEntry(user->id, "water", value, true);
					renderAddFoodAccept(*user);
				} catch (const std::exception& e) {
					renderAddFoodError(*user);
				}
			}
		});

		_bot.getEvents().onCallbackQuery([this](CallbackQuery::Ptr query) {
			std::shared_ptr<UserInfo> user;
			try {
				user = _db.getUser(query->from->id);
			} catch (...) {
				return;
			}
			try {
				std::string key;
				std::string value;
				auto pos = query->data.find('/');
				if (pos != std::string::npos) {
					key = query->data.substr(0, pos);
					value = query->data.substr(pos + 1);
				} else {
					key = query->data;
				}

				if (key == BODY_INFO_ADD) {
					renderBodyInfoAdd(*user);
				} else if (key == ADD_WEIGHT_VALUE) {
					user->state = UserInfo::ADDING_WEIGHT_VALUE;
					renderBodyAddWeight(*user);
				} else if (key == SHOW_STATS) {
					renderStatsMenu(*user);
				} else if (key == SHOW_WEIGHT_STATS_INTERVAL) {
					renderShowWeightStats(*user);
				} else if (key == SHOW_WEIGHT_STATS) {
					renderBodyStats(*user, "weight", _callbackData[user->id][value]);
				} else if (key == FOOD_ADD) {
					user->state = UserInfo::ADDING_FOOD_CALLORIES;
					renderFoodAdd(*user);
				} else if (key == SHOW_FOOD_STATS_INTERVAL) {
					renderShowFoodStats(*user);
				} else if (key == SHOW_FOOD_STATS) {
					renderFoodStats(*user, "food", _callbackData[user->id][value]);
				}
			} catch (const std::exception& e) {
				renderMainMenu(*user);
			}
		});

		signal(SIGINT, [](int s) {
			printf("SIGINT got\n");
			exit(0);
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

	static TgBot::InlineKeyboardButton::Ptr makeButon(const std::string& label, const std::string& key,
													  std::string value = {}) {
		TgBot::InlineKeyboardButton::Ptr bt(new TgBot::InlineKeyboardButton);
		bt->text = label;
		bt->callbackData = key + "/" + value;

		return bt;
	}

	static void setButton(TgBot::InlineKeyboardMarkup::Ptr keyboard, size_t x, size_t y,
						  TgBot::InlineKeyboardButton::Ptr btn) {
		if (keyboard->inlineKeyboard.size() <= y) {
			keyboard->inlineKeyboard.resize(y + 1);
		}
		if (keyboard->inlineKeyboard[y].size() <= x) {
			keyboard->inlineKeyboard[y].resize(x + 1);
		}
		keyboard->inlineKeyboard[y][x] = btn;
	}

	void renderMainMenu(const UserInfo& user) {
		using namespace TgBot;

		InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
		setButton(keyboard, 0, 0, makeButon("➕ℹ️ Добавить показатель тела", BODY_INFO_ADD));
		setButton(keyboard, 0, 1, makeButon("➕🍔 Добавить съеденую еду", FOOD_ADD));
		// setButton(keyboard, 0, 2, makeButon("➕🥤 Добавить выпитую жидкость", LIQUID_ADD));
		setButton(keyboard, 0, 2, makeButon("⏳📊 Посмотреть статистику", SHOW_STATS));

		_bot.getApi().sendMessage(user.chatId, "Меню", false, 0, keyboard);
	}

	void renderStatsMenu(const UserInfo& user) {
		using namespace TgBot;

		InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
		setButton(keyboard, 0, 0, makeButon("📊⚖️ Веса", SHOW_WEIGHT_STATS_INTERVAL));
		setButton(keyboard, 0, 1, makeButon("📊🍔 Каллорий", SHOW_FOOD_STATS_INTERVAL));

		_bot.getApi().sendMessage(user.chatId, "Какую статистику смотреть❓", false, 0, keyboard);
	}

	void renderBodyInfoAdd(const UserInfo& user) {
		using namespace TgBot;
		InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
		setButton(keyboard, 0, 0, makeButon("➕⚖️ Добавить знечения веса", ADD_WEIGHT_VALUE));

		_bot.getApi().sendMessage(user.chatId, "Что обновить❓", false, 0, keyboard);
	}

	void renderShowWeightStats(const UserInfo& user) {
		using namespace TgBot;

		InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
		setButton(keyboard, 0, 0, makeButon("Неделя", SHOW_WEIGHT_STATS, "7"));
		_callbackData[user.id]["7"] = 7;
		setButton(keyboard, 1, 0, makeButon("Месяц", SHOW_WEIGHT_STATS, "30"));
		_callbackData[user.id]["30"] = 30;
		setButton(keyboard, 0, 1, makeButon("Полгода", SHOW_WEIGHT_STATS, "183"));
		_callbackData[user.id]["183"] = 183;
		setButton(keyboard, 1, 1, makeButon("Год", SHOW_WEIGHT_STATS, "365"));
		_callbackData[user.id]["365"] = 365;

		_bot.getApi().sendMessage(user.chatId, "За какой период❓", false, 0, keyboard);
	}

	void renderShowFoodStats(const UserInfo& user) {
		using namespace TgBot;

		InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
		setButton(keyboard, 0, 0, makeButon("День", SHOW_FOOD_STATS, "1"));
		_callbackData[user.id]["1"] = 1;
		setButton(keyboard, 1, 0, makeButon("Неделя", SHOW_FOOD_STATS, "7"));
		_callbackData[user.id]["7"] = 7;
		setButton(keyboard, 0, 1, makeButon("2 Недели", SHOW_FOOD_STATS, "14"));
		_callbackData[user.id]["14"] = 14;
		setButton(keyboard, 1, 1, makeButon("Месяц", SHOW_FOOD_STATS, "30"));
		_callbackData[user.id]["30"] = 30;
		setButton(keyboard, 0, 2, makeButon("Полгода", SHOW_FOOD_STATS, "183"));
		_callbackData[user.id]["183"] = 183;
		setButton(keyboard, 1, 2, makeButon("Год", SHOW_FOOD_STATS, "365"));
		_callbackData[user.id]["365"] = 365;

		_bot.getApi().sendMessage(user.chatId, "За какой период❓", false, 0, keyboard);
	}

	void renderBodyAddWeight(const UserInfo& user) {
		_bot.getApi().sendMessage(user.chatId, "⚖️ Введите вес:");
	}

	void renderFoodAdd(const UserInfo& user) { _bot.getApi().sendMessage(user.chatId, "➕ Введите кКаллории:"); }

	void renderBodyAddWeightError(const UserInfo& user) {
		_bot.getApi().sendMessage(user.chatId, "⚠️Не удалось прочитать вес");
		renderMainMenu(user);
	}

	void renderBodyAddWeightAccept(const UserInfo& user) {
		_bot.getApi().sendMessage(user.chatId, "🆗Значение добавленно");
		renderMainMenu(user);
	}

	void renderAddFoodError(const UserInfo& user) {
		_bot.getApi().sendMessage(user.chatId, "⚠️Не удалось прочитать ввод");
		renderMainMenu(user);
	}

	void renderAddFoodAccept(const UserInfo& user) {
		_bot.getApi().sendMessage(user.chatId, "🆗Значение добавленно");
		renderMainMenu(user);
	}

	void renderBodyStats(const UserInfo& user, const std::string& name, uint64_t days) {
		using namespace sciplot;
		auto to = std::time(nullptr);
		auto from = to - days * 24 * 3600;
		auto samples = _db.getBodyValues(user.id, name, from, to);

		if (samples.empty()) {
			_bot.getApi().sendMessage(user.chatId, "⚠️Нет данных для отображения");
			renderMainMenu(user);

			return;
		}

		std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

		Vec x(samples.size());
		Vec y(samples.size());
		for (size_t i = 0; i != samples.size(); ++i) {
			x[i] = (samples[i].first - from) / (24.0f * 3600);
			y[i] = samples[i].second;
		}
		Plot2D plot;
		plot.palette("paired");
		plot.xlabel("Дни");
		plot.ylabel("Вес");
		plot.drawCurveFilled(x, y).above().label("Статистика веса").lineWidth(2);
		plot.drawPoints(x, y).label("");
		plot.xtics().interval(0, 1, days);
		plot.xrange(0.0, days);

		Figure fig = {{plot}};
		// Create canvas to hold figure
		Canvas canvas = {{fig}};
		auto filename = "/tmp/" + std::to_string(user.id) + ".png";
		canvas.save(filename);

		_bot.getApi().sendPhoto(user.chatId, TgBot::InputFile::fromFile(filename, "image/png"));
		renderMainMenu(user);
	}

	void renderFoodStats(const UserInfo& user, const std::string& name, uint64_t days) {
		using namespace sciplot;
		auto to = std::time(nullptr);
		auto from = to - days * 24 * 3600;
		auto samples = _db.getFoodEntries(user.id, name, false, from, to);

		if (samples.empty()) {
			_bot.getApi().sendMessage(user.chatId, "⚠️Нет данных для отображения");
			renderMainMenu(user);

			return;
		}

		std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

		Vec x(samples.size());
		Vec y(samples.size());
		for (size_t i = 0; i != samples.size(); ++i) {
			x[i] = (samples[i].first - from) / (24.0f * 3600);
			y[i] = samples[i].second;
		}
		Plot2D plot;
		plot.palette("paired");
		plot.xlabel("Дни");
		plot.ylabel("кКаллории");
		plot.drawCurveFilled(x, y).above().label("Статистика потребления").lineWidth(2);
		plot.xtics().interval(0, 1, days);
		plot.xrange(0.0, days);
		plot.drawPoints(x, y).label("");

		Figure fig = {{plot}};
		// Create canvas to hold figure
		Canvas canvas = {{fig}};
		auto filename = "/tmp/" + std::to_string(user.id) + ".png";
		canvas.save(filename);

		_bot.getApi().sendPhoto(user.chatId, TgBot::InputFile::fromFile(filename, "image/png"));
		renderMainMenu(user);
	}

  private:
	TgBot::Bot _bot;
	Db _db;
	std::unordered_map<int64_t, std::unordered_map<std::string, nlohmann::json>> _callbackData;
};

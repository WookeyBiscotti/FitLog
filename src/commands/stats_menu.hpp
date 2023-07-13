#pragma once

#include "command.hpp"

#include "bot.hpp"
#include "commands/start.hpp"
#include "db.hpp"
#include "render.hpp"
#include "time_utils.hpp"

#include <sciplot/sciplot.hpp>

class StatsMenu : public Command {
	static inline const std::string SHOW_WEIGHT_STATS_INTERVAL = "show_weight_stats_interval";
	static inline const std::string SHOW_FOOD_STATS_INTERVAL = "show_food_stats_interval";
	static inline const std::string SHOW_BICEP_CIRCUMFERENCE_STATS_INTERVAL = "show_bicep_circumference_stats_interval";

	enum Stage {
		MAIN = 0,
		FOOD_INTERVAL = 1,
		WEIGHT_INTERVAL = 2,
		BICEP_CIRCUMFERENCE = 3,
	};

  public:
	StatsMenu(Db& db, Bot& bot) : _db(db), _bot(bot) {}

	static std::string name() { return "stats_menu"; }

	void onCommand(UserContext& context, const std::string& command) override {
		auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
		setButton(keyboard, 0, 0, makeButon("📊⚖️Веса", SHOW_WEIGHT_STATS_INTERVAL));
		setButton(keyboard, 0, 1, makeButon("📊🍔Калорий", SHOW_FOOD_STATS_INTERVAL));
		setButton(keyboard, 0, 2, makeButon("📊💪Охват бицепса", SHOW_BICEP_CIRCUMFERENCE_STATS_INTERVAL));
		setButton(keyboard, 0, 3, makeButon("⬅️Отмена", "/" + StartCommand::name()));

		context.stack.push_back({this, {}});
		auto& state = context.stack.back().state;
		state["stage"] = MAIN;

		_bot.api().sendMessage(context.userInfo->chatId, "Какую статистику смотреть❓", false, 0, keyboard);
	}

	void onQuery(UserContext& context, const std::string& query) override {
		auto& state = context.stack.back().state;
		int stage = state["stage"];

		if (stage == MAIN) {
			auto keyboard = std::make_shared<TgBot::InlineKeyboardMarkup>();
			auto& values = state["values"];
			if (query == SHOW_WEIGHT_STATS_INTERVAL) {
				state["stage"] = WEIGHT_INTERVAL;
			} else if (query == SHOW_FOOD_STATS_INTERVAL) {
				state["stage"] = FOOD_INTERVAL;
			} else if (query == SHOW_BICEP_CIRCUMFERENCE_STATS_INTERVAL) {
				state["stage"] = BICEP_CIRCUMFERENCE;
			}
			setButton(keyboard, 0, 0, makeButon("День", "1"));
			values["1"] = 1;
			setButton(keyboard, 1, 0, makeButon("Неделя", "7"));
			values["7"] = 7;
			setButton(keyboard, 0, 1, makeButon("2 Недели", "14"));
			values["14"] = 14;
			setButton(keyboard, 1, 1, makeButon("Месяц", "30"));
			values["30"] = 30;
			setButton(keyboard, 0, 2, makeButon("Полгода", "183"));
			values["183"] = 183;
			setButton(keyboard, 1, 2, makeButon("Год", "365"));
			values["365"] = 365;
			setButton(keyboard, 0, 3, makeButon("⬅️Отмена", "/" + StartCommand::name()));
			
			_bot.api().sendMessage(context.userInfo->chatId, "За какой период❓", false, 0, keyboard);
		} else {
			using namespace sciplot;

			auto& values = state["values"];
			int days = values[query];

			context.stack.pop_back();

			auto to = std::time(nullptr);
			auto from = to - days * 24 * 3600;

			std::vector<std::pair<uint64_t, float>> samples;
			std::string xAxis;
			std::string yAxis;

			void (*plotFn)(const std::string& filename, const std::string& xAxis, const std::string& yAxis,
						   const std::valarray<uint64_t>& ts, const std::valarray<double>& values);

			if (stage == WEIGHT_INTERVAL) {
				samples = _db.getBodyValues(context.userInfo->id, "weight", from, to);
				xAxis = "Дни";
				xAxis = "Вес";
				plotFn = &savePlot;
			} else if (stage == FOOD_INTERVAL) {
				samples = _db.getFoodEntriesAll(context.userInfo->id, false, from, to);
				xAxis = "Дни";
				xAxis = "кКалории";
				if (days > 2) {
					plotFn = &savePlotFromSeriesSumByDay;
				} else {
					plotFn = &savePlotFromSeries;
				}
			} else if (stage == BICEP_CIRCUMFERENCE) {
				samples = _db.getBodyValues(context.userInfo->id, "bicep_circumference", from, to);
				xAxis = "Дни";
				xAxis = "Охват";
				plotFn = &savePlot;
			}
			if (samples.empty()) {
				_bot.api().sendMessage(context.userInfo->chatId,
									   "⚠️Нет данных для отображения");
				sendMainMenu(_bot.api(), context);

				return;
			}
			std::valarray<uint64_t> x(samples.size());
			Vec y(samples.size());
			for (size_t i = 0; i != samples.size(); ++i) {
				x[i] = samples[i].first;
				y[i] = samples[i].second;
			}

			const auto filename = "/tmp/" + std::to_string(context.userInfo->id) + ".png";
			plotFn(filename, xAxis, yAxis, x, y);

			_bot.api().sendPhoto(context.userInfo->chatId, TgBot::InputFile::fromFile(filename, "image/png"));
			sendMainMenu(_bot.api(), context);
		}
	}

  private:
	Db& _db;
	Bot& _bot;
};

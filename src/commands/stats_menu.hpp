#pragma once

#include "command.hpp"

#include "bot.hpp"
#include "db.hpp"
#include "render.hpp"

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
			_bot.api().sendMessage(context.userInfo->chatId, "За какой период❓", false, 0, keyboard);
		} else if (stage == WEIGHT_INTERVAL) {
			auto& values = state["values"];
			int days = values[query];

			context.stack.pop_back();

			using namespace sciplot;
			auto to = std::time(nullptr);
			auto from = to - days * 24 * 3600;
			auto samples = _db.getBodyValues(context.userInfo->id, "weight", from, to);

			if (samples.empty()) {
				_bot.api().sendMessage(context.userInfo->chatId,
									   "⚠️Нет данных для отображения");
				sendMainMenu(_bot.api(), context);

				return;
			}

			std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

			Vec x(samples.size());
			Vec y(samples.size());
			int minW = 1000;
			int maxW = -1;
			for (size_t i = 0; i != samples.size(); ++i) {
				x[i] = (samples[i].first - from) / (24.0f * 3600);
				y[i] = samples[i].second;
				if (y[i] > maxW) {
					maxW = y[i];
				}
				if (y[i] < minW) {
					minW = y[i];
				}
			}
			Plot2D plot;
			plot.palette("paired");
			plot.xlabel("Дни").fontSize(7);
			plot.ylabel("Вес").fontSize(7);
			plot.drawCurve(x, y).label("");
			plot.xrange(0.0, days);
			if (maxW != minW) {
				plot.ytics().fontSize(7).interval(minW, (maxW - minW) / 10.0f, maxW);
				plot.yrange(minW, maxW);
			} else {
				plot.ytics().fontSize(7).interval(minW - 1, 1, maxW + 1);
				plot.yrange(minW - 1, maxW + 1);
			}
			plot.legend().hide();
			plot.grid().show();

			Figure fig = {{plot}};
			Canvas canvas = {{fig}};
			auto filename = "/tmp/" + std::to_string(context.userInfo->id) + ".png";
			canvas.save(filename);

			_bot.api().sendPhoto(context.userInfo->chatId, TgBot::InputFile::fromFile(filename, "image/png"));
			sendMainMenu(_bot.api(), context);
		} else if (stage == FOOD_INTERVAL) {
			auto& values = state["values"];
			int days = values[query];

			context.stack.pop_back();

			using namespace sciplot;
			auto to = std::time(nullptr);
			auto from = to - days * 24 * 3600;
			auto samples = _db.getFoodEntries(context.userInfo->id, "food", false, from, to);

			if (samples.empty()) {
				_bot.api().sendMessage(context.userInfo->chatId,
									   "⚠️Нет данных для отображения");
				sendMainMenu(_bot.api(), context);

				return;
			}

			std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

			Vec x(samples.size());
			Vec y(samples.size());
			int minW = 1000;
			int maxW = -1;
			for (size_t i = 0; i != samples.size(); ++i) {
				x[i] = (samples[i].first - from) / (24.0f * 3600);
				y[i] = samples[i].second;
				if (y[i] > maxW) {
					maxW = y[i];
				}
				if (y[i] < minW) {
					minW = y[i];
				}
			}
			Plot2D plot;
			plot.palette("paired");
			plot.xlabel("Дни").fontSize(7);
			plot.ylabel("кКалории").fontSize(7);
			plot.drawImpulses(x, y).label("");
			plot.xtics().fontSize(7).interval(0, std::max(1, days / 7), days);
			plot.xrange(0.0, days);
			// plot.ytics().fontSize(7).interval(minW, 1, maxW);
			plot.ytics().fontSize(7);
			plot.yrange(minW, maxW);
			plot.legend().hide();
			plot.grid().show();

			Figure fig = {{plot}};
			Canvas canvas = {{fig}};
			auto filename = "/tmp/" + std::to_string(context.userInfo->id) + ".png";
			canvas.save(filename);

			_bot.api().sendPhoto(context.userInfo->chatId, TgBot::InputFile::fromFile(filename, "image/png"));
			sendMainMenu(_bot.api(), context);
		} else if (stage == BICEP_CIRCUMFERENCE) {
			auto& values = state["values"];
			int days = values[query];

			context.stack.pop_back();

			using namespace sciplot;
			auto to = std::time(nullptr);
			auto from = to - days * 24 * 3600;
			auto samples = _db.getBodyValues(context.userInfo->id, "bicep_circumference", from, to);

			if (samples.empty()) {
				_bot.api().sendMessage(context.userInfo->chatId,
									   "⚠️Нет данных для отображения");
				sendMainMenu(_bot.api(), context);

				return;
			}

			std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

			Vec x(samples.size());
			Vec y(samples.size());
			int minW = 100000;
			int maxW = -1;
			for (size_t i = 0; i != samples.size(); ++i) {
				x[i] = (samples[i].first - from) / (24.0f * 3600);
				y[i] = samples[i].second;
				if (y[i] > maxW) {
					maxW = y[i];
				}
				if (y[i] < minW) {
					minW = y[i];
				}
			}
			Plot2D plot;
			plot.palette("paired");
			plot.xlabel("Дни").fontSize(7);
			plot.ylabel("Охват").fontSize(7);
			plot.drawCurveFilled(x, y).label("").above();
			plot.xtics().fontSize(7).interval(0, std::max(1, days / 7), days);
			plot.xrange(0.0, days);
			if (maxW != minW) {
				plot.ytics().fontSize(7).interval(minW, (maxW - minW) / 10.0f, maxW);
				plot.yrange(minW, maxW);
			} else {
				plot.ytics().fontSize(7).interval(minW - 1, 1, maxW + 1);
				plot.yrange(minW - 1, maxW + 1);
			}
			plot.legend().hide();
			plot.grid().show();

			Figure fig = {{plot}};
			Canvas canvas = {{fig}};
			auto filename = "/tmp/" + std::to_string(context.userInfo->id) + ".png";
			canvas.save(filename);

			_bot.api().sendPhoto(context.userInfo->chatId, TgBot::InputFile::fromFile(filename, "image/png"));
			sendMainMenu(_bot.api(), context);
		}
	}

  private:
	Db& _db;
	Bot& _bot;
};

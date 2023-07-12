#include "render.hpp"
#include "commands/add_body_stats.hpp"
#include "commands/add_food_stats.hpp"
#include "commands/start.hpp"
#include "commands/stats_menu.hpp"

#include <date/date.h>
#include <date/tz.h>
#include <sciplot/sciplot.hpp>

TgBot::InlineKeyboardButton::Ptr makeButon(const std::string& label, const std::string& key) {
	TgBot::InlineKeyboardButton::Ptr bt(new TgBot::InlineKeyboardButton);
	bt->text = label;
	bt->callbackData = key;

	return bt;
}

void setButton(TgBot::InlineKeyboardMarkup::Ptr keyboard, size_t x, size_t y, TgBot::InlineKeyboardButton::Ptr btn) {
	if (keyboard->inlineKeyboard.size() <= y) {
		keyboard->inlineKeyboard.resize(y + 1);
	}
	if (keyboard->inlineKeyboard[y].size() <= x) {
		keyboard->inlineKeyboard[y].resize(x + 1);
	}
	keyboard->inlineKeyboard[y][x] = btn;
}

void sendMainMenu(const TgBot::Api& api, const UserContext& userContext) {
	using namespace TgBot;

	InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
	setButton(keyboard, 0, 0,
			  makeButon("➕ℹ️ Добавить показатель тела", "/" + AddBodyStats::name()));
	setButton(keyboard, 0, 1, makeButon("➕🍔 Добавить съеденую еду", "/" + AddFoodStats::name()));
	// setButton(keyboard, 0, 2, makeButon("➕🥤 Добавить выпитую жидкость", LIQUID_ADD));
	setButton(keyboard, 0, 2, makeButon("⏳📊 Посмотреть статистику", "/" + StatsMenu::name()));

	api.sendMessage(userContext.userInfo->chatId, "Меню", false, 0, keyboard);
}

// void sendAddBodyWeightError(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.userInfo->chatId, "⚠️Не удалось прочитать вес");
// }

// void sendAddBodyWeightAccept(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.userInfo->chatId, "🆗Значение добавленно");
// }

// void sendStatsMenu(const TgBot::Api& api, const UserContext& userContext) {
// 	using namespace TgBot;

// 	InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
// 	setButton(keyboard, 0, 0, makeButon("📊⚖️ Веса", SHOW_WEIGHT_STATS_INTERVAL));
// 	setButton(keyboard, 0, 1, makeButon("📊🍔 Каллорий", SHOW_FOOD_STATS_INTERVAL));

// 	api.sendMessage(userContext.userInfo->chatId, "Какую статистику смотреть❓", false, 0, keyboard);
// }

// void renderShowWeightStats(const TgBot::Api& api, const UserContext& userContext) {
// 	using namespace TgBot;

// 	InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
// 	setButton(keyboard, 0, 0, makeButon("Неделя", SHOW_WEIGHT_STATS, "7"));
// 	_callbackData[userContext.id]["7"] = 7;
// 	setButton(keyboard, 1, 0, makeButon("Месяц", SHOW_WEIGHT_STATS, "30"));
// 	_callbackData[userContext.id]["30"] = 30;
// 	setButton(keyboard, 0, 1, makeButon("Полгода", SHOW_WEIGHT_STATS, "183"));
// 	_callbackData[userContext.id]["183"] = 183;
// 	setButton(keyboard, 1, 1, makeButon("Год", SHOW_WEIGHT_STATS, "365"));
// 	_callbackData[userContext.id]["365"] = 365;

// 	api.sendMessage(userContext.chatId, "За какой период❓", false, 0, keyboard);
// }

// void renderShowFoodStats(const TgBot::Api& api, const UserContext& userContext) {
// 	using namespace TgBot;

// 	InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
// 	setButton(keyboard, 0, 0, makeButon("День", SHOW_FOOD_STATS, "1"));
// 	_callbackData[userContext.id]["1"] = 1;
// 	setButton(keyboard, 1, 0, makeButon("Неделя", SHOW_FOOD_STATS, "7"));
// 	_callbackData[userContext.id]["7"] = 7;
// 	setButton(keyboard, 0, 1, makeButon("2 Недели", SHOW_FOOD_STATS, "14"));
// 	_callbackData[userContext.id]["14"] = 14;
// 	setButton(keyboard, 1, 1, makeButon("Месяц", SHOW_FOOD_STATS, "30"));
// 	_callbackData[userContext.id]["30"] = 30;
// 	setButton(keyboard, 0, 2, makeButon("Полгода", SHOW_FOOD_STATS, "183"));
// 	_callbackData[userContext.id]["183"] = 183;
// 	setButton(keyboard, 1, 2, makeButon("Год", SHOW_FOOD_STATS, "365"));
// 	_callbackData[userContext.id]["365"] = 365;

// 	api.sendMessage(userContext.chatId, "За какой период❓", false, 0, keyboard);
// }

void sendAddBodyWeight(const TgBot::Api& api, Command& command, UserContext& userContext) {
	userContext.stack.push_back({&command, nlohmann::json{}});
	api.sendMessage(userContext.userInfo->chatId, "⚖️ Введите вес:");
}

// void renderFoodAdd(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.chatId, "➕ Введите кКаллории:");
// }

// void renderBodyAddWeightError(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.chatId, "⚠️Не удалось прочитать вес");
// 	renderMainMenu(userContext);
// }

// void renderBodyAddWeightAccept(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.chatId, "🆗Значение добавленно");
// 	renderMainMenu(userContext);
// }

// void renderAddFoodError(const TgBot::Api& api, const UserContext& userContext) {
// 	api.sendMessage(userContext.chatId, "⚠️Не удалось прочитать ввод");
// 	renderMainMenu(userContext);
// }

// void renderAddFoodAccept(const TgBot::Api& api, const UserInfo& userContext) {
// 	api.sendMessage(userContext.chatId, "🆗Значение добавленно");
// 	renderMainMenu(userContext);
// }

// void renderBodyStats(const TgBot::Api& api, const UserInfo& userContext, const std::string& name, uint64_t days) {
// 	using namespace sciplot;
// 	auto to = std::time(nullptr);
// 	auto from = to - days * 24 * 3600;
// 	auto samples = _db.getBodyValues(userContext.id, name, from, to);

// 	if (samples.empty()) {
// 		api.sendMessage(userContext.chatId, "⚠️Нет данных для отображения");
// 		renderMainMenu(userContext);

// 		return;
// 	}

// 	std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

// 	Vec x(samples.size());
// 	Vec y(samples.size());
// 	for (size_t i = 0; i != samples.size(); ++i) {
// 		x[i] = (samples[i].first - from) / (24.0f * 3600);
// 		y[i] = samples[i].second;
// 	}
// 	Plot2D plot;
// 	plot.palette("paired");
// 	plot.xlabel("Дни");
// 	plot.ylabel("Вес");
// 	plot.drawCurveFilled(x, y).above().label("Статистика веса").lineWidth(2);
// 	plot.drawPoints(x, y).label("");
// 	plot.xtics().interval(0, 1, days);
// 	plot.xrange(0.0, days);

// 	Figure fig = {{plot}};
// 	// Create canvas to hold figure
// 	Canvas canvas = {{fig}};
// 	auto filename = "/tmp/" + std::to_string(userContext.id) + ".png";
// 	canvas.save(filename);

// 	api.sendPhoto(userContext.chatId, TgBot::InputFile::fromFile(filename, "image/png"));
// 	renderMainMenu(userContext);
// }

// void renderFoodStats(const TgBot::Api& api, const UserInfo& userContext, const std::string& name, uint64_t days) {
// 	using namespace sciplot;
// 	auto to = std::time(nullptr);
// 	auto from = to - days * 24 * 3600;
// 	auto samples = _db.getFoodEntries(userContext.id, name, false, from, to);

// 	if (samples.empty()) {
// 		api.sendMessage(userContext.chatId, "⚠️Нет данных для отображения");
// 		renderMainMenu(userContext);

// 		return;
// 	}

// 	std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

// 	Vec x(samples.size());
// 	Vec y(samples.size());
// 	for (size_t i = 0; i != samples.size(); ++i) {
// 		x[i] = (samples[i].first - from) / (24.0f * 3600);
// 		y[i] = samples[i].second;
// 	}
// 	Plot2D plot;
// 	plot.palette("paired");
// 	plot.xlabel("Дни");
// 	plot.ylabel("кКаллории");
// 	plot.drawCurveFilled(x, y).above().label("Статистика потребления").lineWidth(2);
// 	plot.xtics().interval(0, 1, days);
// 	plot.xrange(0.0, days);
// 	plot.drawPoints(x, y).label("");

// 	Figure fig = {{plot}};
// 	// Create canvas to hold figure
// 	Canvas canvas = {{fig}};
// 	auto filename = "/tmp/" + std::to_string(userContext.id) + ".png";
// 	canvas.save(filename);

// 	api.sendPhoto(userContext.chatId, TgBot::InputFile::fromFile(filename, "image/png"));
// 	renderMainMenu(userContext);
// }

void savePlotFromSeriesSum(const std::string& filename, const std::string& xAxis, const std::string& yAxis,
						   const std::valarray<uint64_t>& ts, const std::valarray<double>& values) {
	using namespace sciplot;
	using namespace std::chrono;

	constexpr unsigned int secsInDay = 3600 * 24;

	auto minTs = ts.min();
	auto maxTS = ts.max();

	uint64_t daysTotal = 1 + (maxTS - minTs) / secsInDay;

	auto zone = date::locate_zone("Europe/Moscow");
	auto t = date::make_zoned(zone, date::sys_time<seconds>(seconds(static_cast<long>(minTs))));
	auto startDay = floor<date::days>(t.get_local_time());
	// auto seconds = floor<date::w>(startDay);
	auto startTs = seconds(startDay.time_since_epoch());

	std::valarray<double> x = linspace(0, daysTotal, daysTotal);
	std::valarray<double> y(daysTotal);
	Strings names(daysTotal);

	for (size_t i = 0; i != values.size(); ++i) {
		auto idx = (ts[i] - minTs) / secsInDay;
		y[idx] += values[i];
	}
	for (auto i = 0; i != daysTotal; ++i) {
		names[i] = format(std::locale("ru_RU.utf8"), "%a", (date::year_month_weekday{startDay}.weekday() + date::days(i)));
	}

	Plot2D plot;
	plot.palette("paired");
	plot.xlabel(xAxis).fontSize(7);
	plot.ylabel(yAxis).fontSize(7);
	plot.drawBoxes(names, y).label("");
	plot.xtics().fontSize(7).interval(0, std::max<double>(1, daysTotal / 10.0f), daysTotal);
	plot.xrange(0.0, daysTotal);
	plot.ytics().fontSize(7);
	plot.legend().hide();
	plot.grid().show();

	Figure fig = {{plot}};
	Canvas canvas = {{fig}};
	canvas.save(filename);
}

#pragma once

#include "user_context.hpp"

#include <tgbot/Api.h>


TgBot::InlineKeyboardButton::Ptr makeButon(const std::string& label, const std::string& key);

void setButton(TgBot::InlineKeyboardMarkup::Ptr keyboard, size_t x, size_t y, TgBot::InlineKeyboardButton::Ptr btn);

void sendMainMenu(const TgBot::Api& api, const UserContext& userContext);

void sendAddBodyStatsMenu(const TgBot::Api& api, const UserContext& userContext);

void sendStatsMenu(const TgBot::Api& api, const UserContext& userContext);

void renderShowWeightStats(const TgBot::Api& api, const UserContext& userContext);

void renderShowFoodStats(const TgBot::Api& api, const UserContext& userContext);

void sendAddBodyWeight(const TgBot::Api& api, Command& command, UserContext& userContext);

void renderFoodAdd(const TgBot::Api& api, const UserContext& userContext);

void sendAddBodyWeightError(const TgBot::Api& api, const UserContext& userContext);
void sendAddBodyWeightAccept(const TgBot::Api& api, const UserContext& userContext);

void renderAddFoodError(const TgBot::Api& api, const UserContext& userContext);
void renderAddFoodAccept(const TgBot::Api& api, const UserContext& userContext);

void renderBodyStats(const TgBot::Api& api, const UserContext& userContext, const std::string& name, uint64_t days);
void renderFoodStats(const TgBot::Api& api, const UserContext& userContext, const std::string& name, uint64_t days);

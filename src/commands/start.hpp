#pragma once

#include "bot.hpp"
#include "command.hpp"
#include "render.hpp"

class Db;
class Bot;

class StartCommand : public Command {
  public:
	StartCommand(Db& db, Bot& bot) : _db(db), _bot(bot) {}

	static std::string name() { return "start"; }

	void onCommand(UserContext& context, const std::string& command) override { sendMainMenu(_bot.api(), context); }

  private:
	Db& _db;
	Bot& _bot;
};

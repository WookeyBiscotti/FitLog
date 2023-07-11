#pragma once

#include "command.hpp"

class DB;
class Bot;

class StartCommand : public Command {
  public:
	StartCommand(DB& db, Bot& bot) : _db(db), _bot(bot) {}

	static std::string name() { return "start"; }

	void onCommand(UserContext& context, const std::string& command) override;

  private:
	DB& _db;
	Bot& _bot;
};

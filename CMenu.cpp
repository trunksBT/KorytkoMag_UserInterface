#include "UserInterface/CMenu.hpp"
#include <UserInterface/CtrlCommandsValidators/AlmagCommandValidationManager.hpp>
#include <UserInterface/CtrlCommandsValidators/DatabaseCommandValidationManager.hpp>
#include <CommandPattern/AlmagControllerNull.hpp>
#include <UserInterface/Database/CDatabaseCommand.hpp>
#include <PluginConstraints/DatabaseConstraints.hpp> // TO REMOVE
#include <Utils/Functions.hpp>
#include <Utils/Utils.hpp>
#include <Utils/Logger.hpp>

using namespace defaultVals;
using namespace ui;
using namespace constraints;

CMenu::CMenu(
   const std::string&  inMenuName, const std::string& inCommandName,
   Database& inDB, IAlmagControllerPtr almagCtrl)
   : db_(inDB)
   , almagCtrl_(almagCtrl)
   , almagCmdValidationMgr_{std::make_unique<AlmagCommandValidationManager>(db_)}
   , databaseCmdValidationMgr_{std::make_unique<DatabaseCommandValidationManager>(db_)}
{}

bool CMenu::run(const Strings& inArgs)
{
   ReturnCode finalResultCode = true;

	while (finalResultCode)
	{
	   Strings userInput = parser_.receiveAndLex();
      finalResultCode &= runImpl(userInput);
	}
	return finalResultCode;
}

bool CMenu::runPredefinedCommands(const StringsMatrix& inCommands)
{
   LOG(info) << "Start";
   ReturnCode finalResultCode = true;

	for (const auto& it : inCommands)
	{
      finalResultCode &= runImpl(it);
	}
   LOG(info) << "End";
	return finalResultCode;
}

ReturnCode CMenu::runImpl(const Strings& userInput)
{
	if (userInput.size() == 0)
	{
      LOG(error) << "Empty user input";
		LOG(debug) << actionHelp();
		return false;
	}
	std::string receivedCmd = userInput[idx::COMMAND_OR_ACTION_NAME];

   if (funs::anyOf(databaseCommandConstraints_, receivedCmd))
   {
      LOG(info) << receivedCmd;
		return interpretDatabaseCommand(userInput);
   }
   else if (funs::anyOf(almagCommandConstraints_, receivedCmd))
   {
      LOG(info) << receivedCmd;
		return interpretControllerCommand(userInput);
   }
   else if (actions::HELP == receivedCmd)
	{
	   LOG(info) << actionHelp();
	   return true;
	}
   else if (actions::EXIT == receivedCmd)
   {
      return false;
   }
   LOG(info) << receivedCmd << actions::HELP_WHEN_UNKNOWN;
	return true;
}

ReturnCode CMenu::interpretControllerCommand(const Strings& userInput)
{
   LOG(debug) << "Start";
   if (const auto validatedUserInput = almagCmdValidationMgr_->perform(userInput))
   {
      almagCtrl_->addCommands({*validatedUserInput});
      return almagCtrl_->executeCommand();
   }
   LOG(warning) << "Validation rejected the command";
   return true;
}

ReturnCode CMenu::interpretDatabaseCommand(const Strings& userInput)
{
   LOG(debug) << "Start";
   if (const auto validatedUserInput = databaseCmdValidationMgr_->perform(userInput))
   {
      CDatabaseCommand updateDatabase(db_, userInput);
      return updateDatabase.runCommand();
   }
   LOG(warning) << "Validation rejected the command";
   return true;
}

void CMenu::setAlmagCommandsConstraints(const Strings& constraints)
{
   almagCommandConstraints_ = constraints;
}

void CMenu::setDatabaseCommandsConstraints(const Strings& constraints)
{
   databaseCommandConstraints_ = constraints;
}

#include <TriggerFlag.h>

namespace CmdLineParser
{
	TriggerFlag::TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagRequired)
	{}

	TriggerFlag::TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagRequired), _triggeredFunc(&triggeredFunc)
	{}

	TriggerFlag::TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg,
		bool argRequired, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagArg, argRequired, flagRequired), _triggeredFunc(&triggeredFunc)
	{}

	TriggerFlag& TriggerFlag::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;

		return *this;
	}

	void TriggerFlag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

		Flag::Raise(itr, end);
	}

	bool TriggerFlag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		try
		{
			_triggeredFunc->Run();
		}
		catch (std::exception& e)
		{
			if (errorMsg)
				*errorMsg = e.what();

			return false;
		}

		return Flag::TryRaise(itr, end, errorMsg);
	}
}
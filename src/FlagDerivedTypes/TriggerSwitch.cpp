#include <TriggerSwitch.h>

namespace CmdLineParser
{
	TriggerSwitch::TriggerSwitch(Tokens&& flagTokens, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagRequired),
		SwitchFlag(std::move(flagTokens), std::move(flagDesc), defaultSwitchState, flagRequired),
		TriggerFlag(std::move(flagTokens), std::move(flagDesc), flagRequired)
	{}

	TriggerSwitch::TriggerSwitch(Tokens&& flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagRequired),
		SwitchFlag(std::move(flagTokens), std::move(flagDesc), defaultSwitchState, flagRequired),
		TriggerFlag(std::move(flagTokens), std::move(flagDesc), triggeredFunc, flagRequired)
	{}

	TriggerSwitch& TriggerSwitch::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;

		return *this;
	}

	void TriggerSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

		SwitchFlag::Raise(itr, end);
	}

	bool TriggerSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		try
		{
			_triggeredFunc->Run();
		}
		catch (const std::exception& e)
		{
			if (errorMsg)
				*errorMsg = e.what();

			return false;
		}

		return SwitchFlag::TryRaise(itr, end, errorMsg);
	}
}
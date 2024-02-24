#include <BranchTriggerSwitch.h>

namespace CmdLineParser
{
	BranchTriggerSwitch::BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagToken), std::move(flagDesc), flagRequired),
		  BranchFlag(std::move(flagToken), std::move(flagDesc), flagRequired),
		  TriggerSwitch(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired)
	{}

	BranchTriggerSwitch& BranchTriggerSwitch::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;

		return *this;
	}

	void BranchTriggerSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

		switchState.SetDefaultValue(true);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchTriggerSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		try
		{
			_triggeredFunc->Run();

			switchState.SetDefaultValue(true);
		}
		catch (const std::exception& e)
		{
			if (errorMsg)
				*errorMsg = e.what();

			return false;
		}

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}
}
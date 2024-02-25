#include <BranchSwitch.h>

namespace CmdLineParser
{
	BranchSwitch::BranchSwitch(std::string&& flagToken, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagToken), std::move(flagDesc), flagRequired),
		  BranchFlag(std::move(flagToken), std::move(flagDesc), flagRequired),
		  SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired)
	{}

	void BranchSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		SwitchFlag::Raise(itr, end);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		SwitchFlag::TryRaise(itr, end, errorMsg);

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}
}
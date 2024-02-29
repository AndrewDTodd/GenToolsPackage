#include <BranchTrigger.h>

namespace CmdLineParser
{
	BranchTrigger::BranchTrigger(std::string&& flagToken, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
		  BranchFlag(std::move(flagToken), std::move(flagDesc), flagRequired),
		  TriggerFlag(std::move(flagToken), std::move(flagDesc), flagRequired)
	{}

	BranchTrigger& BranchTrigger::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;

		return *this;
	}

	void BranchTrigger::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

		Flag::Raise(itr, end);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchTrigger::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		Flag::TryRaise(itr, end, errorMsg);

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}
}
#include <BranchFlag.h>

namespace CmdLineParser
{
	BranchFlag::BranchFlag(std::string&& flagToken, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired)
	{}

	void BranchFlag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		Flag::Raise(itr, end);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchFlag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
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
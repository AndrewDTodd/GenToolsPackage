#ifndef BRANCH_SWITCH_FLAG_H
#define BRANCH_SWITCH_FLAG_H

#include <Flag.h>
#include <FlagSubTypes/SwitchFlag.h>

namespace CmdLineParser
{
	class BranchSwitch : public SwitchFlag
	{
	protected:
		std::vector<Flag> _nestedFlags;

	public:
		BranchSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchSwitch(std::string&& flagToken, std::string&& flagDesc, Flags&&... subFlags,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
		}

		template<FlagType... Flags>
		BranchSwitch& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchSwitch(const BranchSwitch&) = delete;
		BranchSwitch(BranchSwitch&&) = delete;

		BranchSwitch& operator=(const BranchSwitch&) = delete;
		BranchSwitch& operator=(BranchSwitch&&) = delete;
	};
}
#endif // !BRANCH_SWITCH_FLAG_H

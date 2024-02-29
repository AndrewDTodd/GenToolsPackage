#ifndef BRANCH_TRIGGER_SWITCH_FLAG_H
#define BRANCH_TRIGGER_SWITCH_FLAG_H

#include <BranchFlag.h>
#include <TriggerSwitch.h>

namespace CmdLineParser
{
	class BranchTriggerSwitch : public BranchFlag, public TriggerSwitch
	{
	public:
		BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, Flags&&... subFlags,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
			  BranchFlag(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerSwitch(std::move(flagToken), std::move(flagDesc), triggeredFunc, defaultSwitchState, flagRequired)
		{
		}

		template<FlagType... Flags>
		BranchTriggerSwitch& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTriggerSwitch& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchTriggerSwitch(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch(BranchTriggerSwitch&&) = delete;

		BranchTriggerSwitch& operator=(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch& operator=(BranchTriggerSwitch&&) = delete;
	};
}
#endif // !BRANCH_TRIGGER_SWITCH_FLAG_H

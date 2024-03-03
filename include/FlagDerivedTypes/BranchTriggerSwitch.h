#ifndef BRANCH_TRIGGER_SWITCH_FLAG_H
#define BRANCH_TRIGGER_SWITCH_FLAG_H

#include <BranchFlag.h>
#include <TriggerSwitch.h>

namespace CmdLineParser
{
	template<IsFlagEvent Flag_Event>
	class BranchTriggerSwitch : public BranchFlag<Arg_Bool>, public TriggerSwitch<Flag_Event>
	{
	public:
		BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Arg_Bool>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
			BranchFlag<Arg_Bool>(std::move(flagToken), std::move(flagDesc), flagRequired),
			TriggerSwitch<Flag_Event>(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired)
		{}

		template<FlagType... Flags>
		explicit BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc, const Flag_Event&& triggeredFunc, Flags&&... subFlags,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Arg_Bool>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
			  BranchFlag<Arg_Bool>(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerSwitch<Flag_Event>(std::move(flagToken), std::move(flagDesc), std::move(triggeredFunc), defaultSwitchState, flagRequired)
		{}

		template<FlagType... Flags>
		BranchTriggerSwitch& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTriggerSwitch& SetFlagEvent(const Flag_Event&& triggeredFunc) noexcept
		{
			const_cast<Flag_Event&>(_triggeredFunc) = std::move(triggeredFunc);

			_triggerSet = true;

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			_triggeredFunc.Run();

			SwitchFlag::Raise(itr, end);

			for (auto& subFlag : _nestedFlags)
			{
				subFlag.Raise(itr, end);
			}
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept override
		{
			if (!_triggeredFunc.TryRun(errorMsg))
				return false;

			SwitchFlag::TryRaise(itr, end, errorMsg);

			for (auto& subFlag : _nestedFlags)
			{
				if (!subFlag.TryRaise(itr, end, errorMsg))
					return false;
			}

			return true;
		}

		BranchTriggerSwitch(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch(BranchTriggerSwitch&&) = delete;

		BranchTriggerSwitch& operator=(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch& operator=(BranchTriggerSwitch&&) = delete;
	};
}
#endif // !BRANCH_TRIGGER_SWITCH_FLAG_H

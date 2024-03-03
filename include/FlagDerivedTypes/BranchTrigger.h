#ifndef BRANCH_TRIGGER_FLAG_H
#define BRANCH_TRIGGER_FLAG_H

#include <BranchFlag.h>
#include <TriggerFlag.h>

namespace CmdLineParser
{
	template<IsFlagArgument Flag_Argument, IsFlagEvent Flag_Event>
	class BranchTrigger : public BranchFlag<Flag_Argument>, public TriggerFlag<Flag_Argument, Flag_Event>
	{
	public:
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
			BranchFlag<Flag_Argument>(std::move(flagToken), std::move(flagDesc), flagRequired),
			TriggerFlag<Flag_Argument, Flag_Event>(std::move(flagToken), std::move(flagDesc), flagRequired)
		{}

		template<FlagType... Flags>
		explicit BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const Flag_Event&& triggeredFunc, Flags&&... subFlags,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired),
			  BranchFlag<Flag_Argument>(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerFlag<Flag_Argument, Flag_Event>(std::move(flagToken), std::move(flagDesc), std::move(triggeredFunc), flagRequired)
		{}

		template<FlagType... Flags>
		explicit BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const Flag_Event&& triggeredFunc, const Flag_Argument&& flagArg, Flags&&... subFlags,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), std::move(flagArg), argRequired, flagRequired),
			  BranchFlag<Flag_Argument>(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerFlag<Flag_Argument, Flag_Event>(std::move(flagToken), std::move(flagDesc), std::move(triggeredFunc), flagRequired)
		{}

		template<FlagType... Flags>
		BranchTrigger& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTrigger& SetFlagEvent(const Flag_Event&& triggeredFunc) noexcept
		{
			const_cast<Flag_Event&>(_triggeredFunc) = std::move(triggeredFunc);

			_triggerSet = true;

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			_triggeredFunc.Run();

			Flag<Flag_Argument>::Raise(itr, end);

			for (auto& subFlag : _nestedFlags)
			{
				subFlag.Raise(itr, end);
			}
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			if (!_triggeredFunc.TryRun(errorMsg))
				return false;

			Flag<Flag_Argument>::TryRaise(itr, end, errorMsg);

			for (auto& subFlag : _nestedFlags)
			{
				if (!subFlag.TryRaise(itr, end, errorMsg))
					return false;
			}

			return true;
		}

		BranchTrigger(const BranchTrigger&) = delete;
		BranchTrigger(BranchTrigger&&) = delete;

		BranchTrigger& operator=(const BranchTrigger&) = delete;
		BranchTrigger& operator=(BranchTrigger&&) = delete;
	};
}
#endif // !BRANCH_TRIGGER_FLAG_H

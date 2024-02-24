#ifndef BRANCH_TRIGGER_FLAG_H
#define BRANCH_TRIGGER_FLAG_H

#include <BranchFlag.h>
#include <TriggerFlag.h>

namespace CmdLineParser
{
	class BranchTrigger : public BranchFlag, public TriggerFlag
	{
	public:
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, Flags&&... subFlags,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagRequired),
			  BranchFlag(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerFlag(std::move(flagToken), std::move(flagDesc), triggeredFunc, flagRequired)
		{}

		template<FlagType... Flags>
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg, Flags&&... subFlags,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagArg, argRequired, flagRequired),
			  BranchFlag(std::move(flagToken), std::move(flagDesc), std::forward<Flags>(subFlags)..., flagRequired),
			  TriggerFlag(std::move(flagToken), std::move(flagDesc), triggeredFunc, flagRequired)
		{}

		template<FlagType... Flags>
		BranchTrigger& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTrigger& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchTrigger(const BranchTrigger&) = delete;
		BranchTrigger(BranchTrigger&&) = delete;

		BranchTrigger& operator=(const BranchTrigger&) = delete;
		BranchTrigger& operator=(BranchTrigger&&) = delete;
	};
}
#endif // !BRANCH_TRIGGER_FLAG_H

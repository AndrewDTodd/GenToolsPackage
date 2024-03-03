#ifndef TRIGGER_SWITCH_FLAG_H
#define TRIGGER_SWITCH_FLAG_H

#include <TriggerFlag.h>
#include <SwitchFlag.h>

namespace CmdLineParser
{
	template<IsFlagEvent Flag_Event>
	class TriggerSwitch : public SwitchFlag, public TriggerFlag<Arg_Bool, Flag_Event>
	{
	public:
		TriggerSwitch(Tokens&& flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false) noexcept
			: Flag<Arg_Bool>(std::move(flagTokens), std::move(flagDesc), flagRequired),
			SwitchFlag(std::move(flagTokens), std::move(flagDesc), defaultSwitchState, flagRequired),
			TriggerFlag<Arg_Bool, Flag_Event>(std::move(flagTokens), std::move(flagDesc), flagRequired)
		{}

		explicit TriggerSwitch(Tokens&& flagTokens, std::string&& flagDesc, const Flag_Event&& triggeredFunc,
			bool defaultSwitchState = false, bool flagRequired = false) noexcept
			: Flag<Arg_Bool>(std::move(flagTokens), std::move(flagDesc), flagRequired),
			SwitchFlag(std::move(flagTokens), std::move(flagDesc), defaultSwitchState, flagRequired),
			TriggerFlag<Arg_Bool, Flag_Event>(std::move(flagTokens), std::move(flagDesc), std::move(triggeredFunc), flagRequired)
		{}

		TriggerSwitch& SetFlagEvent(const Flag_Event&& triggeredFunc) noexcept
		{
			const_cast<flag_event&>(_triggeredFunc) = std::move(triggeredFunc);

			const_cast<bool&>(_triggerSet) = true;

			return *this;
		}

		inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			_triggeredFunc.Run();

			SwitchFlag::Raise(itr, end);
		}

		inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept override
		{
			if (!_triggeredFunc.TryRun(errorMsg))
				return false;

			return SwitchFlag::TryRaise(itr, end, errorMsg);
		}

		TriggerSwitch(const TriggerSwitch&) = delete;
		TriggerSwitch(TriggerSwitch&&) = delete;

		TriggerSwitch& operator=(const TriggerSwitch&) = delete;
		TriggerSwitch& operator=(TriggerSwitch&&) = delete;
	};
}
#endif // !TRIGGER_SWITCH_FLAG_H

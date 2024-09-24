#ifndef GENTOOLS_GENPARSECMDLINE_TRIGGER_FLAG_H
#define GENTOOLS_GENPARSECMDLINE_TRIGGER_FLAG_H

#include <Flag.h>
#include <flag_event.h>
#include <concepts>

namespace GenTools::GenParse
{
	template<typename T>
	concept IsFlagEvent = std::is_base_of_v<flag_event, T> && std::movable<T>;

	template<IsFlagArgument Flag_Argument, IsFlagEvent Flag_Event>
	class TriggerFlag : virtual public Flag<Flag_Argument>
	{
	protected:
		Flag_Event _triggeredFunc;
		bool _triggerSet = false;

	public:
		TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc,
			bool flagRequired = false) noexcept
			: Flag<Flag_Argument>(std::move(flagTokens), std::move(flagDesc), flagRequired)
		{}

		explicit TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc, Flag_Event&& triggeredFunc,
			bool flagRequired = false) noexcept
			: Flag<Flag_Argument>(std::move(flagTokens), std::move(flagDesc), flagRequired), _triggeredFunc(std::move(triggeredFunc)), _triggerSet(true)
		{}

		explicit TriggerFlag(Tokens&& flagTokens, std::string&& flagDesc, Flag_Event&& triggeredFunc, Flag_Argument&& flagArg,
			bool argRequired = false, bool flagRequired = false) noexcept
			: Flag<Flag_Argument>(std::move(flagTokens), std::move(flagDesc), std::move(flagArg), argRequired, flagRequired), _triggeredFunc(std::move(triggeredFunc)), _triggerSet(true)
		{}

		TriggerFlag& SetFlagEvent(Flag_Event&& triggeredFunc) noexcept
		{
			_triggeredFunc = std::move(triggeredFunc);

			_triggerSet = true;

			return *this;
		}

		inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			_triggeredFunc.Run();

			Flag<Flag_Argument>::Raise(itr, end);
		}

		inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept override
		{
			if (!_triggeredFunc.TryRun(errorMsg))
				return false;

			return Flag<Flag_Argument>::TryRaise(itr, end, errorMsg);
		}

		TriggerFlag(const TriggerFlag&) = delete;
		TriggerFlag(TriggerFlag&&) = delete;

		TriggerFlag& operator=(const TriggerFlag&) = delete;
		TriggerFlag& operator=(TriggerFlag&&) = delete;
	};
}
#endif // !GENTOOLS_GENPARSECMDLINE_TRIGGER_FLAG_H

#ifndef TRIGGER_SWITCH_FLAG_H
#define TRIGGER_SWITCH_FLAG_H

#include <Flag.h>
#include <FlagSubTypes/SwitchFlag.h>

namespace CmdLineParser
{
	class TriggerSwitch : public SwitchFlag
	{
	protected:
		const flag_event* _triggeredFunc;

	public:
		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired)
		{
		}

		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired), _triggeredFunc(&triggeredFunc)
		{
		}

		TriggerSwitch& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		TriggerSwitch(const TriggerSwitch&) = delete;
		TriggerSwitch(TriggerSwitch&&) = delete;

		TriggerSwitch& operator=(const TriggerSwitch&) = delete;
		TriggerSwitch& operator=(TriggerSwitch&&) = delete;
	};
}
#endif // !TRIGGER_SWITCH_FLAG_H

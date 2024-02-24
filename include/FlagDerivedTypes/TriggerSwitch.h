#ifndef TRIGGER_SWITCH_FLAG_H
#define TRIGGER_SWITCH_FLAG_H

#include <TriggerFlag.h>
#include <SwitchFlag.h>

namespace CmdLineParser
{
	class TriggerSwitch : public SwitchFlag, public TriggerFlag
	{
	public:
		TriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired),
			  SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired),
			  TriggerFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired)
		{
		}

		TriggerSwitch(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired),
			  SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired),
			  TriggerFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), triggeredFunc, flagRequired)
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

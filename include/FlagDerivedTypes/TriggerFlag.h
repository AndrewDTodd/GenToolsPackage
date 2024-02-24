#ifndef TRIGGER_FLAG_H
#define TRIGGER_FLAG_H

#include <Flag.h>

namespace CmdLineParser
{
	class TriggerFlag : virtual public Flag
	{
	protected:
		const flag_event* _triggeredFunc;

	public:
		TriggerFlag(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired)
		{}

		TriggerFlag(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired), _triggeredFunc(&triggeredFunc)
		{}
		
		TriggerFlag(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagArg, argRequired, flagRequired), _triggeredFunc(&triggeredFunc)
		{}

		TriggerFlag& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		TriggerFlag(const TriggerFlag&) = delete;
		TriggerFlag(TriggerFlag&&) = delete;

		TriggerFlag& operator=(const TriggerFlag&) = delete;
		TriggerFlag& operator=(TriggerFlag&&) = delete;
	};
}
#endif // !TRIGGER_FLAG_H

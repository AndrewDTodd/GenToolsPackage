#ifndef SWITCH_FLAG_H
#define SWITCH_FLAG_H

#include <Flag.h>
#include <flag_argument_specializations.h>

namespace CmdLineParser
{
	class SwitchFlag : virtual public Flag
	{
	protected:
		Arg_Bool switchState;

	public:
		SwitchFlag(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		SwitchFlag(Tokens&&... flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired)
		{
			switchState.SetDefaultValue(std::move(defaultSwitchState));
		}

		SwitchFlag& SetDefaultState(bool defaultSwitchState) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		SwitchFlag(const SwitchFlag&) = delete;
		SwitchFlag(SwitchFlag&&) = delete;

		SwitchFlag& operator=(const SwitchFlag&) = delete;
		SwitchFlag& operator=(SwitchFlag&&) = delete;
	};
}
#endif // !SWITCH_FLAG_H

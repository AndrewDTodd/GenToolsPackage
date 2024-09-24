#ifndef GENTOOLS_GENPARSECMDLINE_SWITCH_FLAG_H
#define GENTOOLS_GENPARSECMDLINE_SWITCH_FLAG_H

#include <Flag.h>
#include <flag_argument_specializations.h>

namespace GenTools::GenParse
{
	class SwitchFlag : virtual public Flag<Arg_Bool>
	{
	public:
		SwitchFlag(Tokens&& flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false) noexcept
			: Flag<Arg_Bool>(std::move(flagTokens), std::move(flagDesc), Arg_Bool(std::move(defaultSwitchState)), false, flagRequired)
		{}

		SwitchFlag& SetDefaultState(bool defaultSwitchState) noexcept
		{
			_flagArg.SetDefaultValue(std::move(defaultSwitchState));

			return *this;
		}

		inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			_flagArg.SetDefaultValue(!_flagArg.GetArg());
		}

		inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			_flagArg.SetDefaultValue(!_flagArg.GetArg());

			return true;
		}

		SwitchFlag(const SwitchFlag&) = delete;
		SwitchFlag(SwitchFlag&&) = delete;

		SwitchFlag& operator=(const SwitchFlag&) = delete;
		SwitchFlag& operator=(SwitchFlag&&) = delete;
	};
}
#endif // !GENTOOLS_GENPARSECMDLINE_SWITCH_FLAG_H

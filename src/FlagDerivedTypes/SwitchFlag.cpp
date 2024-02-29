#include <SwitchFlag.h>

namespace CmdLineParser
{
	SwitchFlag::SwitchFlag(Tokens&& flagTokens, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagTokens), std::move(flagDesc), flagRequired)
	{
		switchState.SetDefaultValue(std::move(defaultSwitchState));
	}

	SwitchFlag& SwitchFlag::SetDefaultState(bool defaultSwitchState) noexcept
	{
		switchState.SetDefaultValue(std::move(defaultSwitchState));

		return *this;
	}
}
#include <SwitchFlag.h>

namespace CmdLineParser
{
	SwitchFlag::SwitchFlag(std::string&& flagToken, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagToken), std::move(flagDesc), flagRequired)
	{
		switchState.SetDefaultValue(std::move(defaultSwitchState));
	}

	SwitchFlag& SwitchFlag::SetDefaultState(bool defaultSwitchState) noexcept
	{
		switchState.SetDefaultValue(std::move(defaultSwitchState));

		return *this;
	}
}
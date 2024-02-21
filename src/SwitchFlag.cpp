#include "../../include/FlagSubTypes/SwitchFlag.h"

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

void SwitchFlag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
{
	switchState.SetDefaultValue(true);
}

bool SwitchFlag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
{
	switchState.SetDefaultValue(true);

	return true;
}
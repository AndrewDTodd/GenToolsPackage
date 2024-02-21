#include "../../include/FlagSubTypes/TriggerSwitch.h"

TriggerSwitch& TriggerSwitch::SetFlagEvent(const flag_event& triggeredFunc) noexcept
{
	_triggeredFunc = &triggeredFunc;

	return *this;
}

void TriggerSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
{
	_triggeredFunc->Run();

	switchState.SetDefaultValue(true);
}

bool TriggerSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
{
	try
	{
		_triggeredFunc->Run();
	}
	catch (const std::exception& e)
	{
		if (errorMsg)
			*errorMsg = e.what();

		return false;
	}

	switchState.SetDefaultValue(true);

	return true;
}
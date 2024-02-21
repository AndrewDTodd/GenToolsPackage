#include "../../include/FlagSubTypes/TriggerFlag.h"

TriggerFlag& TriggerFlag::SetFlagEvent(const flag_event& triggeredFunc) noexcept
{
	_triggeredFunc = &triggeredFunc;

	return *this;
}

void TriggerFlag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
{
	_triggeredFunc->Run();

#ifdef _DEBUG
	if (itr == end)
		throw std::logic_error("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");
#endif // _DEBUG

	_flagArg->Parse(itr->data());

	itr++;
}

bool TriggerFlag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
{
#ifdef _DEBUG
	if (itr == end)
	{
		if (errorMsg)
			*errorMsg = "Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse";

		return false;
	}
#endif // _DEBUG

	try
	{
		_triggeredFunc->Run();

		_flagArg->Parse(itr->data());
	}
	catch (const std::exception& e)
	{
		if (errorMsg)
			*errorMsg = e.what();

		return false;
	}

	itr++;

	return true;
}
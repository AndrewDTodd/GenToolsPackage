#include "../include/Flag.h"

namespace CmdLineParser
{
	Flag::Flag(std::string&& flagToken, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: _flagDesc(std::move(flagDesc)), FlagRequired(flagRequired)
	{
		flagToken.size() > 0 ? (flagToken.size() == 1 ? _shortToken = "-" + std::move(flagToken) : _longTokens.emplace_back("--" + std::move(flagToken))) :
#ifdef _DEBUG
			throw std::logic_error("Empty token provided");
#else
			std::cerr << "Error: Empty token provided to Flag! Run in debug mode for more details on this error." << std::endl;
#endif // _DEBUG
	}

	Flag::Flag(std::string&& flagToken, std::string&& flagDesc, const flag_argument& flagArg,
		bool argRequired, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: _flagDesc(std::move(flagDesc)), _flagArg(&flagArg), ArgRequired(argRequired), FlagRequired(flagRequired)
	{
		flagToken.size() > 0 ? (flagToken.size() == 1 ? _shortToken = "-" + std::move(flagToken) : _longTokens.emplace_back("--" + std::move(flagToken))) :
#ifdef _DEBUG
			throw std::logic_error("Empty token provided");
#else
			std::cerr << "Error: Empty token provided to Flag! Run in debug mode for more details on this error." << std::endl;
#endif // _DEBUG
	}

	Flag::Flag(Flag&& other) noexcept : 
		_shortToken(std::move(other._shortToken)), 
		_longTokens(std::move(other._longTokens)), 
		_flagArg(std::exchange(other._flagArg, nullptr)), 
		_flagDesc(std::move(other._flagDesc))
	{
	}

	Flag& Flag::operator=(Flag&& other) noexcept
	{
		if (this != &other)
		{
			_shortToken = std::move(other._shortToken);
			_longTokens = std::move(other._longTokens);
			_flagArg = std::exchange(other._flagArg, nullptr);
			_flagDesc = std::move(other._flagDesc);
		}

		return *this;
	}

	Flag& Flag::SetFlagRequired(bool required) noexcept
	{
		const_cast<bool&>(FlagRequired) = required;

		return *this;
	}

	Flag& Flag::SetFlagArgRequired(bool required) noexcept
	{
		const_cast<bool&>(ArgRequired) = required;

		return *this;
	}

	Flag& Flag::SetFlagIsPosParsable(bool posParsable) noexcept
	{
		const_cast<bool&>(PosParsable) = posParsable;

		if (posParsable)
		{
			if (FlagRequired)
				const_cast<bool&>(ArgRequired) = true;
		}

		return *this;
	}

	Flag& Flag::SetFlagArgument(const flag_argument& flagArg) noexcept
	{
		_flagArg = &flagArg;

		return *this;
	}

	const std::string& Flag::ShortToken() const noexcept
	{
		return _shortToken;
	}

	const std::vector<std::string>& Flag::LongTokens() const noexcept
	{
		return _longTokens;
	}

	const flag_argument& Flag::FlagArgument() const noexcept
	{
		return *_flagArg;
	}

	const std::string& Flag::FlagDescription() const noexcept
	{
		return _flagDesc;
	}

	void Flag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
#ifdef _DEBUG
		if (itr == end)
			throw std::logic_error("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");
#endif // _DEBUG

		_flagArg->Parse(itr->data());

		itr++;
	}

	bool Flag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
#ifdef _DEBUG
		if (itr == end)
		{
			if(errorMsg)
				*errorMsg = "Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse";

			return false;
		}
#endif // _DEBUG

		try
		{
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
			if(errorMsg)
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

	BranchFlag::BranchFlag(std::string&& flagToken, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagToken), std::move(flagDesc), flagRequired)
	{}

	void BranchFlag::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
#ifdef _DEBUG
		if (itr == end)
			throw std::logic_error("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");
#endif // _DEBUG

		_flagArg->Parse(itr->data());

		itr++;

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchFlag::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
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
			_flagArg->Parse(itr->data());
		}
		catch (const std::exception& e)
		{
			if (errorMsg)
				*errorMsg = e.what();

			return false;
		}

		itr++;

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}

	BranchSwitch::BranchSwitch(std::string&& flagToken, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired)
	{
	}

	void BranchSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		switchState.SetDefaultValue(true);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		switchState.SetDefaultValue(true);

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}

	BranchTrigger::BranchTrigger(std::string&& flagToken, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: Flag(std::move(flagToken), std::move(flagDesc), flagRequired)
	{
	}

	BranchTrigger& BranchTrigger::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;

		return *this;
	}

	void BranchTrigger::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

#ifdef _DEBUG
		if (itr == end)
			throw std::logic_error("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");
#endif // _DEBUG

		_flagArg->Parse(itr->data());

		itr++;

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchTrigger::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
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

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}

	BranchTriggerSwitch::BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
		bool defaultSwitchState, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired)
	{
	}

	BranchTriggerSwitch& BranchTriggerSwitch::SetFlagEvent(const flag_event& triggeredFunc) noexcept
	{
		_triggeredFunc = &triggeredFunc;
		 
		return *this;
	}

	void BranchTriggerSwitch::Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
	{
		_triggeredFunc->Run();

		switchState.SetDefaultValue(true);

		for (auto& subFlag : _nestedFlags)
		{
			subFlag.Raise(itr, end);
		}
	}

	bool BranchTriggerSwitch::TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
	{
		try
		{
			_triggeredFunc->Run();

			switchState.SetDefaultValue(true);
		}
		catch (const std::exception& e)
		{
			if (errorMsg)
				*errorMsg = e.what();

			return false;
		}

		for (auto& subFlag : _nestedFlags)
		{
			if (!subFlag.TryRaise(itr, end, errorMsg))
				return false;
		}

		return true;
	}
}
#include <Flag.h>

//#include <utility>

namespace CmdLineParser
{
	Tokens::Tokens(Tokens&& other) :
		_shortToken(std::move(other._shortToken)),
		_longTokens(std::move(other._longTokens))
	{}

	Tokens& Tokens::operator=(Tokens&& other)
	{
		if (this != &other)
		{
			_shortToken = std::move(other._shortToken);
			_longTokens = std::move(other._longTokens);
		}

		return *this;
	}

	Flag::Flag(Tokens&& flagTokens, std::string&& flagDesc,
		bool flagRequired)
#ifndef _DEBUG 
		noexcept
#endif // !_DEBUG
		: _tokens(std::move(flagTokens)), _flagDesc(std::move(flagDesc)), FlagRequired(flagRequired)
	{}

	Flag::Flag(Tokens&& flagTokens, std::string&& flagDesc, const flag_argument& flagArg,
		bool argRequired, bool flagRequired)
#ifndef _DEBUG
		noexcept
#endif // !_DEBUG
		: _tokens(std::move(flagTokens)), _flagDesc(std::move(flagDesc)), _flagArg(&flagArg), ArgRequired(argRequired), FlagRequired(flagRequired)
	{}

	Flag::Flag(Flag&& other) noexcept :
		_tokens(std::move(other._tokens)),
		_flagArg(std::exchange(other._flagArg, nullptr)), 
		_flagDesc(std::move(other._flagDesc)),
		FlagRequired(other.FlagRequired),
		ArgRequired(other.ArgRequired),
		PosParsable(other.PosParsable)
	{}

	Flag& Flag::operator=(Flag&& other) noexcept
	{
		if (this != &other)
		{
			_tokens = std::move(other._tokens);
			_flagArg = std::exchange(other._flagArg, nullptr);
			_flagDesc = std::move(other._flagDesc);
			const_cast<bool&>(FlagRequired) = other.FlagRequired;
			const_cast<bool&>(ArgRequired) = other.ArgRequired;
			const_cast<bool&>(PosParsable) = other.PosParsable;
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
		return _tokens._shortToken;
	}

	const std::vector<std::string>& Flag::LongTokens() const noexcept
	{
		return _tokens._longTokens;
	}

	const flag_argument& Flag::FlagArgument() const noexcept
	{
		return *_flagArg;
	}

	const std::string& Flag::FlagDescription() const noexcept
	{
		return _flagDesc;
	}
}
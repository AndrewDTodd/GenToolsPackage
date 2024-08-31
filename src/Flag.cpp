#include <Flag.h>

#include <utility>

namespace TokenValueParser
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
}
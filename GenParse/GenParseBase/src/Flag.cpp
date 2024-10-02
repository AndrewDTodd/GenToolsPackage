#include <Flag.h>

#include <utility>

namespace GenTools::GenParse
{
	Tokens::Tokens(Tokens&& other) noexcept :
		_shortToken(std::move(other._shortToken)),
		_longTokens(std::move(other._longTokens))
	{}

	Tokens& Tokens::operator=(Tokens&& other) noexcept
	{
		if (this != &other)
		{
			_shortToken = std::move(other._shortToken);
			_longTokens = std::move(other._longTokens);
		}

		return *this;
	}
}
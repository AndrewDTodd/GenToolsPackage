#ifndef CMD_LINE_PARSER_FLAGS_H
#define CMD_LINE_PARSER_FLAGS_H

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <flag_argument.h>
#include <flag_event.h>

namespace CmdLineParser
{
	//Flag Type
	//************************************************************************************************
	constexpr bool REQUIRED_FLAG = true;
	constexpr bool OPTIONAL_FLAG = false;
	constexpr bool ARGUMENT_REQUIRED = true;
	constexpr bool ARGUMENT_OPTIONAL = false;

	template<typename T>
	concept StringType = std::same_as<T, std::string>;

	class Flag
	{
	protected:
		std::string _shortToken = "";
		std::vector<std::string> _longTokens;
		const flag_argument* _flagArg = nullptr;
		std::string _flagDesc;

	public:
		Flag(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		Flag(Tokens&&... flagTokens, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG 
			noexcept
#endif // !_DEBUG
			: _flagDesc(std::move(flagDesc)), FlagRequired(flagRequired)
		{
			static_assert(sizeof...(Tokens) > 0, "At least one token is required for a Flag");

			((flagTokens.size() > 0 ? (flagTokens.size() == 1 ? _shortToken = "-" + std::move(flagTokens) : _longTokens.emplace_back("--" + std::move(flagTokens))) :
#ifdef _DEBUG
				throw std::logic_error("Empty token provided")
#else
				std::cerr << "Error: Empty token provided to Flag! Run in debug mode for more details on this error." << std::endl
#endif // _DEBUG
				), ...);
		}

		Flag(std::string&& flagToken, std::string&& flagDesc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		Flag(Tokens&&... flagTokens, std::string&& flagDesc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: _flagDesc(std::move(flagDesc)), _flagArg(&flagArg), ArgRequired(argRequired), FlagRequired(flagRequired)
		{
			static_assert(sizeof...(Tokens) > 0, "At least one token is required for a Flag");

			((flagTokens.size() > 0 ? (flagTokens.size() == 1 ? _shortToken = "-" + std::move(flagTokens) : _longTokens.emplace_back("--" + std::move(flagTokens))) :
#ifdef _DEBUG
				throw std::logic_error("Empty token provided")
#else
				std::cerr << "Error: Empty token provided to Flag! Run in debug mode for more details on this error." << std::endl
#endif // _DEBUG
			), ...);
		}

		Flag(Flag&& other) noexcept;

		Flag& operator=(Flag&& other) noexcept;

		const bool FlagRequired = false;
		const bool ArgRequired  = false;
		const bool PosParsable  = false;

		Flag& SetFlagRequired(bool required) noexcept;
		Flag& SetFlagArgRequired(bool required) noexcept;
		Flag& SetFlagIsPosParsable(bool posParsable) noexcept;

		Flag& SetFlagArgument(const flag_argument& flagArg) noexcept;

		const std::string& ShortToken() const noexcept;

		const std::vector<std::string>& LongTokens() const noexcept;

		const flag_argument& FlagArgument() const noexcept;

		const std::string& FlagDescription() const noexcept;

		virtual void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end);

		virtual bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept;

		Flag(const Flag&) = delete;

		Flag& operator=(const Flag&) = delete;
	};
	//************************************************************************************************

	//Flag Modifying/Extension Functions
	//************************************************************************************************
	template<typename... Flags>
	void MakeFlagsPositionParsable(Flags&... flags)
	{
		static_assert((std::is_same_v<Flags, Flag> && ...), "All arguments passed to MakeFlagsPositionParsable must be of type Flag");
		
		(..., flags.SetFlagIsPosParsable(true));
	}
	//************************************************************************************************
}
#endif // !CMD_LINE_PARSER_FLAGS_H

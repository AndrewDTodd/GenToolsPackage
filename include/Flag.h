#ifndef CMD_LINE_PARSER_FLAGS_H
#define CMD_LINE_PARSER_FLAGS_H

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <flag_argument.h>
#include <flag_event.h>

#include <rootConfig.h>

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
				PRINT_ERROR("Error: Empty token provided to Flag! Run in debug mode for more details on this error.");
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
				PRINT_ERROR("Error: Empty token provided to Flag! Run in debug mode for more details on this error.");
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

		virtual inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
		{
			if (!_flagArg)
				throw std::logic_error("Error: The Flag's argument has not been set. Only Switches can be Raised without having set an argument.\nSet the argument in a constructor, or by calling SetFlagArgument.");

			if (itr == end && ArgRequired)
				throw std::logic_error("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");

			try
			{
				_flagArg->Parse(itr->data());
			}
			catch (std::invalid_argument& argExc)
			{
				if (ArgRequired)
					throw argExc;

				return;
			}

			itr++;
		}

		virtual inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept
		{
			if (!_flagArg)
			{
				if(errorMsg)
					*errorMsg = "Error: The Flag's argument has not been set. Only Switches can be Raised without having set an argument.\nSet the argument in a constructor, or by calling SetFlagArgument.";

				PRINT_ERROR("Error: The Flag's argument has not been set. Only Switches can be Raised without having set an argument.\nSet the argument in a constructor, or by calling SetFlagArgument.");

				return false;
			}

			if (itr == end && ArgRequired)
			{
				if(errorMsg)
					*errorMsg = "Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse";

				PRINT_ERROR("Error: The iterator passed to the Flag is already pointing to the container's end. No item to parse");

				return false;
			}

			try
			{
				_flagArg->Parse(itr->data());
			}
			catch (std::invalid_argument& argExc)
			{
				if (ArgRequired)
				{
					if (errorMsg)
						*errorMsg = argExc.what();

					return false;
				}

				return true;
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

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

	template<typename... Ts>
	class initializer_pack
	{
	protected:
		std::array<std::common_type_t<Ts...>, sizeof...(Ts)> collection;

	public:
		constexpr initializer_pack(Ts&&... ts) noexcept
			: collection{ std::forward<Ts>(ts)... }
		{}
	};

	// Deduction guide
	template<typename... Ts>
	initializer_pack(Ts&&... ts) -> initializer_pack<std::decay_t<Ts>...>;

	/*template<typename T>
	concept StringType = requires(T a) {
		{ std::string(a) } -> std::same_as<std::string>;
	};*/

	struct Tokens
	{
		std::string _shortToken = "";
		std::vector<std::string> _longTokens;

		template<typename... Ts>
		requires(sizeof...(Ts) > 0)
		Tokens(Ts&&... tokens)
		{
			auto to_string = [](auto&& token){
				static_assert(std::is_convertible_v<std::remove_reference_t<decltype(token)>, std::string>, "Tokens constructor expects string types");
				return std::string(std::forward<decltype(token)>(token));
			};

			auto token_sort = [&](auto&& token){
				std::string str_token = to_string(std::forward<decltype(token)>(token));
				if (str_token.size() > 0)
				{
					if (str_token.size() == 1)
						_shortToken = "-" + std::move(str_token);
					else
						_longTokens.emplace_back("--" + std::move(str_token));
				}
				else
				{
#ifdef _DEBUG
					throw std::logic_error("Empty token provided");
#else
					PRINT_ERROR("Error: Empty token provided! Run in debug mode for more details on this error.");
#endif // _DEBUG
				}
			};

			(token_sort(std::forward<Ts>(tokens)), ...);
		}

		Tokens(Tokens&& other);
		Tokens& operator=(Tokens&& other);

		Tokens(const Tokens&) = delete;
		Tokens& operator=(const Tokens&) = delete;
	};

	class Flag
	{
	protected:
		Tokens _tokens;
		const flag_argument* _flagArg = nullptr;
		std::string _flagDesc;

	public:
		Flag(Tokens&& flagTokens, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG 
			noexcept
#endif // !_DEBUG
			;

		Flag(Tokens&& flagTokens, std::string&& flagDesc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

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

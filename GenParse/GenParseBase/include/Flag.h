#ifndef GENTOOLS_GENPARSE_FLAG_H
#define GENTOOLS_GENPARSE_FLAG_H

#include <string>
#include <string_view>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <type_traits>
#include <flag_argument.h>
#include <concepts>

#include <PlatformInterface.h>

namespace GenTools::GenParse
{
	//Flag Type
	//************************************************************************************************
	constexpr bool REQUIRED_FLAG = true;
	constexpr bool OPTIONAL_FLAG = false;
	constexpr bool ARGUMENT_REQUIRED = true;
	constexpr bool ARGUMENT_OPTIONAL = false;

	template<typename T>
	concept StringConvertible = std::is_convertible_v<T, std::string>;

	struct Tokens
	{
		std::string _shortToken = "";
		std::vector<std::string> _longTokens;

		template<StringConvertible... Ts>
		requires(sizeof...(Ts) > 0)
		Tokens(Ts&&... tokens)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
		{
			auto to_string = [](auto&& token){
				using T = std::remove_reference_t<decltype(token)>;

				if constexpr (std::is_same_v<T, std::string>)
				{
					return std::move(token);
				}
				else
				{
					return std::string(std::move(token));
				}
			};

			auto token_sort = [&](auto&& token){
				std::string str_token = std::move(to_string(std::move(token)));
				if (str_token.size() > 0)
				{
					if (str_token.size() == 1)
					{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
						if (_shortToken.size() != 0)
						{
							TERMINAL::PRINT_WARNING("Warning: In Flag with short token \'" + _shortToken + "\'\n >>>Short token has already been set. Another token, \'" + str_token + "\', is overriding previous short token");
						}
#endif // _DEBUG
						_shortToken = std::move(str_token);
					}
					else
						_longTokens.emplace_back(std::move(str_token));
				}
				else
				{
#ifdef _DEBUG
					throw std::logic_error("Empty token provided");
#endif // _DEBUG
#ifdef _RELEASE_DEV
					TERMINAL::PRINT_ERROR("Error: In Flag instantiation\n >>>Empty token provided! Run in debug mode for more details on this error");
#endif // RELEASE_ERROR_MSG
				}
			};

			(token_sort(std::move(tokens)), ...);
		}

		Tokens(Tokens&& other) noexcept;
		Tokens& operator=(Tokens&& other) noexcept;

		Tokens(const Tokens&) = delete;
		Tokens& operator=(const Tokens&) = delete;
	};

	class flag_interface
	{
	public:
		const bool FlagRequired = false;
		const bool ArgRequired = false;
		const bool PosParsable = false;

		flag_interface(bool flagRequired, bool argRequired, bool posParsable = false) : FlagRequired(flagRequired), ArgRequired(argRequired), PosParsable(posParsable) {}

		virtual ~flag_interface() = default;

		virtual inline flag_interface&& SetFlagRequired(bool required) noexcept = 0;

		virtual inline flag_interface&& SetFlagArgRequired(bool required) noexcept = 0;

		virtual inline flag_interface&& SetFlagIsPosParsable(bool posParsable) noexcept = 0;

		virtual inline flag_interface&& SetFlagArgument(flag_argument&& flagArg)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			= 0;

		virtual inline const std::string& ShortToken() const noexcept = 0;

		virtual inline const std::vector<std::string>& LongTokens() const noexcept = 0;

		virtual inline bool ArgumentSet() const noexcept = 0;

		virtual inline const flag_argument& FlagArgument() const noexcept = 0;

		virtual inline const std::string& FlagDescription() const noexcept = 0;

		virtual inline const std::string& FlagName() const noexcept = 0;

		virtual inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) = 0;

		virtual inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept = 0;
	};

	template<typename T>
	concept FlagType = std::is_base_of_v<flag_interface, T>&& std::movable<T>;

	template<IsFlagArgument Flag_Argument>
	class Flag : public flag_interface
	{
	protected:
		Tokens _tokens;
		std::string _flagDesc;
		std::string& _flagName;
		Flag_Argument _flagArg;
		bool _argSet = false;

	public:
		Flag(Tokens&& flagTokens, std::string&& flagDesc,
			bool flagRequired = false) noexcept
			: flag_interface(flagRequired, false), 
			_tokens(std::move(flagTokens)), 
			_flagDesc(std::move(flagDesc)), 
			_flagName(_tokens._longTokens.size() ? _tokens._longTokens[0] : _tokens._shortToken)
		{
		}

		explicit Flag(Tokens&& flagTokens, std::string&& flagDesc, Flag_Argument&& flagArg,
			bool argRequired = true, bool flagRequired = false, bool posParsable = false) noexcept
			: flag_interface(flagRequired, argRequired, posParsable),
			_tokens(std::move(flagTokens)),
			_flagDesc(std::move(flagDesc)),
			_flagName(_tokens._longTokens.size() ? _tokens._longTokens[0] : _tokens._shortToken),
			_flagArg(std::move(flagArg)),
			_argSet(true)
		{
		}

		Flag(Flag&& other) noexcept :
			flag_interface(other.FlagRequired, other.ArgRequired, other.PosParsable),
			_flagName(other._flagName),
			_tokens(std::move(other._tokens)),
			_flagArg(std::move(other._flagArg)),
			_argSet(true),
			_flagDesc(std::move(other._flagDesc))
		{
			other._argSet = false;
		}

		Flag& operator=(Flag&& other) noexcept
		{
			if (this != &other)
			{
				_flagName = other._flagName;
				_tokens = std::move(other._tokens);
				_flagArg = std::move(other._flagArg);
				_argSet = true;
				_flagDesc = std::move(other._flagDesc);
				const_cast<bool&>(FlagRequired) = other.FlagRequired;
				const_cast<bool&>(ArgRequired) = other.ArgRequired;
				const_cast<bool&>(PosParsable) = other.PosParsable;

				other._argSet = false;
			}

			return *this;
		}

		inline Flag&& SetFlagRequired(bool required) noexcept final
		{
			const_cast<bool&>(FlagRequired) = required;

			return std::move(*this);
		}

		inline Flag&& SetFlagArgRequired(bool required) noexcept final
		{
			const_cast<bool&>(ArgRequired) = required;

			return std::move(*this);
		}

		inline Flag&& SetFlagIsPosParsable(bool posParsable) noexcept final
		{
			const_cast<bool&>(PosParsable) = posParsable;

			if (posParsable)
			{
				if (FlagRequired)
					const_cast<bool&>(ArgRequired) = true;
			}

			return std::move(*this);
		}

		inline Flag&& SetFlagArgument(flag_argument&& flagArg)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			final
		{
#ifdef _DEBUG
			_flagArg = dynamic_cast<Flag_Argument&&>(std::move(flagArg));
#else
			_flagArg = static_cast<Flag_Argument&&>(std::move(flagArg));
#endif // _DEBUG

			_argSet = true;

			return std::move(*this);
		}

		inline const std::string& ShortToken() const noexcept final
		{
			return _tokens._shortToken;
		}

		inline const std::vector<std::string>& LongTokens() const noexcept final
		{
			return _tokens._longTokens;
		}

		inline bool ArgumentSet() const noexcept final
		{
			return _argSet;
		}

		inline const Flag_Argument& FlagArgument() const noexcept final
		{
			return _flagArg;
		}

		inline const std::string& FlagDescription() const noexcept final
		{
			return _flagDesc;
		}

		inline const std::string& FlagName() const noexcept final
		{
			return _flagName;
		}

		virtual inline void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (!_argSet)
				throw std::logic_error("Error: In Flag instance with name \'" + _flagName + "\'\n >>>The Flag's argument has not been set. Only Switches can be Raised without having set an argument.\nSet the argument in a constructor, or by calling SetFlagArgument");
#endif

			if (itr == end && ArgRequired)
				throw std::invalid_argument("Error: In Flag instance with name \'" + _flagName + "\'\n >>>The iterator passed to the Flag is already pointing to the container's end. No item to parse, and argument is required for this flag");
			else if (itr == end && !ArgRequired)
				return;

			try
			{
				_flagArg.Parse(itr->data());
			}
			catch (std::invalid_argument& argExc)
			{
				if (ArgRequired)
					throw argExc;

				return;
			}

			itr++;
		}

		virtual inline bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (!_argSet)
			{
				TERMINAL::PRINT_ERROR("Error: In Flag instance with name \'" + _flagName + "\'\n >>>The Flag's argument has not been set. Only Switches can be Raised without having set an argument.\nSet the argument in a constructor, or by calling SetFlagArgument");

				return false;
			}
#endif

			if (itr == end && ArgRequired)
			{
				TERMINAL::PRINT_ERROR("Error: In Flag instance with name \'" + _flagName + "\'\n >>>The iterator passed to the Flag is already pointing to the container's end. No item to parse, and argument is required for this flag");

				return false;
			}
			else if (itr == end && !ArgRequired)
				return true;

			try
			{
				_flagArg.Parse(itr->data());
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
	template<typename T>
	concept IsFlag = std::is_base_of_v<flag_interface, T>;

	template<IsFlag... Flags>
	void MakeFlagsPositionParsable(Flags&... flags) noexcept
	{	
		(..., flags.SetFlagIsPosParsable(true));
	}
	//************************************************************************************************
}
#endif // !GENTOOLS_GENPARSE_FLAG_H

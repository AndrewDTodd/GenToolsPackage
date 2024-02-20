#ifndef CMD_LINE_PARSER_FLAGS_H
#define CMD_LINE_PARSER_FLAGS_H

#include <string>
#include <string_view>
#include <vector>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <iostream>

#ifdef _DEBUG
#include <typeinfo>
#endif // _DEBUG


namespace CmdLineParser
{
	//Flag Aggregate Types
	//************************************************************************************************
	template<typename ArgType>
	class flag_argument_t;

	template<typename ArgType>
	class flag_pointer_t;

	class flag_argument
	{
	public:
		virtual void Parse(const char* str) const = 0;
		virtual bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept = 0;

		template<typename ArgType>
		auto as() const
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
		{
			if constexpr (std::is_pointer_v<ArgType>)
			{
				using NonPtrArgType = typename std::remove_pointer<ArgType>::type;
#ifdef _DEBUG
				const flag_pointer_t<NonPtrArgType>* derived = dynamic_cast<const flag_pointer_t<NonPtrArgType>*>(this);
		
				return derived->GetArg();
#else
				return static_cast<const flag_pointer_t<NonPtrArgType>*>(this)->GetArg();
#endif // _DEBUG
			}
			else
			{
#ifdef _DEBUG
				const flag_argument_t<ArgType>* derived = dynamic_cast<const flag_argument_t<ArgType>*>(this);
				
				return derived->GetArg();
#else
				return static_cast<const flag_argument_t<ArgType>*>(this)->GetArg();
#endif // _DEBUG
			}
		}
	};

	template<typename ArgType>
	class flag_argument_t: public flag_argument
	{
	private:
		mutable ArgType argument;
		mutable ArgType(*parseFunc)(const char*) = nullptr;

	public:
		const ArgType& GetArg() const noexcept
		{
			return argument;
		}

		template<typename default_constructible_type = ArgType,
			std::enable_if_t<std::is_default_constructible<default_constructible_type>::value>* = nullptr>
		flag_argument_t() noexcept : argument(ArgType())
		{}

		flag_argument_t(ArgType&& defaultValue) noexcept : argument(std::move(defaultValue))
		{}

		template<typename default_constructible_type = ArgType,
			std::enable_if_t<std::is_default_constructible<default_constructible_type>::value>* = nullptr>
		flag_argument_t(ArgType (*parseFunction)(const char*)) noexcept : argument(ArgType()), parseFunc(parseFunction)
		{}

		flag_argument_t(ArgType&& defaultValue, ArgType (*parseFunction)(const char*)) noexcept : argument(std::move(defaultValue)), parseFunc(parseFunction)
		{}

		const flag_argument_t& SetDefaultValue(ArgType&& defaultValue) const noexcept
		{
			argument = std::move(defaultValue);

			return *this;
		}

		const flag_argument_t& SetParseFunction(ArgType (*parseFunction)(const char*)) const noexcept
		{
			parseFunc = parseFunction;

			return *this;
		}

		void Parse(const char* str) const override
		{
			argument = parseFunc(str);
		}

		bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				argument = parseFunc(str);
			}
			catch (const std::exception& e)
			{
				if (errorMsg)
					*errorMsg = e.what();

				return false;
			}

			return true;
		}

		flag_argument_t(const flag_argument_t&) = delete;
		flag_argument_t(flag_argument_t&&) = delete;

		flag_argument_t& operator=(const flag_argument_t&) = delete;
		flag_argument_t& operator=(flag_argument_t&&) = delete;
	};

	template<typename ArgType>
	class flag_pointer_t : public flag_argument
	{
	private:
		mutable ArgType* argument = nullptr;
		mutable ArgType(*parseFunc)(const char*) = nullptr;

	public:
		const ArgType* const GetArg()  noexcept
		{
			return argument;
		}

		flag_pointer_t() noexcept
		{}

		flag_pointer_t(ArgType* linkedValue) noexcept : argument(linkedValue)
		{}

		flag_pointer_t(ArgType(*parseFunction)(const char*)) noexcept : parseFunc(parseFunction)
		{}

		flag_pointer_t(ArgType* linkedValue, ArgType(*parseFunction)(const char*)) noexcept : argument(linkedValue), parseFunc(parseFunction)
		{}

		const flag_pointer_t& SetLinkedValue(ArgType* linkedValue) const noexcept
		{
			argument = linkedValue;

			return *this;
		}

		const flag_pointer_t& SetParseFunction(ArgType(*parseFunction)(const char*)) const noexcept
		{
			parseFunc = parseFunction;

			return *this;
		}

		void Parse(const char* str) const override
		{
			*argument = parseFunc(str);
		}

		bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				*argument = parseFunc(str);
			}
			catch (const std::exception& e)
			{
				if (errorMsg)
					*errorMsg = e.what();

				return false;
			}

			return true;
		}

		flag_pointer_t(const flag_pointer_t&) = delete;
		flag_pointer_t(flag_pointer_t&&) = delete;

		flag_pointer_t& operator=(const flag_pointer_t&) = delete;
		flag_pointer_t& operator=(flag_pointer_t&&) = delete;
	};

	class flag_event
	{
	public:
		virtual void Run() const = 0;
		virtual bool TryRun(std::string* errorMsg = nullptr) const noexcept = 0;
	};

	template<typename RtrnType, typename... Args>
	class flag_event_t : public flag_event
	{
	private:
		mutable RtrnType(*triggeredFunc)(Args...) = nullptr;
		mutable std::tuple<Args...> args;

	public:
		flag_event_t()
		{}

		flag_event_t(RtrnType(*triggeredFunction)(Args...), Args... arguments) : triggeredFunc(triggeredFunction), args(std::make_tuple(arguments...))
		{}

		const flag_event_t& SetTriggeredFunc(RtrnType(*triggeredFuntion)(Args...)) const noexcept
		{
			triggeredFunc = triggeredFuntion;

			return *this;
		}

		const flag_event_t& SetTriggeredFunc(RtrnType(*triggeredFunction)(Args...), Args... arguments) const noexcept
		{
			triggeredFunc = triggeredFunction;
			args = std::make_tuple(arguments...);

			return *this;
		}

		const flag_event_t& SetFuncArguments(Args... arguments) const noexcept
		{
			args = std::make_tuple(arguments...);

			return *this;
		}

		void Run() const override
		{
			std::apply(triggeredFunc, args);
		}

		bool TryRun(std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				std::apply(triggeredFunc, args);
			}
			catch (const std::exception& e)
			{
				if (errorMsg)
					*errorMsg = e.what();

				return false;
			}

			return true;
		}

		flag_event_t(const flag_event_t&) = delete;
		flag_event_t(flag_event_t&&) = delete;

		flag_event_t& operator=(const flag_event_t&) = delete;
		flag_event_t& operator=(flag_event_t&&) = delete;
	};
	//************************************************************************************************

	//Parsing Functions
	//************************************************************************************************
	bool StringToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"yes", true}, {"no", false},
			{"true", true}, {"false", false},
			{"t", true}, {"f", false},
			{"1", true}, {"0", false},
			{"Yes", true}, {"No", false},
			{"True", true}, {"False", false},
			{"T", true}, {"F", false},
			{"YES", true}, {"NO", false},
			{"TRUE", true}, {"FALSE", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Could not parse " + s + " to bool value");
	}

	bool YesNoToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"yes", true}, {"no", false},
			{"Yes", true}, {"No", false},
			{"YES", true}, {"NO", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Could not parse " + s + " to bool value");
	}

	bool TrueFalseToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"true", true}, {"false", false},
			{"True", true}, {"False", false},
			{"TRUE", true}, {"FALSE", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Could not parse " + s + " to bool value");
	}

	bool TFToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"t", true}, {"f", false},
			{"T", true}, {"F", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Could not parse " + s + " to bool value");
	}

	bool OneZeroToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"1", true}, {"0", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Could not parse " + s + " to bool value");
	}

	int32_t ParseInt32(const char* str)
	{
		size_t pos;
		std::string s(str);
		int32_t value = std::stoi(s, &pos, 0);
		if (pos != s.size())
			throw std::invalid_argument("Could not parse " + s + " to integer value");
		return value;
	}

	uint32_t ParseUInt32(const char* str)
	{
		size_t pos;
		std::string s(str);
		unsigned long value = std::stoul(s, &pos, 0);
		if (pos != s.size() || value > UINT32_MAX)
			throw std::invalid_argument("Could not parse " + s + " to unsigned integer value");
		return static_cast<uint32_t>(value);
	}

	int64_t ParseInt64(const char* str)
	{
		size_t pos;
		std::string s(str);
		long long value = std::stoll(s, &pos, 0);
		if (pos != s.size())
			throw std::invalid_argument("Could not parse " + s + " to QWord value");
		return value;
	}

	uint64_t ParseUInt64(const char* str)
	{
		size_t pos;
		std::string s(str);
		unsigned long long value = std::stoull(s, &pos, 0);
		if (pos != s.size())
			throw std::invalid_argument("Could not parse " + s + " to unsigned QWord value");
		return value;
	}

	float ParseFloat(const char* str)
	{
		size_t pos;
		std::string s(str);
		float value = std::stof(s, &pos);
		if (pos != s.size())
			throw std::invalid_argument("Could not parse " + s + " to float value");
		return value;
	}

	long double ParseDouble(const char* str)
	{
		size_t pos;
		std::string s(str);
		long double value = std::stold(s, &pos);
		if (pos != s.size())
			throw std::invalid_argument("Could not parse " + s + " to double value");
		return value;
	}

	std::string ParseString(const char* str)
	{
		std::string s(str);
		if (s.size() < 2 || s.front() != '"' || s.back() != '"')
			throw std::invalid_argument("Could not parse " + s + " to string value");
		return s.substr(1, s.size() - 2);
	}
	//************************************************************************************************

	//Flag Aggregate Specializations
	//************************************************************************************************
	class Arg_Bool : public flag_argument_t<bool>
	{
	public:
		Arg_Bool() : flag_argument_t<bool>(false, StringToBool) {}
	};

	class Arg_InvertBool : public flag_argument_t<bool>
	{
	public:
		Arg_InvertBool() : flag_argument_t<bool>(true, StringToBool) {}
	};

	class Arg_Int32 : public flag_argument_t<int32_t>
	{
	public:
		Arg_Int32() : flag_argument_t<int32_t>(0, ParseInt32) {}
		Arg_Int32(int32_t&& defaultValue) : flag_argument_t<int32_t>(std::move(defaultValue), ParseInt32) {}
	};

	class Arg_UInt32 : public flag_argument_t<uint32_t>
	{
	public:
		Arg_UInt32() : flag_argument_t<uint32_t>(0, ParseUInt32) {}
		Arg_UInt32(uint32_t&& defaultValue) : flag_argument_t<uint32_t>(std::move(defaultValue), ParseUInt32) {}
	};

	class Arg_Int64 : public flag_argument_t<int64_t>
	{
	public:
		Arg_Int64() : flag_argument_t<int64_t>(0, ParseInt64) {}
		Arg_Int64(int64_t&& defaultValue) : flag_argument_t<int64_t>(std::move(defaultValue), ParseInt64) {}
	};

	class Arg_UInt64 : public flag_argument_t<uint64_t>
	{
	public:
		Arg_UInt64() : flag_argument_t<uint64_t>(0, ParseUInt64) {}
		Arg_UInt64(uint64_t&& defaultValue) : flag_argument_t<uint64_t>(std::move(defaultValue), ParseUInt64) {}
	};

	class Arg_Float : public flag_argument_t<float>
	{
	public:
		Arg_Float() : flag_argument_t<float>(0.0f, ParseFloat) {}
		Arg_Float(float&& defaultValue) : flag_argument_t<float>(std::move(defaultValue), ParseFloat) {}
	};

	class Arg_Double : public flag_argument_t <long double>
	{
	public:
		Arg_Double() : flag_argument_t<long double>(0.0L, ParseDouble) {}
		Arg_Double(long double&& defaultValue) : flag_argument_t<long double>(std::move(defaultValue), ParseDouble) {}
	};

	class Arg_String : public flag_argument_t<std::string>
	{
	public:
		Arg_String() : flag_argument_t<std::string>("", ParseString) {}
		Arg_String(std::string&& defalutValue) : flag_argument_t<std::string>(std::move(defalutValue), ParseString) {}
	};

	class Ptr_Bool : public flag_pointer_t<bool>
	{
	public:
		Ptr_Bool(bool* targetBool) : flag_pointer_t<bool>(targetBool, StringToBool) {}
	};

	class Ptr_Int32 : public flag_pointer_t<int32_t>
	{
	public:
		Ptr_Int32(int32_t* targetInteger) : flag_pointer_t<int32_t>(targetInteger, ParseInt32) {}
	};

	class Ptr_UInt32 : public flag_pointer_t<uint32_t>
	{
	public:
		Ptr_UInt32(uint32_t* targetUnsignedInteger) : flag_pointer_t<uint32_t>(targetUnsignedInteger, ParseUInt32) {}
	};

	class Ptr_Int64 : public flag_pointer_t<int64_t>
	{
	public:
		Ptr_Int64(int64_t* targetInteger) : flag_pointer_t<int64_t>(targetInteger, ParseInt64) {}
	};

	class Ptr_UInt64 : public flag_pointer_t<uint64_t>
	{
	public:
		Ptr_UInt64(uint64_t* targetUnsignedInteger) : flag_pointer_t<uint64_t>(targetUnsignedInteger, ParseUInt64) {}
	};

	class Ptr_Float : public flag_pointer_t<float>
	{
	public:
		Ptr_Float(float* targetFloat) : flag_pointer_t<float>(targetFloat, ParseFloat) {}
	};

	class Ptr_Double : public flag_pointer_t <long double>
	{
	public:
		Ptr_Double(long double* targetDouble) : flag_pointer_t<long double>(targetDouble, ParseDouble) {}
	};

	class Ptr_String : public flag_pointer_t<std::string>
	{
	public:
		Ptr_String(std::string* targetString) : flag_pointer_t<std::string>(targetString, ParseString) {}
	};
	//************************************************************************************************

	//Flag Types
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

	class SwitchFlag : public Flag
	{
	protected:
		Arg_Bool switchState;

	public:
		SwitchFlag(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<StringType... Tokens>
		SwitchFlag(Tokens&&... flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired)
		{
			switchState.SetDefaultValue(std::move(defaultSwitchState));
		}

		SwitchFlag& SetDefaultState(bool defaultSwitchState) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		SwitchFlag(const SwitchFlag&) = delete;
		SwitchFlag(SwitchFlag&&) = delete;

		SwitchFlag& operator=(const SwitchFlag&) = delete;
		SwitchFlag& operator=(SwitchFlag&&) = delete;
	};

	class TriggerFlag : public Flag
	{
	protected:
		const flag_event* _triggeredFunc;

	public:
		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired)
		{}

		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagRequired), _triggeredFunc(&triggeredFunc)
		{}

		template<StringType... Tokens>
		TriggerFlag(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), flagArg, argRequired, flagRequired), _triggeredFunc(&triggeredFunc)
		{}

		TriggerFlag& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		TriggerFlag(const TriggerFlag&) = delete;
		TriggerFlag(TriggerFlag&&) = delete;

		TriggerFlag& operator=(const TriggerFlag&) = delete;
		TriggerFlag& operator=(TriggerFlag&&) = delete;
	};

	class TriggerSwitch : public SwitchFlag
	{
	protected:
		const flag_event* _triggeredFunc;

	public:
		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc,
			bool defaultSwitchState = false , bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired)
		{
		}

		template<StringType... Tokens>
		TriggerSwitch(Tokens&&... flagTokens, std::string&& flagDesc, const flag_event& triggeredFunc,
			bool defaultSwitchState = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::forward<Tokens>(flagTokens)..., std::move(flagDesc), defaultSwitchState, flagRequired), _triggeredFunc(&triggeredFunc)
		{
		}

		TriggerSwitch& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		TriggerSwitch(const TriggerSwitch&) = delete;
		TriggerSwitch(TriggerSwitch&&) = delete;

		TriggerSwitch& operator=(const TriggerSwitch&) = delete;
		TriggerSwitch& operator=(TriggerSwitch&&) = delete;
	};

	template<typename T>
	concept FlagType = std::is_base_of<Flag, T>::value;

	class BranchFlag : public Flag
	{
	protected:
		std::vector<Flag> _nestedFlags;

	public:
		BranchFlag(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchFlag(std::string&& flagToken, std::string&& flagDesc, Flags&&... subFlags,
			bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchFlag(std::string&& flagToken, std::string&& flagDesc, const flag_argument& flagArg, Flags&&... subFlags,
			bool argRequired = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagArg, argRequired, flagRequired), _nestedFlags{std::forward<Flags>(subFlags)...}
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchFlag& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchFlag(const BranchFlag&) = delete;
		BranchFlag(BranchFlag&&) = delete;

		BranchFlag& operator=(const BranchFlag&) = delete;
		BranchFlag& operator=(BranchFlag&&) = delete;
	};

	class BranchSwitch : public SwitchFlag
	{
	protected:
		std::vector<Flag> _nestedFlags;

	public:
		BranchSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchSwitch(std::string&& flagToken, std::string&& flagDesc, Flags&&... subFlags,
			bool defaultSwitchState = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
		}

		template<FlagType... Flags>
		BranchSwitch& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchSwitch(const BranchSwitch&) = delete;
		BranchSwitch(BranchSwitch&&) = delete;

		BranchSwitch& operator=(const BranchSwitch&) = delete;
		BranchSwitch& operator=(BranchSwitch&&) = delete;
	};

	class BranchTrigger : public Flag
	{
	protected:
		const flag_event* _triggeredFunc;
		std::vector<Flag> _nestedFlags;

	public:
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, Flags&&... subFlags,
			bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagRequired), _triggeredFunc(&triggeredFunc), _nestedFlags{std::forward<Flags>(subFlags)...}
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchTrigger(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, const flag_argument& flagArg, Flags&&... subFlags,
			bool argRequired = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag(std::move(flagToken), std::move(flagDesc), flagArg, argRequired, flagRequired), _triggeredFunc(&triggeredFunc), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchTrigger& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTrigger& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchTrigger(const BranchTrigger&) = delete;
		BranchTrigger(BranchTrigger&&) = delete;

		BranchTrigger& operator=(const BranchTrigger&) = delete;
		BranchTrigger& operator=(BranchTrigger&&) = delete;
	};

	class BranchTriggerSwitch : public SwitchFlag
	{
	protected:
		const flag_event* _triggeredFunc;
		std::vector<Flag> _nestedFlags;

	public:
		BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc,
			bool defaultSwitchState = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			;

		template<FlagType... Flags>
		BranchTriggerSwitch(std::string&& flagToken, std::string&& flagDesc, const flag_event& triggeredFunc, Flags&&... subFlags,
			bool defaultSwitchState = false, bool flagRequired = false) 
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: SwitchFlag(std::move(flagToken), std::move(flagDesc), defaultSwitchState, flagRequired), _triggeredFunc(&triggeredFunc), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
		}

		template<FlagType... Flags>
		BranchTriggerSwitch& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		BranchTriggerSwitch& SetFlagEvent(const flag_event& triggeredFunc) noexcept;

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override;

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override;

		BranchTriggerSwitch(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch(BranchTriggerSwitch&&) = delete;

		BranchTriggerSwitch& operator=(const BranchTriggerSwitch&) = delete;
		BranchTriggerSwitch& operator=(BranchTriggerSwitch&&) = delete;
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

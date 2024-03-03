#ifndef FLAG_ARGUMENT_H
#define FLAG_ARGUMENT_H

#include <string>
#include <stdexcept>
#include <concepts>
#include <utility>

namespace CmdLineParser
{
	//Flag Aggregate Types
	//************************************************************************************************
	template<std::movable ArgType>
	class flag_argument_t;

	template<std::movable ArgType>
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

	template<std::movable ArgType>
	class flag_argument_t : public flag_argument
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
		constexpr flag_argument_t() noexcept : argument(ArgType())
		{}

		flag_argument_t(ArgType&& defaultValue) noexcept : argument(std::move(defaultValue))
		{}

		template<typename default_constructible_type = ArgType,
			std::enable_if_t<std::is_default_constructible<default_constructible_type>::value>* = nullptr>
		flag_argument_t(ArgType(*parseFunction)(const char*)) noexcept : argument(ArgType()), parseFunc(parseFunction)
		{}

		flag_argument_t(ArgType&& defaultValue, ArgType(*parseFunction)(const char*)) noexcept : argument(std::move(defaultValue)), parseFunc(parseFunction)
		{}

		flag_argument_t(flag_argument_t&& other) :
			argument(std::move(other.argument)),
			parseFunc(std::exchange(other.parseFunc, nullptr))
		{}

		flag_argument_t& operator=(flag_argument_t&& other)
		{
			if (this != &other)
			{
				argument = std::move(other.argument);
				parseFunc = std::exchange(other.parseFunc, nullptr);
			}

			return *this;
		}

		const flag_argument_t& SetDefaultValue(ArgType&& defaultValue) const noexcept
		{
			argument = std::move(defaultValue);

			return *this;
		}

		const flag_argument_t& SetParseFunction(ArgType(*parseFunction)(const char*)) const noexcept
		{
			parseFunc = parseFunction;

			return *this;
		}

		void Parse(const char* str) const override
		{
			argument = std::move(parseFunc(str));
		}

		bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				argument = std::move(parseFunc(str));
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

		flag_argument_t& operator=(const flag_argument_t&) = delete;
	};

	template<std::movable ArgType>
	class flag_pointer_t : public flag_argument
	{
	private:
		mutable ArgType* argument = nullptr;
		mutable ArgType(*parseFunc)(const char*) = nullptr;

	public:
		const ArgType* GetArg() const noexcept
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

		flag_pointer_t(flag_pointer_t&& other) :
			argument(std::exchange(other.argument, nullptr)),
			parseFunc(std::exchange(other.parseFunc), nullptr)
		{}

		flag_pointer_t& operator=(flag_pointer_t&& other)
		{
			if (this != &other)
			{
				argument = std::exchange(other.argument, nullptr);
				parseFunc = std::exchange(other.parseFunc, nullptr);
			}

			return *this;
		}

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
			*argument = std::move(parseFunc(str));
		}

		bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				*argument = std::move(parseFunc(str));
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

		flag_pointer_t& operator=(const flag_pointer_t&) = delete;
	};
}
#endif // !FLAG_ARGUMENT_H

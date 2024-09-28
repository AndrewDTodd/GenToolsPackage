#ifndef GENTOOLS_GENPARSE_FLAG_EVENT_H
#define GENTOOLS_GENPARSE_FLAG_EVENT_H

#include <string>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <functional>

namespace GenTools::GenParse
{
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
		mutable std::function<RtrnType(Args...)> triggeredFunc;
		mutable std::tuple<Args...> args;

	public:
		flag_event_t() noexcept
		{}

		flag_event_t(std::function<RtrnType(Args...)> triggeredFunction, Args&&... arguments) noexcept : triggeredFunc(triggeredFunction), args(std::forward<Args>(arguments)...)
		{}

		flag_event_t(flag_event_t&& other) noexcept
			: triggeredFunc(std::move(other.triggeredFunc)), args(std::move(other.args))
		{}

		flag_event_t& operator=(flag_event_t&& other) noexcept
		{
			if (this != &other)
			{
				triggeredFunc = std::move(other.triggeredFunc);
				args = std::move(other.args);
			}

			return *this;
		}

		const flag_event_t&& SetTriggeredFunc(std::function<RtrnType(Args...)> triggeredFunction) const noexcept
		{
			triggeredFunc = triggeredFunction;

			return std::move(*this);
		}

		template<typename... FuncArgs>
		const flag_event_t&& SetTriggeredFunc(std::function<RtrnType(Args...)> triggeredFunction, FuncArgs&&... arguments) const noexcept
		{
			static_assert(std::is_same_v<std::tuple<FuncArgs...>, std::tuple<Args...>>, "Provided arguments types don't match the types specified in the template instance");

			triggeredFunc = triggeredFunction;
			args = std::make_tuple(std::forward<FuncArgs>(arguments)...);

			return std::move(*this);
		}

		const flag_event_t&& SetFuncArguments(Args&&... arguments) const noexcept
		{
			args = std::make_tuple(std::forward<Args>(arguments)...);

			return std::move(*this);
		}

		void Run() const override
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (!triggeredFunc) throw std::logic_error("Error: Target function not set for flag_event_t instance. Can't trigger the event");
#endif

			std::apply(triggeredFunc, args);
		}

		bool TryRun(std::string* errorMsg = nullptr) const noexcept override
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (!triggeredFunc)
			{
				if (errorMsg)
					*errorMsg = "Error: Target function not set for flag_event_t instance. Can't trigger the event";
				return false;
			}
#endif

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

		flag_event_t& operator=(const flag_event_t&) = delete;
	};
	//************************************************************************************************
}
#endif // !GENTOOLS_GENPARSE_FLAG_EVENT_H

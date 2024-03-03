#ifndef FLAG_EVENT_H
#define FLAG_EVENT_H

#include <string>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace CmdLineParser
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
		mutable RtrnType(*triggeredFunc)(Args...) = nullptr;
		mutable std::tuple<Args...> args;

	public:
		flag_event_t() noexcept
		{}

		flag_event_t(RtrnType(*triggeredFunction)(Args...), Args&&... arguments) : triggeredFunc(triggeredFunction), args(std::forward<Args>(arguments)...)
		{}

		flag_event_t(flag_event_t&& other) noexcept
			: triggeredFunc(std::exchange(other.triggeredFunc, nullptr)), args(std::move(other.args))
		{}

		flag_event_t& operator=(flag_event_t&& other) noexcept
		{
			if (this != &other)
			{
				triggeredFunc = std::exchange(other.triggeredFunc, nullptr);
				args = std::move(other.args);
			}

			return *this;
		}

		const flag_event_t& SetTriggeredFunc(RtrnType(*triggeredFunction)(Args...)) const noexcept
		{
			triggeredFunc = triggeredFunction;

			return *this;
		}

		template<typename... FuncArgs>
		const flag_event_t& SetTriggeredFunc(RtrnType(*triggeredFunction)(Args...), FuncArgs&&... arguments) const
		{
			static_assert(std::is_same_v<std::tuple<FuncArgs...>, std::tuple<Args...>>, "Provided arguments types don't match the types specified in the template instance");

			triggeredFunc = triggeredFunction;
			args = std::make_tuple(std::forward<FuncArgs>(arguments)...);

			return *this;
		}

		const flag_event_t& SetFuncArguments(Args&&... arguments) const
		{
			args = std::make_tuple(std::forward<Args>(arguments)...);

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

		flag_event_t& operator=(const flag_event_t&) = delete;
	};
	//************************************************************************************************
}
#endif // !FLAG_EVENT_H

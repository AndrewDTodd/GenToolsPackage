#ifndef BRANCH_FLAG_H
#define BRANCH_FLAG_H

#include <unordered_map>
#include <memory>

#include <Flag.h>

namespace CmdLineParser
{
	template<typename T>
	concept FlagType = std::is_base_of_v<flag_interface, T> && std::movable<T>;

	template<typename T, template <typename> class U>
	concept IsSpecializationOf = requires (T a) {
		{ static_cast<U<typename T::Flag_Argument>*>(nullptr) } -> std::same_as<T*>;
	};

	template<IsFlagArgument Flag_Argument>
	class BranchFlag : virtual public Flag<Flag_Argument>
	{
	protected:
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _nestedFlags;

		template<FlagType F>
		void AddFlagToMap(F&& flag)
		{
			std::shared_ptr<flag_interface> newFlag = std::make_shared<flag_interface>(std::move(flag));

			if constexpr (IsSpecializationOf<F, BranchFlag>)
			{
				for (const auto& longToken : newFlag->LongTokens())
				{
					_nestedFlags.emplace(longToken, newFlag);
				}
			}
			else
			{
				if (!newFlag->ShortToken().empty())
				{
					_nestedFlags.emplace("-" + newFlag->ShortToken(), newFlag);
				}

				for (const auto& longToken : newFlag->LongTokens())
				{
					_nestedFlags.emplace("--" + longToken, newFlag);
				}
			}
		}

	public:
		BranchFlag(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired)
		{
#ifdef _DEBUG
			if (this->_tokens._shortToken.length() != 0)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG
		}

		explicit BranchFlag(std::string&& flagToken, std::string&& flagDesc, Flag_Argument&& flagArg,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), std::move(flagArg), argRequired, flagRequired)
		{
#ifdef _DEBUG
			if (this->_tokens._shortToken.length() != 0)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG
		}

		template<FlagType... Flags>
		BranchFlag(std::string&& flagToken, std::string&& flagDesc, bool flagRequired, Flags&&... subFlags)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired)
		{
#ifdef _DEBUG
			if (this->_tokens._shortToken.length() != 0)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);
		}

		template<FlagType... Flags>
		explicit BranchFlag(std::string&& flagToken, std::string&& flagDesc, Flag_Argument&& flagArg, bool argRequired, bool flagRequired, Flags&&... subFlags)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), std::move(flagArg), argRequired, flagRequired)
		{
#ifdef _DEBUG
			if (this->_tokens._shortToken.length() != 0)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);
		}

		BranchFlag(BranchFlag&& other) noexcept :
			Flag<Flag_Argument>(std::move(other)),
			_nestedFlags(std::move(other._nestedFlags))
		{}

		BranchFlag& operator=(BranchFlag&& other) noexcept
		{
			if (this != &other)
			{
				Flag<Flag_Argument>::operator=(std::move(other));

				_nestedFlags = std::move(other._nestedFlags);
			}

			return *this;
		}

		template<FlagType... Flags>
		BranchFlag& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& NestedFlags() const noexcept
		{
			return _nestedFlags;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			Flag<Flag_Argument>::Raise(itr, end);

			_nestedFlags 
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			Flag<Flag_Argument>::TryRaise(itr, end, errorMsg);

			for (auto& subFlag : _nestedFlags)
			{
				if (!subFlag->TryRaise(itr, end, errorMsg))
					return false;
			}

			return true;
		}

		BranchFlag(const BranchFlag&) = delete;

		BranchFlag& operator=(const BranchFlag&) = delete;
	};
}
#endif // !BRANCH_FLAG_H

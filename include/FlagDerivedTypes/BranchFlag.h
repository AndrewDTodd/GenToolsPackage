#ifndef BRANCH_FLAG_H
#define BRANCH_FLAG_H

#include <Flag.h>

namespace CmdLineParser
{
	template<typename T>
	concept FlagType = std::is_base_of<flag_interface, T>::value;

	template<IsFlagArgument Flag_Argument>
	class BranchFlag : virtual public Flag<Flag_Argument>
	{
	protected:
		std::vector<flag_interface> _nestedFlags;

	public:
		BranchFlag(std::string&& flagToken, std::string&& flagDesc,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired)
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#elif RELEASE_ERROR_MSG
			PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchFlag(std::string&& flagToken, std::string&& flagDesc, Flags&&... subFlags,
			bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#elif RELEASE_ERROR_MSG
			PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		explicit BranchFlag(std::string&& flagToken, std::string&& flagDesc, const Flag_Argument&& flagArg, Flags&&... subFlags,
			bool argRequired = false, bool flagRequired = false)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Flag_Argument>(Tokens(std::move(flagToken)), std::move(flagDesc), std::move(flagArg), argRequired, flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
		{
#ifdef _DEBUG
			if (flagToken.length() == 1)
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#elif RELEASE_ERROR_MSG
			PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
		}

		template<FlagType... Flags>
		BranchFlag& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_nestedFlags.clear();
			(_nestedFlags.emplace_back(std::forward<Flags>(subFlags)), ...);

			return *this;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			Flag<Flag_Argument>::Raise(itr, end);

			for (auto& subFlag : _nestedFlags)
			{
				subFlag.Raise(itr, end);
			}
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			Flag<Flag_Argument>::TryRaise(itr, end, errorMsg);

			for (auto& subFlag : _nestedFlags)
			{
				if (!subFlag.TryRaise(itr, end, errorMsg))
					return false;
			}

			return true;
		}

		BranchFlag(const BranchFlag&) = delete;
		BranchFlag(BranchFlag&&) = delete;

		BranchFlag& operator=(const BranchFlag&) = delete;
		BranchFlag& operator=(BranchFlag&&) = delete;
	};
}
#endif // !BRANCH_FLAG_H

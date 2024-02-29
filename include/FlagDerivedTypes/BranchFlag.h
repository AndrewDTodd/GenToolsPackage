#ifndef BRANCH_FLAG_H
#define BRANCH_FLAG_H

#include <Flag.h>

namespace CmdLineParser
{
	template<typename T>
	concept FlagType = std::is_base_of<Flag, T>::value;

	class BranchFlag : virtual public Flag
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
			: Flag(Tokens(std::move(flagToken)), std::move(flagDesc), flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
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
			: Flag(Tokens(std::move(flagToken)), std::move(flagDesc), flagArg, argRequired, flagRequired), _nestedFlags{ std::forward<Flags>(subFlags)... }
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
}
#endif // !BRANCH_FLAG_H

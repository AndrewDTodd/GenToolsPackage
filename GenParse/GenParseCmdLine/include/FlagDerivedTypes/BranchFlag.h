#ifndef GENTOOLS_GENPARSECMDLINE_BRANCH_FLAG_H
#define GENTOOLS_GENPARSECMDLINE_BRANCH_FLAG_H

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>

#include <Flag.h>
#include <CmdLineParser.h>
#include <common_definitions.h>
#include <TriggerSwitch.h>

#include <PlatformInterface.h>

namespace GenTools::GenParse
{
	template<
		ParsingMode ParseMode = ParsingMode::Lenient,
		ExceptionMode ExcMode = ExceptionMode::Strict,
		VerbositySetting Verbosity = VerbositySetting::Verbose,
		PosParsingSetting PosParse = PosParsingSetting::AllowNonDelimited
	>
	class BranchFlag : virtual public Flag<__void_argument>
	{
	protected:
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _optionalFlags;
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _requiredFlags;
		std::vector<std::shared_ptr<flag_interface>> _posParsableFlags;

		uint32_t _reqFlagsCount = 0;
		uint32_t _detectedReqFlags = 0;
		std::string _requiredFlagsExpectedTokens;

		std::string _posParsableLeftDelimiter = "{";
		std::string _posParsableRightDelimiter = "}";

		FORCE_INLINE std::pair<std::string, size_t> FlagDetails(const std::shared_ptr<flag_interface>& flag, size_t& longestDetails) const noexcept
		{
			const std::string& st = flag->ShortToken();
			const std::vector<std::string>& lt = flag->LongTokens();
			std::string argType;

			if (dynamic_cast<SwitchFlag*>(flag.get()) != nullptr)
			{
				argType = "";
			}
			else
			{
				argType = flag->FlagArgument().GetArgType();
			}

			std::stringstream flagStream;
			size_t flagLength = 0;

			if (!st.empty())
			{
				flagStream << TERMINAL::_GRAPHIC_MODE_DIMB << '-' << TERMINAL::_GRAPHIC_RESET_DIMB << st << ", ";
				flagLength += st.length() + 3; // Shorth Token length plus '-' and ", "
			}

			flagStream << TERMINAL::_GRAPHIC_MODE_DIMB << "--" << TERMINAL::_GRAPHIC_RESET_DIMB << lt[0];
			flagLength += lt[0].length() + 2; // First Long Token plus "--"

			for (int i = 1; i < lt.size(); i++)
			{
				flagStream << ", --" << lt[i];
				flagLength += lt[i].length() + 4; // Long Token plus ", --"
			}
			
			if (!argType.empty())
			{
				flagStream << TERMINAL::_GRAPHIC_MODE_DIMB << " <" << argType << '>' << TERMINAL::_GRAPHIC_RESET_DIMB;
				flagLength += argType.length() + 3; // Arg Type string plus " <" and '>'
			}

			if (flagLength > longestDetails)
				longestDetails = flagLength;

			return { flagStream.str(), flagLength };
		}

		FORCE_INLINE void PrintPosParsableFlagsDetails(const std::vector<std::shared_ptr<flag_interface>>& posParsableFlags) const noexcept
		{
			int num = 1;
			for (const auto& flag : posParsableFlags)
			{
				std::cout << "  " << TERMINAL::_GRAPHIC_MODE_DIMB << '(' << num << ") " << TERMINAL::_GRAPHIC_RESET_DIMB;
				
				if (!flag->ShortToken().empty())
					std::cout << TERMINAL::_GRAPHIC_MODE_DIMB << '-' << TERMINAL::_GRAPHIC_RESET_DIMB << flag->ShortToken();

				for (const auto& longToken : flag->LongTokens())
				{
					std::cout << ", " << TERMINAL::_GRAPHIC_MODE_DIMB << "--" << TERMINAL::_GRAPHIC_RESET_DIMB << longToken;
				}
				std::cout << TERMINAL::_GRAPHIC_MODE_DIMB << " <" << flag->FlagArgument().GetArgType() << ">\n" << TERMINAL::_GRAPHIC_RESET_DIMB;
				num++;
			}
		}

		void DefaultHelpDialog() const noexcept
		{
			TERMINAL::_SET_COLOR_256("216");

			std::unordered_set<std::shared_ptr<flag_interface>> uniqueRequiredFlags;
			std::vector<std::pair<std::pair<std::string, size_t>, std::string>> detailsRequiredFlags;
			size_t longestDetails = 0;
			for (const auto& [name, flag] : _requiredFlags)
			{
				uniqueRequiredFlags.insert(flag);
			}

			std::unordered_set<std::shared_ptr<flag_interface>> uniqueOptionalFlags;
			std::vector<std::pair<std::pair<std::string, size_t>, std::string>> detailsOptionalFlags;
			for (const auto& [name, flag] : _optionalFlags)
			{
				uniqueOptionalFlags.insert(flag);
			}

			std::cout << "BranchFlag \'" << _flagName << "\' : " << FlagDescription() << "\n\n" <<
				"Usage:\n " << CmdLineParser<>::GetInstance()->GetProgramName() << " --" << _flagName << " [flags]";

			for (const auto& flag : _posParsableFlags)
			{
				std::cout << " <" << flag->FlagName() << '>';
			}
			std::cout << "\n\n";

			longestDetails += 2;
			if (_requiredFlags.size())
			{
				std::cout << "Required Flags:\n";

				for (const auto& flag : uniqueRequiredFlags)
				{
					std::pair<std::string, size_t> flagDetails = FlagDetails(flag, longestDetails);
					detailsRequiredFlags.emplace_back(flagDetails, flag->FlagDescription());
				}

				for (const auto& pair : detailsRequiredFlags)
				{
					size_t padding = (pair.first.second + 2 < longestDetails) ? (longestDetails - pair.first.second + 3) : 0;
					std::cout << "  " << pair.first.first << std::setw(padding) << " : " << pair.second << '\n';
				}
				std::cout << '\n';
			}

			if (_optionalFlags.size())
			{
				std::cout << "Optional Flags:\n";

				for (const auto& flag : uniqueOptionalFlags)
				{
					std::pair<std::string, size_t> flagDetails = FlagDetails(flag, longestDetails);
					detailsOptionalFlags.emplace_back(flagDetails, flag->FlagDescription());
				}

				for (const auto& pair : detailsOptionalFlags)
				{
					size_t padding = (pair.first.second + 2 < longestDetails) ? (longestDetails - pair.first.second + 3) : 0;
					std::cout << "  " << pair.first.first << std::setw(padding) << " : " << pair.second << '\n';
				}
				std::cout << '\n';
			}

			if (_posParsableFlags.size())
			{
				if constexpr (PosParse == PosParsingSetting::AssertDelimited)
				{
					std::cout << "\nThe Following Flags are marked as position parsable and may be set by providing a delimited ordered sequence of arguments\n" <<
						"left delimiter=\'" << _posParsableLeftDelimiter << "\', right delimiter=\'" << _posParsableRightDelimiter << "\'\n";
				}
				else
				{
					std::cout << "\nThe Following Flags are marked as position parsable and may be set by providing an ordered sequence of arguments\n";
				}
				PrintPosParsableFlagsDetails(_posParsableFlags);
				std::cout << '\n';
			}

			std::cout << TERMINAL::_GRAPHIC_MODE_RESET;
		}

		template<FlagType F>
		FORCE_INLINE void AddFlagToMap(F&& flag)
		{
			std::shared_ptr<flag_interface> newFlag = std::make_shared<F>(std::move(flag));

			if constexpr (IsBranchFlag<F>)
			{
				for (const auto& longToken : newFlag->LongTokens())
				{
					_optionalFlags.emplace(longToken, newFlag);
				}
			}
			else
			{
				if (newFlag->PosParsable)
				{
					_posParsableFlags.emplace_back(newFlag);

#if defined(_DEBUG) or defined(_RELEASE_DEV)
					if constexpr (PosParse == PosParsingSetting::AllowNonDelimited)
					{
						if (dynamic_cast<const Arg_String*>(&newFlag->FlagArgument()))
						{
							std::string warning = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" +
								" >>>Flag with \'Arg_String\' argument type may cause ambiguous parsing when used in non-delimited position parsable sequences. It is recommended to use the \'Arg_DelString\' type instead";
							TERMINAL::PRINT_WARNING(warning);
						}
					}
#endif
				}

				if (!newFlag->FlagRequired)
				{
					if (!newFlag->ShortToken().empty())
					{
						_optionalFlags.emplace("-" + newFlag->ShortToken(), newFlag);
					}

					for (const auto& longToken : newFlag->LongTokens())
					{
						_optionalFlags.emplace("--" + longToken, newFlag);
					}
				}
				else
				{
					if (!newFlag->ShortToken().empty())
					{
						_requiredFlags.emplace("-" + newFlag->ShortToken(), newFlag);
					}

					for (const auto& longToken : newFlag->LongTokens())
					{
						_requiredFlags.emplace("--" + longToken, newFlag);
					}

					_requiredFlagsExpectedTokens += std::string(" (-") + newFlag->ShortToken() + std::string(", --") + newFlag->LongTokens()[0] + std::string("),");
					_reqFlagsCount++;
				}
			}
		}

		FORCE_INLINE void ParseTokenFlags(const std::string& key, std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
		{
			auto itrOptionalFlags = _optionalFlags.find(key);
			if (itrOptionalFlags != _optionalFlags.end())
			{
				std::shared_ptr<flag_interface> flag = itrOptionalFlags->second;
				itr++;

				if constexpr (ExcMode == ExceptionMode::Strict)
				{
					flag->Raise(itr, end);
				}
				else
				{
					try
					{
						flag->Raise(itr, end);
					}
					catch (const std::exception& e)
					{
						std::string errorMsg = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
							" >>>Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'.\nError: " + e.what();
						TERMINAL::PRINT_WARNING(errorMsg);
					}
				}
			}
			
			auto itrRequiredFlags = _requiredFlags.find(key);
			if (itrRequiredFlags != _requiredFlags.end())
			{
				std::shared_ptr<flag_interface> flag = itrRequiredFlags->second;
				itr++;

				if constexpr (ExcMode == ExceptionMode::Strict)
				{
					flag->Raise(itr, end);
					_detectedReqFlags++;
				}
				else
				{
					try
					{
						flag->Raise(itr, end);
						_detectedReqFlags++;
					}
					catch (const std::exception& e)
					{
						std::string errorMsg = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
							" >>>Ignoring parsing error from options branch sub - option.Thrown from option with token \'" + key + "\'.\nError: " + e.what();
						TERMINAL::PRINT_WARNING(errorMsg);
					}
				}
			}
		}

		FORCE_INLINE void ParseDelPositionParsableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end)
		{
			std::vector<std::string_view>::const_iterator itr = leftDelimiter;
			uint32_t arguments = 0;
			itr++;
			while (itr != end && *itr != _posParsableRightDelimiter)
			{
				itr++;
				arguments++;
			}

			if(itr == end)
				throw std::runtime_error("Error: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
					" >>>Expected matching right delimiter \'" + _posParsableRightDelimiter + "\' to the left delimiter \'" + _posParsableLeftDelimiter + "\' that denote the boundaries of the position parsable flags arguments");

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParsableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						throw std::invalid_argument("Error: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
							" >>>Excess arguments will not be ignored.\nOptions branch " + this->_tokens._longTokens[0] + " has " + std::to_string(_posParsableFlags.size()) + " position parsable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parsable sequence");
					}
					else
					{
						TERMINAL::PRINT_WARNING("Warning: In BranchFlag instance with name \'" + _flagName + "\'\n >>>Ignoring excess arguments in the position parsable sequence");
					}
				}
			}

			leftDelimiter++;

			uint32_t iterations = _posParsableFlags.size() > arguments ? arguments : _posParsableFlags.size();

			for (uint32_t i = 0; i < iterations; i++)
			{
				_posParsableFlags[i]->Raise(leftDelimiter, end);
				if (_posParsableFlags[i]->ArgRequired)
					_detectedReqFlags++;
			}

			leftDelimiter = itr;
			leftDelimiter++;
		}

		FORCE_INLINE bool TryParseTokenFlags(const std::string& key, std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
		{
			auto itrOptionalFlags = _optionalFlags.find(key);
			if (itrOptionalFlags != _optionalFlags.end())
			{
				std::shared_ptr<flag_interface> flag = itrOptionalFlags->second;
				itr++;

				if constexpr (ExcMode == ExceptionMode::Strict)
				{
					if (!flag->TryRaise(itr, end, errorMsg))
						return false;
				}
				else
				{
					if (!flag->TryRaise(itr, end, errorMsg))
					{
						std::string msg = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
							" >>>Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'";
						if (errorMsg)
							msg += ".\nError: " + *errorMsg;
						TERMINAL::PRINT_WARNING(msg);
					}
				}
			}
			else
			{
				auto itrRequiredFlags = _requiredFlags.find(key);
				if (itrRequiredFlags != _requiredFlags.end())
				{
					std::shared_ptr<flag_interface> flag = itrRequiredFlags->second;
					itr++;

					if constexpr (ExcMode == ExceptionMode::Strict)
					{
						if (!flag->TryRaise(itr, end, errorMsg))
							return false;

						_detectedReqFlags++;
					}
					else
					{
						if (!flag->TryRaise(itr, end, errorMsg))
						{
							std::string msg = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
								" >>>Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'";
							if (errorMsg)
								msg += ".\nError: " + *errorMsg;
							TERMINAL::PRINT_WARNING(msg);
						}
						
						_detectedReqFlags++;
					}
				}
			}

			return true;
		}

		FORCE_INLINE bool TryParseDelPositionParsableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
		{
			std::vector<std::string_view>::const_iterator itr = leftDelimiter;
			uint32_t arguments = 0;
			itr++;
			while (itr != end && *itr != _posParsableRightDelimiter)
			{
				itr++;
				arguments++;
			}

			if (itr == end)
			{
				std::string msg = "Error: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
					" >>>Expected matching right delimiter \'" + _posParsableRightDelimiter + "\' to the left delimiter \'" + _posParsableLeftDelimiter + "\' that denote the boundaries of the position parsable flags arguments";

				TERMINAL::PRINT_ERROR(msg);

				return false;
			}

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParsableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						std::string msg = "Error: In BranchFlag instance with name \'" + _flagName + "\'\n" +
							" >>>Excess arguments will not be ignored. Options branch has " + std::to_string(_posParsableFlags.size()) + " position parsable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parsable sequence";

						TERMINAL::PRINT_ERROR(msg);

						return false;
					}
					else
					{
						std::string msg = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" +
							" >>>Ignoring excess arguments in the position parsable sequence";
						TERMINAL::PRINT_WARNING(msg);
					}
				}
			}

			leftDelimiter++;

			uint32_t iterations = _posParsableFlags.size() > arguments ? arguments : _posParsableFlags.size();

			for (uint32_t i = 0; i < iterations; i++)
			{
				if (!_posParsableFlags[i]->TryRaise(leftDelimiter, end, errorMsg))
					return false;
				if (_posParsableFlags[i]->ArgRequired)
					_detectedReqFlags++;
			}

			leftDelimiter = itr;
			leftDelimiter++;

			return true;
		}

		FORCE_INLINE void ParsePositionParsableFlags(std::vector<std::string_view>::const_iterator& sequenceStart, const std::vector<std::string_view>::const_iterator end) noexcept
		{
			if (_posParsableFlags.empty())
				return;

			size_t i = 0;
			while (sequenceStart != end)
			{
				if (!_posParsableFlags[i]->TryRaise(sequenceStart, end))
					return;

				if (_posParsableFlags[i]->ArgRequired)
					_detectedReqFlags++;

				i++;
			}
		}

	public:
		BranchFlag(std::string&& branchName, std::string&& branchDesc)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<__void_argument>(Tokens(std::move(branchName)), std::move(branchDesc), false)
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (this->_tokens._shortToken.length() != 0)
			{
#ifdef _DEBUG
				throw std::invalid_argument("Error: In BranchFlag instance with description \'" + _flagDesc + "\'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#ifdef _RELEASE_DEV
				TERMINAL::PRINT_ERROR("Error: In BranchFlag instance with description \'" + _flagDesc + "\'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _RELEASE_DEV
			}
#endif
		}

		template<FlagType... Flags>
		BranchFlag(std::string&& branchName, std::string&& branchDesc, Flags&&... subFlags)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<__void_argument>(Tokens(std::move(branchName)), std::move(branchDesc), false)
		{
#if defined(_DEBUG) or defined(_RELEASE_DEV)
			if (this->_tokens._shortToken.length() != 0)
			{
#ifdef _DEBUG
				throw std::invalid_argument("Error: In BranchFlag instance with description \'" + _flagDesc +"\'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#ifdef _RELEASE_DEV
				TERMINAL::PRINT_ERROR("Error: In BranchFlag instance with description \'" + _flagDesc + "\'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _RELEASE_DEV
			}
#endif

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			if (!_optionalFlags.contains("--help"))
			{
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, BranchFlag*>>>(Tokens("help"), "Provides information about the branch, and the options provided by its sub-flags", flag_event_t<void, BranchFlag*>(std::bind(&BranchFlag::DefaultHelpDialog, this), this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}
		}

		BranchFlag(BranchFlag&& other) noexcept :
			Flag<__void_argument>(std::move(other)),
			_optionalFlags(std::move(other._optionalFlags)),
			_requiredFlags(std::move(other._requiredFlags)),
			_posParsableFlags(std::move(other._posParsableFlags)),
			_posParsableLeftDelimiter(std::move(other._posParsableLeftDelimiter)),
			_posParsableRightDelimiter(std::move(other._posParsableRightDelimiter)),
			_detectedReqFlags(std::exchange(other._detectedReqFlags, 0)),
			_requiredFlagsExpectedTokens(std::move(other._requiredFlagsExpectedTokens))
		{}

		BranchFlag& operator=(BranchFlag&& other) noexcept
		{
			if (this != &other)
			{
				Flag<__void_argument>::operator=(std::move(other));

				_optionalFlags = std::move(other._optionalFlags);
				_requiredFlags = std::move(other._requiredFlags);
				_posParsableFlags = std::move(other._posParsableFlags);
				_posParsableLeftDelimiter = std::move(other._posParsableLeftDelimiter);
				_posParsableRightDelimiter = std::move(other._posParsableRightDelimiter);
				_detectedReqFlags = std::exchange(other._detectedReqFlags, 0);
				_requiredFlagsExpectedTokens = std::move(other._requiredFlagsExpectedTokens);
			}

			return *this;
		}

		template<FlagType... Flags>
		inline BranchFlag&& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_optionalFlags.clear();
			_requiredFlags.clear();
			_posParsableFlags.clear();
			_detectedReqFlags = 0;
			_requiredFlagsExpectedTokens.clear();

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			if (!_optionalFlags.contains("--help"))
			{
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, BranchFlag*>>>(Tokens("help"), "Provides information about the branch, and the options provided by its sub-flags", flag_event_t<void, BranchFlag*>(std::bind(&BranchFlag::DefaultHelpDialog, this), this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}

			return std::move(*this);
		}

		inline BranchFlag&& SetPosParsableDelimiters(std::string&& leftDelimiter, std::string&& rightDelimiter) noexcept
		{
			_posParsableLeftDelimiter = std::move(leftDelimiter);
			_posParsableRightDelimiter = std::move(rightDelimiter);

			return std::move(*this);
		}

		inline const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& OptionalFlags() const noexcept
		{
			return _optionalFlags;
		}

		inline const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& RequiredFlags() const noexcept
		{
			return _requiredFlags;
		}

		inline const std::vector<std::shared_ptr<flag_interface>>& PosParsableFlags() const noexcept
		{
			return _posParsableFlags;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			while (itr != end)
			{
				std::string key(itr->begin(), itr->end());

				if (key != _posParsableLeftDelimiter)
				{
					std::vector<std::string_view>::const_iterator beforeItr = itr;
					ParseTokenFlags(key, itr, end);
					if (itr == beforeItr)
					{
						if constexpr (PosParse == PosParsingSetting::AllowNonDelimited)
						{
							ParsePositionParsableFlags(itr, end);
							if (itr != beforeItr)
							{
								continue;
							}
						}

						if constexpr (ParseMode == ParsingMode::Strict)
						{
							throw std::invalid_argument("Error: In BranchFlag instance with name \'" + _flagName + "\'\n" +
								" >>>Unknown token \'" + key + "\' provided");
						}
						else if constexpr (Verbosity == VerbositySetting::Verbose)
						{
							std::string message = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
								" >>>Ignoring unknown token \'" + key + "\' provided";
							TERMINAL::PRINT_WARNING(message);
						}

						break;
					}
				}
				else
				{
					ParseDelPositionParsableFlags(itr, end);
				}
			}

			if (_detectedReqFlags < _reqFlagsCount && !_optionalFlags["--help"]->FlagArgument().as<bool>())
				throw std::invalid_argument("Error: In BranchFlag instance with name \'" + _flagName + "\'\n" +
					" >>>Options branch requires that the," + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments");
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			while (itr != end)
			{
				std::string key(itr->begin(), itr->end());

				if (key != _posParsableLeftDelimiter)
				{
					std::vector<std::string_view>::const_iterator beforeItr = itr;

					if (!TryParseTokenFlags(key, itr, end, errorMsg))
						return false;

					if (itr == beforeItr)
					{
						if constexpr (PosParse == PosParsingSetting::AllowNonDelimited)
						{
							ParsePositionParsableFlags(itr, end);
							if (itr != beforeItr)
							{
								continue;
							}
						}

						if constexpr (ParseMode == ParsingMode::Strict)
						{
							std::string msg = "Error: In BranchFlag instance with name \'" + _flagName + "\'\n" +
								" >>>Unknown token \'" + key + "\' provided";

							TERMINAL::PRINT_ERROR(msg);

							return false;
						}
						else if constexpr (Verbosity == VerbositySetting::Verbose)
						{
							std::string message = "Warning: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
								" >>>Ignoring unknown token \'" + key + "\' provided";
							TERMINAL::PRINT_WARNING(message);
						}

						break;
					}
				}
				else
				{
					if (!TryParseDelPositionParsableFlags(itr, end, errorMsg))
						return false;
				}
			}

			if (_detectedReqFlags < _reqFlagsCount && !_optionalFlags["--help"]->FlagArgument().as<bool>())
			{
				std::string msg = "Error: In BranchFlag instance with name \'" + _flagName + "\'\n" + 
					" >>>Options branch requires that the," + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments";

				TERMINAL::PRINT_ERROR(msg);

				return false;
			}

			return true;
		}

		BranchFlag(const BranchFlag&) = delete;

		BranchFlag& operator=(const BranchFlag&) = delete;
	};
}
#endif // !GENTOOLS_GENPARSECMDLINE_BRANCH_FLAG_H

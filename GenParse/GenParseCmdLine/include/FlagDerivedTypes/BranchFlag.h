#ifndef GENTOOLS_GENPARSECMDLINE_BRANCH_FLAG_H
#define GENTOOLS_GENPARSECMDLINE_BRANCH_FLAG_H

#include <unordered_map>
#include <memory>
#include <iostream>
#include <string>
#include <typeinfo>

#include <Flag.h>
#include <TriggerSwitch.h>

#include <rootConfig.h>

namespace GenTools::GenParse
{
	template<typename T>
	concept FlagType = std::is_base_of_v<flag_interface, T> && std::movable<T>;

	enum class ParsingMode
	{
		Strict,
		Lenient
	};

	enum class ExceptionMode
	{
		Strict,
		Lenient
	};

	enum class VerbositySetting
	{
		Silent,
		Verbose
	};

	enum class PosParsingSetting
	{
		AssertDelimited,
		AllowNonDelimited
	};

	template<typename T>
	concept IsBranchFlag = requires() {
		typename T::ParsingMode;
		typename T::ExceptionMode;
		typename T::VerbositySetting;
		{ std::is_base_of<Flag<Arg_Bool>, T>::value } -> std::same_as<bool>;
	};

	template<
		ParsingMode ParseMode = ParsingMode::Lenient, 
		ExceptionMode ExcMode = ExceptionMode::Strict, 
		VerbositySetting Verbosity = VerbositySetting::Verbose, 
		PosParsingSetting PosParse = PosParsingSetting::AllowNonDelimited
	>
	class BranchFlag : virtual public Flag<Arg_Bool>
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

		static void DefaultHelpDialog(BranchFlag* instance)
		{
			std::cout << "Test" << std::endl;
		}

		template<FlagType F>
		inline void AddFlagToMap(F&& flag)
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

		inline void ParseTokenFlags(const std::string& key, std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
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

		inline void ParseDelPositionParsableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end)
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

		inline bool TryParseTokenFlags(const std::string& key, std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
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

		inline bool TryParseDelPositionParsableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg) noexcept
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

		inline void ParsePositionParsableFlags(std::vector<std::string_view>::const_iterator& sequenceStart, const std::vector<std::string_view>::const_iterator end) noexcept
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
			: Flag<Arg_Bool>(Tokens(std::move(branchName)), std::move(branchDesc), false)
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
			: Flag<Arg_Bool>(Tokens(std::move(branchName)), std::move(branchDesc), false)
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
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, BranchFlag*>>>(Tokens("help"), "Provides information about the branch, and the options provided by its sub-flags", flag_event_t<void, BranchFlag*>(BranchFlag::DefaultHelpDialog, this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}
		}

		BranchFlag(BranchFlag&& other) noexcept :
			Flag<Arg_Bool>(std::move(other)),
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
				Flag<Arg_Bool>::operator=(std::move(other));

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
		BranchFlag&& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_optionalFlags.clear();
			_requiredFlags.clear();
			_posParsableFlags.clear();
			_detectedReqFlags = 0;
			_requiredFlagsExpectedTokens.clear();

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			if (!_optionalFlags.contains("--help"))
			{
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, BranchFlag*>>>(Tokens("help"), "Provides information about the branch, and the options provided by its sub-flags", flag_event_t<void, BranchFlag*>(BranchFlag::DefaultHelpDialog, this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}

			return std::move(*this);
		}

		BranchFlag&& SetPosParsableDelimiters(std::string&& leftDelimiter, std::string&& rightDelimiter) noexcept
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

			if (_detectedReqFlags < _reqFlagsCount)
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

			if (_detectedReqFlags < _reqFlagsCount)
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

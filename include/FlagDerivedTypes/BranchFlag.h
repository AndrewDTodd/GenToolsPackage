#ifndef BRANCH_FLAG_H
#define BRANCH_FLAG_H

#include <unordered_map>
#include <memory>
#include <iostream>
#include <string>

#include <Flag.h>
#include <TriggerSwitch.h>

#include <rootConfig.h>

namespace TokenValueParser
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

	template<typename T>
	concept IsBranchFlag = requires() {
		typename T::ParsingMode;
		typename T::ExceptionMode;
		typename T::VerbositySetting;
		{ std::is_base_of<Flag<Arg_Bool>, T>::value } -> std::same_as<bool>;
	};

	template<ParsingMode ParseMode = ParsingMode::Lenient, ExceptionMode ExcMode = ExceptionMode::Strict, VerbositySetting Verbosity = VerbositySetting::Verbose>
	class BranchFlag : virtual public Flag<Arg_Bool>
	{
	protected:
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _optionalFlags;
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _requiredFlags;
		std::vector<std::shared_ptr<flag_interface>> _posParseableFlags;

		uint32_t _detectedReqFlags = 0;
		std::string _requiredFlagsExpectedTokens;

		std::string _posParseableLeftDelimiter = "{";
		std::string _posParseableRightDelimiter = "}";

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
					_posParseableFlags.emplace_back(std::move(newFlag));
				}
				else if (!newFlag->FlagRequired)
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
						std::string errorMsg = "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'.\nError: " + e.what();
						PRINT_WARNING(errorMsg);
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
							std::string errorMsg = "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'.\nError" + e.what();
							PRINT_WARNING(errorMsg);
						}
					}
				}
				/*else
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						throw std::invalid_argument("Unknown token \'" + key + "\' provided")
					}
					else if constexpr (Verbosity == VerbositySetting::Verbose)
					{
						std::string message = "Warning: Ignoring unknown token \'" + key + "\' provided.";
						PRINT_WARNING(message)
					}
				}*/
			}
		}

		inline void ParsePositionParseableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end)
		{
			std::vector<std::string_view>::const_iterator itr = leftDelimiter;
			uint32_t arguments = 0;
			while (itr != end && *itr != _posParseableRightDelimiter)
			{
				itr++;
				arguments++;
			}

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParseableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						throw std::invalid_argument("Excess arguments will not be ignored.\nOptions branch " + this->_tokens._longTokens[0] + " has " + std::to_string(_posParseableFlags.size()) + " position parseable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parseable sequence.");
					}
					else
					{
						PRINT_WARNING("Warning: Options branch " + this->_tokens._longTokens[0] + " ignoring excess arguments in the position parseable sequence.");
					}
				}
			}

			if (itr != end)
			{
				leftDelimiter++;

				uint32_t iterations = _posParseableFlags.size() > arguments ? arguments : _posParseableFlags.size();

				for (uint32_t i = 0; i < iterations; i++)
				{
					_posParseableFlags[i]->Raise(leftDelimiter, end);
				}
			}
			else
			{
				throw std::runtime_error("Expected matching right delimiter \'" + _posParseableRightDelimiter + "\' to the left delimiter \'" + _posParseableLeftDelimiter + "\' that denote the boundries of the position parseable flags arguments.");
			}
		}

		inline bool TryParseTokenFlags(const std::string& key, std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg)
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
						std::string msg = "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'.\nError: " + *errorMsg;
						PRINT_WARNING(msg);
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
							std::string msg = "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'" + key + "\'.\nError: " + *errorMsg;
							PRINT_WARNING(msg);
						}
						
						_detectedReqFlags++;
					}
				}
				/*else
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						throw std::invalid_argument("Unknown token \'" + key + "\' provided")
					}
					else if constexpr (Verbosity == VerbositySetting::Verbose)
					{
						std::string message = "Warning: Ignoring unknown token \'" + key + "\' provided.";
						PRINT_WARNING(message)
					}
				}*/
			}

			return true;
		}

		inline bool TryParsePositionParseableFlags(std::vector<std::string_view>::const_iterator& leftDelimiter, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg)
		{
			std::vector<std::string_view>::const_iterator itr = leftDelimiter;
			uint32_t arguments = 0;
			while (itr != end && *itr != _posParseableRightDelimiter)
			{
				itr++;
				arguments++;
			}

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParseableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						if (errorMsg)
							*errorMsg = "Excess arguments will not be ignored.\nOptions branch " + this->_tokens._longTokens[0] + " has " + std::to_string(_posParseableFlags.size()) + " position parseable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parseable sequence.";

#if defined(_DEBUG) or RELEASE_ERROR_MSG
						PRINT_ERROR("Excess arguments will not be ignored.\nOptions branch " + this->_tokens._longTokens[0] + " has " + std::to_string(_posParseableFlags.size()) + " position parseable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parseable sequence.");
#endif
						return false;
					}
					else
					{
						PRINT_WARNING("Warning: Options branch " + this->_tokens._longTokens[0] + " ignoring excess arguments in the position parseable sequence.");
					}
				}
			}

			if (itr != end)
			{
				leftDelimiter++;

				uint32_t iterations = _posParseableFlags.size() > arguments ? arguments : _posParseableFlags.size();

				for (uint32_t i = 0; i < iterations; i++)
				{
					if (!_posParseableFlags[i]->TryRaise(leftDelimiter, end, errorMsg))
						return false;
				}
			}
			else
			{
				if (errorMsg)
					*errorMsg = "Expected matching right delimiter \'" + _posParseableRightDelimiter + "\' to the left delimiter \'" + _posParseableLeftDelimiter + "\' that denote the boundries of the position parseable flags arguments.";

#if defined(_DEBUG) or RELEASE_ERROR_MSG
				PRINT_ERROR("Expected matching right delimiter \'" + _posParseableRightDelimiter + "\' to the left delimiter \'" + _posParseableLeftDelimiter + "\' that denote the boundries of the position parseable flags arguments.");
#endif

				return false;
			}

			return true;
		}

	public:
		BranchFlag(std::string&& branchName, std::string&& branchDesc)
#ifndef _DEBUG
			noexcept
#endif // !_DEBUG
			: Flag<Arg_Bool>(Tokens(std::move(branchName)), std::move(branchDesc), false)
		{
#if defined(_DEBUG) or RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
			{
#ifdef _DEBUG
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG
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
#if defined(_DEBUG) or RELEASE_ERROR_MSG
			if (this->_tokens._shortToken.length() != 0)
			{
#ifdef _DEBUG
				throw std::invalid_argument("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // _DEBUG
#if RELEASE_ERROR_MSG
				PRINT_ERROR("Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token");
#endif // RELEASE_ERROR_MSG
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
			_posParseableFlags(std::move(other._posParseableFlags)),
			_posParseableLeftDelimiter(std::move(other._posParseableLeftDelimiter)),
			_posParseableRightDelimiter(std::move(other._posParseableRightDelimiter)),
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
				_posParseableFlags = std::move(other._posParseableFlags);
				_posParseableLeftDelimiter = std::move(other._posParseableLeftDelimiter);
				_posParseableRightDelimiter = std::move(other._posParseableRightDelimiter);
				_detectedReqFlags = std::exchange(other._detectedReqFlags, 0);
				_requiredFlagsExpectedTokens = std::move(other._requiredFlagsExpectedTokens);
			}

			return *this;
		}

		template<FlagType... Flags>
		BranchFlag& SetSubFlags(Flags&&... subFlags) noexcept
		{
			_optionalFlags.clear();
			_requiredFlags.clear();
			_posParseableFlags.clear();
			_detectedReqFlags = 0;
			_requiredFlagsExpectedTokens.clear();

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			if (!_optionalFlags.contains("--help"))
			{
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, BranchFlag*>>>(Tokens("help"), "Provides information about the branch, and the options provided by its sub-flags", flag_event_t<void, BranchFlag*>(BranchFlag::DefaultHelpDialog, this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}

			return *this;
		}

		BranchFlag& SetPosParseableDelimiters(std::string&& leftDelimiter, std::string&& rightDelimiter) noexcept
		{
			_posParseableLeftDelimiter = std::move(leftDelimiter);
			_posParseableRightDelimiter = std::move(rightDelimiter);

			return *this;
		}

		const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& OptionalFlags() const noexcept
		{
			return _optionalFlags;
		}

		const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& RequiredFlags() const noexcept
		{
			return _requiredFlags;
		}

		const std::vector<std::shared_ptr<flag_interface>>& PosParseableFlags() const noexcept
		{
			return _posParseableFlags;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end) override
		{
			std::string key(itr->begin(), itr->end());

			while (itr != end)
			{
				if (key != _posParseableLeftDelimiter)
				{
					std::vector<std::string_view>::const_iterator beforeItr = itr;
					ParseTokenFlags(key, itr, end);
					if (itr == beforeItr)
						break;
				}
				else
				{
					ParsePositionParseableFlags(itr, end);
					break;
				}
			}

			if (_detectedReqFlags != _requiredFlags.size())
				throw std::invalid_argument(this->_tokens._longTokens[0] + " options branch requires that the ," + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments.");
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept override
		{
			std::string key(itr->begin(), itr->end());

			while (itr != end)
			{
				if (key != _posParseableLeftDelimiter)
				{
					std::vector<std::string_view>::const_iterator beforeItr = itr;

					if (!TryParseTokenFlags(key, itr, end, errorMsg))
						return false;

					if (itr == beforeItr)
						break;
				}
				else
				{
					if (!TryParsePositionParseableFlags(itr, end, errorMsg))
						return false;
					
					break;
				}
			}

			if (_detectedReqFlags != _requiredFlags.size())
			{
				std::string msg = this->_tokens._longTokens[0] + " options branch requires that the ," + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments.";
				if (errorMsg)
					*errorMsg = msg;

#if defined(_DEBUG) or RELEASE_ERROR_MSG
				PRINT_ERROR(msg);
#endif

				return false;
			}

			return true;
		}

		BranchFlag(const BranchFlag&) = delete;

		BranchFlag& operator=(const BranchFlag&) = delete;
	};
}
#endif // !BRANCH_FLAG_H

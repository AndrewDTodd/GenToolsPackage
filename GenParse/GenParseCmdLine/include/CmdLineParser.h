#ifndef GENTOOLS_GENPARSECMDLINE_CMDLINEPARSER_H
#define GENTOOLS_GENPARSECMDLINE_CMDLINEPARSER_H

#include <thread>
#include <shared_mutex>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <iostream>
#include <iomanip>
#include <string>
#include <typeinfo>

#include <Flag.h>

#include <common_definitions.h>
#include <TriggerSwitch.h>

#include <rootConfig.h>

namespace GenTools::GenParse
{
	/// <summary>
	/// Singleton CmdLineParser object can be used to parse command line arguments by populating the instance with flags and branches of flags
	/// Call CmdLineParser.GetInstance() (specify template arguments for non default configuration)
	/// Call Initialize() on the instance and provide the instance with a program name, description, and sub-flags
	/// </summary>
	/// <typeparam name="ParseMode">Lenient or Strict. Configure the parser's policy on irregularities in the arguments sequence it is parsing</typeparam>
	/// <typeparam name="ExcMode">Lenient or Strict. Configures the parser's policy on exceptions produced from its sub-flags while parsing</typeparam>
	/// <typeparam name="Verbosity">Silent or Verbose. Configure the parser's logging behavior</typeparam>
	/// <typeparam name="PosParse">AssertDelimited or AllowNonDelimited. Configure the parser's behavior for position parsable sequences of arguments</typeparam>
	template<
		ParsingMode ParseMode = ParsingMode::Lenient,
		ExceptionMode ExcMode = ExceptionMode::Strict,
		VerbositySetting Verbosity = VerbositySetting::Verbose,
		PosParsingSetting PosParse = PosParsingSetting::AllowNonDelimited
	>
	class CmdLineParser
	{
	private:
		/// <summary>
		/// SharedMutex for concurrent read access and exclusive write access
		/// </summary>
		inline static std::shared_mutex _sharedMutex;

		/// <summary>
		/// Pointer the the instance of this singleton that is created when GetInstance is called
		/// </summary>
		inline static CmdLineParser* s_instance = nullptr;

		/// <summary>
		/// Set when the parser is initialized, 
		/// human readable string indicating the programs name/identifier in system PATH used to launch program. 
		/// Used when printing help messages to indicate totality of command that is entered in terminal
		/// </summary>
		inline static std::string _programName;

		/// <summary>
		/// Set when the parser is initialized, 
		/// human readable string providing a description of the program. 
		/// Used wne printing help messages
		/// </summary>
		inline static std::string _programRootDescription;

		/// <summary>
		/// Collection that holds the flags marked as optional in the set of configured command line arguments
		/// </summary>
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _optionalFlags;

		/// <summary>
		/// Collection that holds the flags marked as required in the set of configured command line arguments
		/// </summary>
		std::unordered_map<std::string, std::shared_ptr<flag_interface>> _requiredFlags;

		/// <summary>
		/// Collection that holds the flags marked as position parsable in the set of configured command line arguments
		/// </summary>
		std::vector<std::shared_ptr<flag_interface>> _posParsableFlags;

		/// <summary>
		/// Variable is set to hold the number of unique flag instances marked as required when the parser is populated with flags. 
		/// Used to verify all required flags were specified and detected at end of parsing
		/// </summary>
		uint32_t _reqFlagsCount = 0;

		/// <summary>
		/// Variable that is iterated to count the number of flags marked as required that are encountered while parsing
		/// </summary>
		uint32_t _detectedReqFlags = 0;

		/// <summary>
		/// Stirng that is set when parser is populated with flags. 
		/// Set to a human readable help message indicating the flags the program requires the user to specify on the command line
		/// </summary>
		std::string _requiredFlagsExpectedTokens;

		/// <summary>
		/// Used when the parser is set to require delimited position parsable argument sequences, 
		/// this is set to the character that defines the beginning of the position parsable sequence
		/// </summary>
		std::string _posParsableLeftDelimiter = "{";

		/// <summary>
		/// Used when the parser is set to require delimited position parsable argument sequences, 
		/// this is set to the character that defines the end of the position parsable sequence
		/// </summary>
		std::string _posParsableRightDelimiter = "}";

		/// <summary>
		/// Part of the DefaultHelpDialog procedure. Generates a human readable string describing one of the parser's flags
		/// </summary>
		/// <param name="flag">Reference to one of the parser's flags</param>
		/// <param name="longestDetails">Modifiable reference to a size_t instance that is updated to keep track of the longest string this method produces</param>
		/// <returns>a pair of the string generated and the length of that string not including special control characters</returns>
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

		/// <summary>
		/// Part of the DefaultHelpDialog procedure. Prints out a human readable description of the parser's flags marked as position parsable
		/// </summary>
		/// <param name="posParsableFlags">Reference to the parser's collection of position parsable flags</param>
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

		/// <summary>
		/// Method that is provided as a default solution to generate help dialog describing the parsers flags and how to use them
		/// </summary>
		void DefaultHelpDialog() const noexcept
		{
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

			TERMINAL::_SET_COLOR_256("216");

			std::cout << _programName << " : " << _programRootDescription << "\n\n" <<
				"Usage:\n" << 
				_programName << " [flags]";

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
		}

		/// <summary>
		/// Method that sorts and categorizes a flag provided to populate the parser. Actually populates the parser's collections of flags
		/// </summary>
		/// <typeparam name="F">Template that is used for compile time types checking of the specific flag instance</typeparam>
		/// <param name="flag">RValue Reference to the flag to categorize and insert</param>
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
							std::string warning = "Warning: In Command Line Parser\n >>>Flag with \'Arg_String\' argument type may cause ambiguous parsing when used in non-delimited position parsable sequences. It is recommended to use the \'Arg_DelString\' type instead";
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

		/// <summary>
		/// Used by the parser to parse regular (not position parsable) flags
		/// </summary>
		/// <param name="key">A string taken from the arguments sequence that is meant to indicate a particular flag in the parser's collection</param>
		/// <param name="itr">Modifiable iterator into the arguments sequence. Points to the current argument being parsed</param>
		/// <param name="end">Iterator to the end of the arguments sequence used to indicate end of parsing</param>
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
						std::string errorMsg = "Warning: In Command Line Parser\n >>>Ignoring parsing error thrown from option with token \'" + key + "\'.\nError: " + e.what();
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
						std::string errorMsg = "Warning: In Command Line Parser\n >>>Ignoring parsing error thrown from option with token \'" + key + "\'.\nError: " + e.what();
						TERMINAL::PRINT_WARNING(errorMsg);
					}
				}
			}
		}

		/// <summary>
		/// Used by the parser to parse position parsable flags with a delimited sequence of arguments
		/// </summary>
		/// <param name="leftDelimiter">Modifiable iterator into the argument sequence. Should be called with iterator pointing to beginning/left delimiter</param>
		/// <param name="end">Iterator to the end of the arguments sequence used to indicate end of parsing</param>
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

			if (itr == end)
				throw std::runtime_error("Error: In Command Line Parser\n >>>Expected matching right delimiter \'" + _posParsableRightDelimiter + "\' to the left delimiter \'" + _posParsableLeftDelimiter + "\' that denote the boundaries of the position parsable flags arguments");

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParsableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						throw std::invalid_argument("Error: In Command Line Parser\n >>>Excess arguments will not be ignored.\nHas " + std::to_string(_posParsableFlags.size()) + " position parsable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parsable sequence");
					}
					else
					{
						TERMINAL::PRINT_WARNING("Warning: In Command Line Parser\n >>>Ignoring excess arguments in the position parsable sequence");
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

		/// <summary>
		/// Same function as ParseTokenFlags, but cannot throw exceptions and returns true or false to indicate success or failure
		/// </summary>
		/// <param name="key">A string taken from the arguments sequence that is meant to indicate a particular flag in the parser's collection</param>
		/// <param name="itr">Modifiable iterator into the arguments sequence. Points to the current argument being parsed</param>
		/// <param name="end">Iterator to the end of the arguments sequence used to indicate end of parsing</param>
		/// <param name="errorMsg">Pointer to string to be set with error messages if error is encountered</param>
		/// <returns>true or false to indicate success or failure</returns>
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
						std::string msg = "Warning: In Command Line Parser\n >>>Ignoring parsing error thrown from option with token \'" + key + "\'";
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
							std::string msg = "Warning: In Command Line Parser\n >>>Ignoring parsing error thrown from option with token \'" + key + "\'";
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

		/// <summary>
		/// Same function as ParseDelPositionParsableFlags, but cannot throw exceptions and returns true or false to indicate success or failure
		/// </summary>
		/// <param name="leftDelimiter">Modifiable iterator into the argument sequence. Should be called with iterator pointing to beginning/left delimiter</param>
		/// <param name="end">Iterator to the end of the arguments sequence used to indicate end of parsing</param>
		/// <param name="errorMsg">Pointer to string to be set with error messages if error is encountered</param>
		/// <returns>true or false to indicate success or failure</returns>
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
				std::string msg = "Error: In Command Line Parser\n >>>Expected matching right delimiter \'" + _posParsableRightDelimiter + "\' to the left delimiter \'" + _posParsableLeftDelimiter + "\' that denote the boundaries of the position parsable flags arguments";

				TERMINAL::PRINT_ERROR(msg);

				return false;
			}

			if constexpr (ParseMode == ParsingMode::Strict || Verbosity == VerbositySetting::Verbose)
			{
				if (arguments > _posParsableFlags.size())
				{
					if constexpr (ParseMode == ParsingMode::Strict)
					{
						std::string msg = "Error: In Command Line Parser\n >>>Excess arguments will not be ignored. Has " + std::to_string(_posParsableFlags.size()) + " position parsable flags configured. " + std::to_string(arguments) + " arguments were provided in the position parsable sequence";

						TERMINAL::PRINT_ERROR(msg);

						return false;
					}
					else
					{
						std::string msg = "Warning: In Command Line Parser\n >>>Ignoring excess arguments in the position parsable sequence";
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

		/// <summary>
		/// Used by the parser to parse position parsable flags with a non-delimited sequence of arguments
		/// </summary>
		/// <param name="sequenceStart">Modifiable iterator pointing to the current argument to consider for parsing</param>
		/// <param name="end">Iterator to the end of the arguments sequence used to indicate end of parsing</param>
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

		/// <summary>
		/// Default constructor is private, CmdLineParser is a singleton. Internal use only
		/// </summary>
		CmdLineParser()
		{}

	public:
		/// <summary>
		/// Get a pointer to the singletons active instance, or makes an instance if there isn't one already
		/// </summary>
		/// <returns>Pointer to the CmdLineParser instance</returns>
		static CmdLineParser<ParseMode, ExcMode, Verbosity, PosParse>* GetInstance() noexcept
		{
			//Acquire concurrent read lock for read access
			std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

			if (!CmdLineParser::s_instance)
			{
				//Release the read lock to prevent deadlock
				readLock.unlock();
				//Acquire exclusive lock for write
				std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

				if (!CmdLineParser::s_instance)
				{
					CmdLineParser::s_instance = new CmdLineParser();
					return CmdLineParser::s_instance;
				}
			}
			return CmdLineParser::s_instance;
		}

		/// <summary>
		/// Initialize the CmdLineParser and configure with provided parameters
		/// </summary>
		/// <param name="programName">A string representing the program's name/its name in PATH that can be called to launch it</param>
		/// <param name="programDescription">A string containing a short description of the program</param>
		inline void Initialize(std::string&& programName, std::string&& programDescription) noexcept
		{
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			_programName = programName;
			_programRootDescription = programDescription;
		}

		template<FlagType... Flags>
		inline CmdLineParser&& SetSubFlags(Flags&&... subFlags) noexcept
		{
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			_optionalFlags.clear();
			_requiredFlags.clear();
			_posParsableFlags.clear();
			_detectedReqFlags = 0;
			_requiredFlagsExpectedTokens.clear();

			(AddFlagToMap(std::forward<Flags>(subFlags)), ...);

			if (!_optionalFlags.contains("--help"))
			{
				std::shared_ptr<flag_interface> flag = std::make_shared<TriggerSwitch<flag_event_t<void, CmdLineParser*>>>(Tokens("help"), "Provides information about program, and the options provided by its flags", flag_event_t<void, CmdLineParser*>(std::bind(&CmdLineParser::DefaultHelpDialog, this), this));

				_optionalFlags.emplace(std::string("--help"), flag);
			}

			return std::move(*this);
		}

		inline CmdLineParser&& SetPosParsableDelimiters(std::string&& leftDelimiter, std::string&& rightDelimiter) noexcept
		{
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

			_posParsableLeftDelimiter = std::move(leftDelimiter);
			_posParsableRightDelimiter = std::move(rightDelimiter);

			return std::move(*this);
		}

		inline const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& OptionalFlags() const noexcept
		{
			//Acquire concurrent read lock for read access
			std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

			return _optionalFlags;
		}

		inline const std::unordered_map<std::string, std::shared_ptr<flag_interface>>& RequiredFlags() const noexcept
		{
			//Acquire concurrent read lock for read access
			std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

			return _requiredFlags;
		}

		inline const std::vector<std::shared_ptr<flag_interface>>& PosParsableFlags() const noexcept
		{
			//Acquire concurrent read lock for read access
			std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

			return _posParsableFlags;
		}

		inline const std::string& GetProgramName() const noexcept
		{
			//Acquire concurrent read lock for read access
			std::shared_lock<std::shared_mutex> readLock(_sharedMutex);

			return _programName;
		}

		void Raise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end)
		{
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

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
							throw std::invalid_argument("Error: In Command Line Parser\n >>>Unknown token \'" + key + "\' provided");
						}
						else if constexpr (Verbosity == VerbositySetting::Verbose)
						{
							std::string message = "Warning: In Command Line Parser\n >>>Ignoring unknown token \'" + key + "\' provided";
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
				throw std::invalid_argument("Error: In Command Line Parser\n >>>" + _programName + " requires that the, " + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments");
		}

		bool TryRaise(std::vector<std::string_view>::const_iterator& itr, const std::vector<std::string_view>::const_iterator end, std::string* errorMsg = nullptr) noexcept
		{
			//Acquire exclusive lock for write
			std::unique_lock<std::shared_mutex> writeLock(_sharedMutex);

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
							std::string msg = "Error: In Command Line Parser\n >>>Unknown token \'" + key + "\' provided";

							TERMINAL::PRINT_ERROR(msg);

							return false;
						}
						else if constexpr (Verbosity == VerbositySetting::Verbose)
						{
							std::string message = "Warning: In Command Line Parser\n >>>Ignoring unknown token \'" + key + "\' provided";
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
				std::string msg = "Error: In Command Line Parser\n >>>" + _programName + " requires that the, " + _requiredFlagsExpectedTokens + " flag tokens all be set with valid arguments";

				TERMINAL::PRINT_ERROR(msg);

				return false;
			}

			return true;
		}

		//Do not attempt to copy, CmdLineParser is a singleton
		CmdLineParser(const CmdLineParser&) = delete;
		//Do not attempt to copy, CmdLineParser is a singleton
		CmdLineParser& operator=(const CmdLineParser&) = delete;
		//Do not attempt to move, CmdLineParser is a singleton. Undefined behavior
		CmdLineParser(CmdLineParser&&) = delete;
		//Do not attempt to move, CmdLineParser is a singleton. Undefined behavior
		CmdLineParser& operator=(CmdLineParser&&) = delete;
	};
}
#endif // !GENTOOLS_GENPARSECMDLINE_CMDLINEPARSER_H

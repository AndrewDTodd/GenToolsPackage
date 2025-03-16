#ifndef GENTOOLS_GENPARSECMDLINE_COMMON_DEFINITIONS_H
#define GENTOOLS_GENPARSECMDLINE_COMMON_DEFINITIONS_H

#include <concepts>

#include <Flag.h>
#include <flag_argument_specializations.h>

namespace GenTools::GenParse
{
	/// <summary>
	/// Configure the parser's policy on irregularities in the arguments sequence it is parsing
	/// </summary>
	enum class ParsingMode
	{
		Strict,
		Lenient
	};

	/// <summary>
	/// Configures the parser's policy on exceptions produced from its sub-flags while parsing
	/// </summary>
	enum class ExceptionMode
	{
		Strict,
		Lenient
	};

	/// <summary>
	/// Configure the parser's logging behavior
	/// </summary>
	enum class VerbositySetting
	{
		Silent,
		Verbose
	};

	/// <summary>
	/// Configure the parser's behavior for position parsable sequences of arguments
	/// </summary>
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
}

#endif //!GENTOOLS_GENPARSECMDLINE_COMMON_DEFINITIONS_H
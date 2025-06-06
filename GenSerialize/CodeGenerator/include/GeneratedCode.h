#ifndef GENTOOLS_GENSERIALIZE_GENERATED_CODE_H
#define GENTOOLS_GENSERIALIZE_GENERATED_CODE_H

#include <string>
#include <unordered_map>

namespace GenTools::GenSerialize
{
	/// <summary>
	/// Holds generated serialization code for a given marked type. First key is type name, inner map has formats (keys) mapped to generated code (values)
	/// </summary>
	struct GeneratedCode
	{
		// Map from format name to generated C++ code
		std::unordered_map<std::string, std::unordered_map<std::string, std::string>> code;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_GENERATED_CODE_H

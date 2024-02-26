#ifndef PARSE_FUNCTIONS_H
#define PARSE_FUNCTIONS_H

#include <string>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include <cstdint>

namespace CmdLineParser
{
	//Parsing Functions
	//************************************************************************************************
	bool StringToBool(const char* str);

	bool YesNoToBool(const char* str);

	bool TrueFalseToBool(const char* str);

	bool TFToBool(const char* str);

	bool OneZeroToBool(const char* str);

	int32_t ParseInt32(const char* str);

	uint32_t ParseUInt32(const char* str);

	int64_t ParseInt64(const char* str);

	uint64_t ParseUInt64(const char* str);

	float ParseFloat(const char* str);

	double ParseDouble(const char* str);

	long double ParseLongDouble(const char* str);

	std::string ParseString(const char* str);
	//************************************************************************************************
}
#endif // !PARSE_FUNCTIONS_H

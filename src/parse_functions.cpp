#include <parse_functions.h>

namespace TokenValueParser
{
	bool StringToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"yes", true}, {"no", false},
			{"true", true}, {"false", false},
			{"y", true}, {"n", false},
			{"t", true}, {"f", false},
			{"1", true}, {"0", false},
			{"Yes", true}, {"No", false},
			{"True", true}, {"False", false},
			{"Y", true}, {"N", false},
			{"T", true}, {"F", false},
			{"YES", true}, {"NO", false},
			{"TRUE", true}, {"FALSE", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Error: Expected boolean parsable value. Could not parse \'" + s + "\'");
	}

	bool YesNoToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"yes", true}, {"no", false},
			{"Yes", true}, {"No", false},
			{"YES", true}, {"NO", false},
			{"y", true}, {"n", false},
			{"Y", true}, {"N", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Error: Expected yes or no value. Could not parse \'" + s + "\'");
	}

	bool TrueFalseToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"true", true}, {"false", false},
			{"True", true}, {"False", false},
			{"TRUE", true}, {"FALSE", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Error: Expected true or false. Could not parse \'" + s + "\'");
	}

	bool TFToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"t", true}, {"f", false},
			{"T", true}, {"F", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Error: Expected t or f. Could not parse \'" + s + "\'");
	}

	bool OneZeroToBool(const char* str)
	{
		static const std::unordered_map<std::string, bool> map = {
			{"1", true}, {"0", false}
		};

		std::string s(str);

		auto it = map.find(s);
		if (it != map.end())
			return it->second;
		else
			throw std::invalid_argument("Error: Expected 1 or 0. Could not parse \'" + s + "\'");
	}

	int32_t ParseInt32(const char* str)
	{
		std::string s(str);
		int32_t value = std::stoi(s, nullptr, 0);
		return value;
	}

	uint32_t ParseUInt32(const char* str)
	{
		if (str[0] == '-')
			throw std::out_of_range("Error: Cannot parse negative value into unsinged integer 32");

		std::string s(str);
		unsigned long value = std::stoul(s, nullptr, 0);
		if (value > UINT32_MAX)
			throw std::out_of_range("Error: Value " + std::to_string(value) + " is out of range for unsigned integer 32");
		return static_cast<uint32_t>(value);
	}

	int64_t ParseInt64(const char* str)
	{
		std::string s(str);
		long long value = std::stoll(s, nullptr, 0);
		return value;
	}

	uint64_t ParseUInt64(const char* str)
	{
		if (str[0] == '-')
			throw std::out_of_range("Error: Cannot parse negative value into an unsinged integer 64");

		std::string s(str);
		unsigned long long value = std::stoull(s, nullptr, 0);
		return value;
	}

	float ParseFloat(const char* str)
	{
		std::string s(str);
		float value = std::stof(s);
		return value;
	}

	double ParseDouble(const char* str)
	{
		std::string s(str);
		double value = std::stod(s);
		return value;
	}

	long double ParseLongDouble(const char* str)
	{
		std::string s(str);
		long double value = std::stold(s);
		return value;
	}

	std::string ParseStringDelimited(const char* str)
	{
		std::string s(str);
		if (s.size() < 2 || s.front() != '"' || s.back() != '"')
			throw std::invalid_argument("Error: Expecting string value delimited by \" characters. Could not parse \'" + s + "\'");
		return s.substr(1, s.size() - 2);
	}

	std::string ParseString(const char* str)
	{
		return std::string(str);
	}
}
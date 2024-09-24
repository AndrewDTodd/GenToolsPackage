#include <gtest/gtest.h>
#include <parse_functions.h>

#include <limits>
#include <sstream>
#include <string>
#include <iomanip>

using namespace GenTools::GenParse;

TEST(ParseFloat, Parse_dec)
{
	EXPECT_EQ(ParseFloat("3.14159"), 3.14159f);
	EXPECT_EQ(ParseFloat("-3.14159"), -3.14159f);
}

TEST(ParseFloat, Parse_scientific)
{
	EXPECT_EQ(ParseFloat("3.14159e-4"), 3.14159e-4f);
	EXPECT_EQ(ParseFloat("-3.14159e4"), -3.14159e4f);
}

TEST(ParseFloat, ThrowOnTooBig)
{
	std::ostringstream oss;
	oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::numeric_limits<float>::max();

	std::string str(std::move(oss.str()));
	str[0] += 1;

	EXPECT_THROW(ParseFloat(str.c_str()), std::out_of_range);
}

TEST(ParseFloat, ThrowOnTooSmall)
{
	std::ostringstream oss;
	oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::numeric_limits<float>::denorm_min();

	std::string str(std::move(oss.str()));
	str[0] -= 1;

	EXPECT_THROW(ParseFloat(str.c_str()), std::out_of_range);
}

TEST(ParseFloat, ThrowOnInvalid)
{
	EXPECT_THROW(ParseFloat("invalid"), std::invalid_argument);
}
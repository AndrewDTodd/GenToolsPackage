#include <gtest/gtest.h>
#include <parse_functions.h>

#include <limits>
#include <sstream>
#include <string>
#include <iomanip>

using namespace GenTools::GenParse;

TEST(ParseLongDouble, Parse_dec)
{
	EXPECT_EQ(ParseLongDouble("3.14159"), 3.14159L);
	EXPECT_EQ(ParseLongDouble("-3.14159"), -3.14159L);
}

TEST(ParseLongDouble, Parse_scientific)
{
	EXPECT_EQ(ParseLongDouble("3.14159e-4"), 3.14159e-4L);
	EXPECT_EQ(ParseLongDouble("-3.14159e4"), -3.14159e4L);
}

TEST(ParseLongDouble, ThrowOnTooBig)
{
	std::ostringstream oss;
	oss << std::setprecision(std::numeric_limits<long double>::max_digits10) << std::numeric_limits<long double>::max();

	std::string str(std::move(oss.str()));
	str[0] += 1;

	EXPECT_THROW(ParseLongDouble(str.c_str()), std::out_of_range);
}

TEST(ParseLongDouble, ThrowOnTooSmall)
{
	std::ostringstream oss;
	oss << std::setprecision(std::numeric_limits<long double>::max_digits10) << std::numeric_limits<long double>::denorm_min();

	std::string str(std::move(oss.str()));
	str[0] -= 3;

	EXPECT_THROW(ParseLongDouble(str.c_str()), std::out_of_range);
}

TEST(ParseLongDouble, ThrowOnInvalid)
{
	EXPECT_THROW(ParseLongDouble("invalid"), std::invalid_argument);
}
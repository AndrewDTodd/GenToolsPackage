#include <gtest/gtest.h>
#include <Flag.h>

#include <limits>
#include <sstream>
#include <string>

using namespace CmdLineParser;

TEST(ParseFloat, Parse_dec)
{
	std::ostringstream oss;
	oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::numeric_limits<float>::max();

	EXPECT_NEAR(ParseFloat(oss.str().c_str()), std::numeric_limits<float>::max(), std::abs(std::numeric_limits<float>::max() * 1e-10));

	oss.str("");
	oss.clear();
	oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::numeric_limits<float>::min();

	EXPECT_NEAR(ParseFloat(oss.str().c_str()), std::numeric_limits<float>::min(), std::abs(std::numeric_limits<float>::min() * 1e-10));
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
	oss << std::setprecision(std::numeric_limits<float>::max_digits10) << std::numeric_limits<float>::min();

	std::string str(std::move(oss.str()));
	str[0] -= 1;

	EXPECT_THROW(ParseFloat(str.c_str()), std::out_of_range);
}

TEST(ParseFloat, ThrowOnInvalid)
{
	EXPECT_THROW(ParseFloat("invalid"), std::invalid_argument);
}
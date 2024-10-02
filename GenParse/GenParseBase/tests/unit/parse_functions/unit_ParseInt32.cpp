#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace GenTools::GenParse;

TEST(ParseInt32, Parse_dec)
{
	EXPECT_EQ(ParseInt32("2147483647"), 2147483647);
	EXPECT_EQ(ParseInt32("-2147483648"), -2147483648);
}

TEST(ParseInt32, Parse_hex)
{
	EXPECT_EQ(ParseInt32("0xff"), 0xff);
	EXPECT_EQ(ParseInt32("-0xff"), -0xff);
	EXPECT_EQ(ParseInt32("0Xff"), 0Xff);
	EXPECT_EQ(ParseInt32("-0Xff"), -0Xff);
}

TEST(ParseInt32, ThrowOnTooBig)
{
	EXPECT_THROW(ParseInt32("2147483648"), std::out_of_range);
}

TEST(ParseInt32, ThrowOnTooSmall)
{
	EXPECT_THROW(ParseInt32("-2147483649"), std::out_of_range);
}

TEST(ParseInt32, ThrowOnInvalid)
{
	EXPECT_THROW(ParseInt32("invalid"), std::invalid_argument);
}
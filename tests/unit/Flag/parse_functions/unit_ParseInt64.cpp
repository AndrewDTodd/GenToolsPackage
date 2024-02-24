#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace CmdLineParser;

TEST(ParseInt64, Parse_dec)
{
	EXPECT_EQ(ParseInt64("9223372036854775807"), 9223372036854775807LL);
	EXPECT_EQ(ParseInt64("-9223372036854775806"), -9223372036854775806LL);
}

TEST(ParseInt64, Parse_hex)
{
	EXPECT_EQ(ParseInt64("0xff"), 0xff);
	EXPECT_EQ(ParseInt64("-0xff"), -0xff);
	EXPECT_EQ(ParseInt64("0Xff"), 0Xff);
	EXPECT_EQ(ParseInt64("-0Xff"), -0Xff);
}

TEST(ParseInt64, ThrowOnTooBig)
{
	EXPECT_THROW(ParseInt64("9223372036854775808"), std::out_of_range);
}

TEST(ParseInt64, ThrowOnTooSmall)
{
	EXPECT_THROW(ParseInt64("-9223372036854775809"), std::out_of_range);
}

TEST(ParseInt64, ThrowOnInvalid)
{
	EXPECT_THROW(ParseInt64("invalid"), std::invalid_argument);
}
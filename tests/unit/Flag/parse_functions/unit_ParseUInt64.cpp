#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace TokenValueParser;

TEST(ParseUInt64, Parse_dec)
{
	EXPECT_EQ(ParseUInt64("18446744073709551615"), 0xffffffffffffffff);
	EXPECT_EQ(ParseUInt64("0"), 0);
}

TEST(ParseUInt64, Parse_hex)
{
	EXPECT_EQ(ParseUInt64("0xff"), 0xff);
	EXPECT_EQ(ParseUInt64("0x00"), 0x00);
	EXPECT_EQ(ParseUInt64("0Xff"), 0Xff);
	EXPECT_EQ(ParseUInt64("0X00"), 0X00);
}

TEST(ParseUInt64, ThrowOnTooBig)
{
	EXPECT_THROW(ParseUInt64("18446744073709551616"), std::out_of_range);
}

TEST(ParseUInt64, ThrowOnTooSmall)
{
	EXPECT_THROW(ParseUInt64("-1"), std::out_of_range);
}

TEST(ParseUInt64, ThrowOnInvalid)
{
	EXPECT_THROW(ParseUInt64("invalid"), std::invalid_argument);
}
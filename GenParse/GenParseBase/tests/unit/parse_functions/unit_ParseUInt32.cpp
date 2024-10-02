#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace GenTools::GenParse;

TEST(ParseUInt32, Parse_dec)
{
	EXPECT_EQ(ParseUInt32("4294967295"), 0xffffffff);
	EXPECT_EQ(ParseUInt32("0"), 0);
}

TEST(ParseUInt32, Parse_hex)
{
	EXPECT_EQ(ParseUInt32("0xff"), 0xff);
	EXPECT_EQ(ParseUInt32("0x00"), 0x00);
	EXPECT_EQ(ParseUInt32("0Xff"), 0Xff);
	EXPECT_EQ(ParseUInt32("0X00"), 0X00);
}

TEST(ParseUInt32, ThrowOnTooBig)
{
	EXPECT_THROW(ParseUInt32("4294967296"), std::out_of_range);
}

TEST(ParseUInt32, ThrowOnTooSmall)
{
	EXPECT_THROW(ParseUInt32("-1"), std::out_of_range);
}

TEST(ParseUInt32, ThrowOnInvalid)
{
	EXPECT_THROW(ParseUInt32("invalid"), std::invalid_argument);
}
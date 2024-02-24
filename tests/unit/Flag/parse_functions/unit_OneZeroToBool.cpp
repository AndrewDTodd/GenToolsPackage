#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace CmdLineParser;

TEST(OneZeroToBool, Parse_1)
{
	EXPECT_TRUE(OneZeroToBool("1"));
}

TEST(OneZeroToBool, Parse_0)
{
	EXPECT_FALSE(OneZeroToBool("0"));
}

TEST(OneZeroToBool, ThrowOnInvalid)
{
	EXPECT_THROW(OneZeroToBool("true"), std::invalid_argument);
	EXPECT_THROW(OneZeroToBool("invalid"), std::invalid_argument);
}
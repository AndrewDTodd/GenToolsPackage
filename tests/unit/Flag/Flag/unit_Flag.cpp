#include <gtest/gtest.h>
#include <Flag.h>

using namespace CmdLineParser;

TEST(FlagConstructor1, VerifyShortToken)
{
	Flag flag({ "d" }, "Test flag description");
	EXPECT_EQ(flag.ShortToken(), "-d");
}

TEST(FlagConstructor1, VerifyLongToken)
{
	Flag flag{ { "debug" }, "Test flag description" };
	EXPECT_EQ(flag.LongTokens()[0], "--debug");
}

TEST(FlagConstructor2, VerifyShortToken)
{
	Flag flag({ "d", "debug" }, "Enable debug mode");
	EXPECT_EQ(flag.ShortToken(), "-d");
}
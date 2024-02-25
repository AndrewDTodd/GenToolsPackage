#include <gtest/gtest.h>
#include <Flag.h>

using namespace CmdLineParser;

TEST(FlagConstructor1, VerifyShortToken)
{
	Flag flag{ std::string("d"), std::string("Test flag description") };
	EXPECT_EQ(flag.ShortToken(), "-d");
}

TEST(FlagConstructor1, VerifyLongToken)
{
	Flag flag{ std::string("debug"), std::string("Test flag description") };
	EXPECT_EQ(flag.LongTokens()[0], "--debug");
}
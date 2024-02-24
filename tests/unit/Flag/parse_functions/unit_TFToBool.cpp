#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace CmdLineParser;

TEST(TFToBool, Parse_t)
{
	EXPECT_TRUE(TFToBool("t"));
}

TEST(TFToBool, Parse_f)
{
	EXPECT_FALSE(TFToBool("f"));
}

TEST(TFToBool, Parse_T)
{
	EXPECT_TRUE(TFToBool("T"));
}

TEST(TFToBool, Parse_F)
{
	EXPECT_FALSE(TFToBool("F"));
}

TEST(TFToBool, ThrowOnInvalid)
{
	EXPECT_THROW(TFToBool("true"), std::invalid_argument);
	EXPECT_THROW(TFToBool("invalid"), std::invalid_argument);
}
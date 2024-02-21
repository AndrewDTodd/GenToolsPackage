#include <gtest/gtest.h>
#include <Flag.h>

using namespace CmdLineParser;

TEST(TrueFalseToBool, Parse_true)
{
	EXPECT_TRUE(TrueFalseToBool("true"));
}

TEST(TrueFalseToBool, Parse_false)
{
	EXPECT_FALSE(TrueFalseToBool("false"));
}

TEST(TrueFalseToBool, Parse_True)
{
	EXPECT_TRUE(TrueFalseToBool("True"));
}

TEST(TrueFalseToBool, Parse_False)
{
	EXPECT_FALSE(TrueFalseToBool("False"));
}

TEST(TrueFalseToBool, Parse_TRUE)
{
	EXPECT_TRUE(TrueFalseToBool("TRUE"));
}

TEST(TrueFalseToBool, Parse_FALSE)
{
	EXPECT_FALSE(TrueFalseToBool("FALSE"));
}

TEST(TrueFalseToBool, ThrowOnInvalid)
{
	EXPECT_THROW(TrueFalseToBool("yes"), std::invalid_argument);
	EXPECT_THROW(TrueFalseToBool("invalid"), std::invalid_argument);
}
#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace CmdLineParser;

TEST(StringToBool, Parse_yes)
{
	EXPECT_TRUE(StringToBool("yes"));
}

TEST(StringToBool, Parse_no)
{
	EXPECT_FALSE(StringToBool("no"));
}

TEST(StringToBool, Parse_true)
{
	EXPECT_TRUE(StringToBool("true"));
}

TEST(StringToBool, Parse_false)
{
	EXPECT_FALSE(StringToBool("false"));
}

TEST(StringToBool, Parse_t)
{
	EXPECT_TRUE(StringToBool("t"));
}

TEST(StringToBool, Parse_f)
{
	EXPECT_FALSE(StringToBool("f"));
}

TEST(StringToBool, Parse_1)
{
	EXPECT_TRUE(StringToBool("1"));
}

TEST(StringToBool, Parse_0)
{
	EXPECT_FALSE(StringToBool("0"));
}

TEST(StringToBool, Parse_Yes)
{
	EXPECT_TRUE(StringToBool("Yes"));
}

TEST(StringToBool, Parse_No)
{
	EXPECT_FALSE(StringToBool("No"));
}

TEST(StringToBool, Parse_True)
{
	EXPECT_TRUE(StringToBool("True"));
}

TEST(StringToBool, Parse_False)
{
	EXPECT_FALSE(StringToBool("False"));
}

TEST(StringToBool, Parse_T)
{
	EXPECT_TRUE(StringToBool("T"));
}

TEST(StringToBool, Parse_F)
{
	EXPECT_FALSE(StringToBool("F"));
}

TEST(StringToBool, Parse_YES)
{
	EXPECT_TRUE(StringToBool("YES"));
}

TEST(StringToBool, Parse_NO)
{
	EXPECT_FALSE(StringToBool("NO"));
}

TEST(StringToBool, Parse_TRUE)
{
	EXPECT_TRUE(StringToBool("TRUE"));
}

TEST(StringToBool, Parse_FALSE)
{
	EXPECT_FALSE(StringToBool("FALSE"));
}

TEST(StringToBool, ThrowOnInvalid)
{
	EXPECT_THROW(StringToBool("invalid"), std::invalid_argument);
}
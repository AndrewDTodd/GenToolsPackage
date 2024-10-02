#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace GenTools::GenParse;

TEST(YesNoToBool, Parse_yes)
{
	EXPECT_TRUE(YesNoToBool("yes"));
}

TEST(YesNoToBool, Parse_no)
{
	EXPECT_FALSE(YesNoToBool("no"));
}

TEST(YesNoToBool, Parse_Yes)
{
	EXPECT_TRUE(YesNoToBool("Yes"));
}

TEST(YesNoToBool, Parse_No)
{
	EXPECT_FALSE(YesNoToBool("No"));
}

TEST(YesNoToBool, Parse_YES)
{
	EXPECT_TRUE(YesNoToBool("YES"));
}

TEST(YesNoToBool, Parse_NO)
{
	EXPECT_FALSE(YesNoToBool("NO"));
}

TEST(YesNoToBool, ThrowOnInvalid)
{
	EXPECT_THROW(YesNoToBool("true"), std::invalid_argument);
	EXPECT_THROW(YesNoToBool("invalid"), std::invalid_argument);
}
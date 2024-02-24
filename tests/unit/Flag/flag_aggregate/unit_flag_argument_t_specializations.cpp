#include <gtest/gtest.h>
#include <flag_argument_specializations.h>

using namespace CmdLineParser;

TEST(Arg_Bool, ParsesTrueString)
{
	Arg_Bool arg;
	ASSERT_NO_THROW(arg.Parse("true"));
	EXPECT_TRUE(arg.as<bool>());
}

TEST(Arg_Bool, ParsesFalseString)
{
	Arg_Bool arg;
	ASSERT_NO_THROW(arg.Parse("false"));
	EXPECT_FALSE(arg.as<bool>());
}

TEST(Arg_Bool, ThrowsForInvalidInput)
{
	Arg_Bool arg;
	EXPECT_THROW(arg.Parse("invalid"), std::invalid_argument);
}

TEST(Arg_Bool, TryParseTrueString)
{
	Arg_Bool arg;
	ASSERT_TRUE(arg.TryParse("true"));
	EXPECT_TRUE(arg.as<bool>());
}

TEST(Arg_Bool, TryParseFalseString)
{
	Arg_Bool arg;
	ASSERT_TRUE(arg.TryParse("false"));
	EXPECT_FALSE(arg.as<bool>());
}

TEST(Arg_Bool, TryParseInvalidInput)
{
	Arg_Bool arg;
	EXPECT_FALSE(arg.TryParse("invalid"));
}
#include <gtest/gtest.h>
#include <Flag.h>

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
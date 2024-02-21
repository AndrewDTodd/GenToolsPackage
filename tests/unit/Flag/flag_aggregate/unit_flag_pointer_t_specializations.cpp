#include <gtest/gtest.h>
#include <Flag.h>

using namespace CmdLineParser;

TEST(Ptr_Bool, ParsesTrueString)
{
	bool test_bool = false;
	Ptr_Bool ptr;
	ptr.SetLinkedValue(&test_bool);
	EXPECT_EQ(&test_bool, ptr.as<bool*>());
	ASSERT_NO_THROW(ptr.Parse("true"));
	EXPECT_TRUE(test_bool);
}

TEST(Ptr_Bool, ParseFalseString)
{
	bool test_bool = true;
	Ptr_Bool ptr(&test_bool);
	EXPECT_EQ(&test_bool, ptr.as<bool*>());
	ASSERT_NO_THROW(ptr.Parse("false"));
	EXPECT_FALSE(test_bool);
}

TEST(Ptr_Bool, ThrowsForInvalidInput)
{
	bool test_bool;
	Ptr_Bool ptr(&test_bool);
	EXPECT_THROW(ptr.Parse("invalid"), std::invalid_argument);
}

TEST(Ptr_Bool, TryParseTrueString)
{
	bool test_bool = false;
	Ptr_Bool ptr;
	ptr.SetLinkedValue(&test_bool);
	EXPECT_EQ(&test_bool, ptr.as<bool*>());
	ASSERT_TRUE(ptr.TryParse("true"));
	EXPECT_TRUE(test_bool);
}

TEST(Ptr_Bool, TryParseFalseString)
{
	bool test_bool = true;
	Ptr_Bool ptr(&test_bool);
	EXPECT_EQ(&test_bool, ptr.as<bool*>());
	ASSERT_TRUE(ptr.TryParse("false"));
	EXPECT_FALSE(test_bool);
}

TEST(Ptr_Bool, TryParseInvalidInput)
{
	bool test_bool;
	Ptr_Bool ptr(&test_bool);
	EXPECT_FALSE(ptr.TryParse("invalid"));
}
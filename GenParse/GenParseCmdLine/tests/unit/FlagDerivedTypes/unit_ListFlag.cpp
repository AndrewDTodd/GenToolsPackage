#include <gtest/gtest.h>
#include <ListFlag.h>

#include <flag_argument_specializations.h>

using namespace GenTools::GenParse;

// Test case 1:
// Non-enclosed, non-separated: tokens are collected until a token that looks like a flag is encountered
TEST(ListFlagTests, NonEnclosed)
{
	std::vector<std::string_view> tokens = { "FileOne.h", "FileTwo.h", "FileThree.h", "--anotherFlag" };
	auto itr = tokens.begin();

	ListFlag<Arg_String, false, false> listFlag({ "file_names" }, "List of file names");

	// Call Raise; it should gather tokens until a token starting with '-' is encountered
	listFlag.Raise(itr, tokens.end());

	const auto& values = listFlag.FlagArgument().as<std::vector<std::string>>();
	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], "FileOne.h");
	EXPECT_EQ(values[1], "FileTwo.h");
	EXPECT_EQ(values[2], "FileThree.h");

	// Iterator should now point to "--anotherFlag"
	ASSERT_NE(itr, tokens.end());
	EXPECT_EQ(std::string(*itr), "--anotherFlag");
}

// Test case 2:
// Enclosed, separated mode with a single token
// The list is given as a single token, enclosed in '[' and ']', with items separated by commas
TEST(ListFlagTests, EnclosedSeperatedSingleToken)
{
	std::vector<std::string_view> tokens = { "[FileOne.h, FileTwo.h, FileThree.h]", "--flag" };
	auto it = tokens.begin();

	ListFlag<Arg_String, true, true> listFlag(Tokens("file_names"), "List of file names", true, false);
	listFlag.Raise(it, tokens.end());

	const auto& values = listFlag.FlagArgument().as<std::vector<std::string>>();
	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], "FileOne.h");
	EXPECT_EQ(values[1], "FileTwo.h");
	EXPECT_EQ(values[2], "FileThree.h");

	// The iterator should now point to "--flag"
	ASSERT_NE(it, tokens.end());
	EXPECT_EQ(std::string(*it), "--flag");
}

// Test case 3:
// Enclosed mode (non-separated) with multiple tokens
// The list is provided in several tokens where the first token starts with '[' and the last token ends with ']'
TEST(ListFlagTests, EnclosedMultiToken)
{
	std::vector<std::string_view> tokens = { "[FileOne.h", "FileTwo.h", "FileThree.h]", "--next" };
	auto it = tokens.begin();

	ListFlag<Arg_String, true, false> listFlag(Tokens("file_names"), "List of file names", true, false);
	listFlag.Raise(it, tokens.end());

	const auto& values = listFlag.FlagArgument().as<std::vector<std::string>>();
	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], "FileOne.h");
	EXPECT_EQ(values[1], "FileTwo.h");
	EXPECT_EQ(values[2], "FileThree.h");

	// The iterator should now point to "--next"
	ASSERT_NE(it, tokens.end());
	EXPECT_EQ(std::string(*it), "--next");
}

// Test case 4:
// Enclosed mode (non-separated) with multiple tokens
// The list is provided in several tokens where the first token is '[' and the last token is ']'
TEST(ListFlagTests, EnclosedMultiTokenBracketsSeperated)
{
	std::vector<std::string_view> tokens = { "[", "FileOne.h", "FileTwo.h", "FileThree.h", "]", "--next" };
	auto it = tokens.begin();

	ListFlag<Arg_String, true, false> listFlag(Tokens("file_names"), "List of file names", true, false);
	listFlag.Raise(it, tokens.end());

	const auto& values = listFlag.FlagArgument().as<std::vector<std::string>>();
	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], "FileOne.h");
	EXPECT_EQ(values[1], "FileTwo.h");
	EXPECT_EQ(values[2], "FileThree.h");

	// The iterator should now point to "--next"
	ASSERT_NE(it, tokens.end());
	EXPECT_EQ(std::string(*it), "--next");
}

// Test case 5:
// Enclosed mode (separated) with multiple tokens
// The list is provided in several tokens where the first token is '[' and the last token is ']'
TEST(ListFlagTests, EnclosedMultiTokenSeparatedBracketsSeperated)
{
	std::vector<std::string_view> tokens = { "[", "FileOne.h,", "FileTwo.h,", "FileThree.h", "]", "--next" };
	auto it = tokens.begin();

	ListFlag<Arg_String, true, true> listFlag(Tokens("file_names"), "List of file names", true, false);
	listFlag.Raise(it, tokens.end());

	const auto& values = listFlag.FlagArgument().as<std::vector<std::string>>();
	ASSERT_EQ(values.size(), 3);
	EXPECT_EQ(values[0], "FileOne.h");
	EXPECT_EQ(values[1], "FileTwo.h");
	EXPECT_EQ(values[2], "FileThree.h");

	// The iterator should now point to "--next"
	ASSERT_NE(it, tokens.end());
	EXPECT_EQ(std::string(*it), "--next");
}

// Test case 6:
// Enclosed mode with a missing closing delimiter should cause TryRaise to fail.
TEST(ListFlagTests, EnclosedMissingClose)
{
	std::vector<std::string_view> tokens = { "[FileOne.h", "FileTwo.h", "FileThree.h", "--flag" };
	auto it = tokens.begin();

	ListFlag<Arg_String, true, false> listFlag(Tokens("file_names"), "List of file names", true, false);
	std::string errorMsg;
	bool result = listFlag.TryRaise(it, tokens.end(), &errorMsg);
	EXPECT_FALSE(result);
	EXPECT_NE(errorMsg.find("Expected closing ']'"), std::string::npos);
}
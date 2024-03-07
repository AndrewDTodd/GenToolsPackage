#include <gtest/gtest.h>
#include <BranchFlag.h>

#include <flag_argument_specializations.h>

using namespace CmdLineParser;

TEST(BranchFlagConstructor1, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag<Arg_Bool>("out", "The file to output the operation too"));
}

TEST(BranchFlagConstructor1, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag<Arg_Bool>("o", "The file to output the operation too"), std::invalid_argument);
#endif // _DEBUG
#if RELEASE_ERROR_MSG
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag<Arg_Bool>("o", "The file to output the operation too"));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#elif !defined(_DEBUG) && !RELEASE_ERROR_MSG
	EXPECT_NO_THROW(BranchFlag<Arg_Bool>("o", "The file to output the operation too"));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor2, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag<Arg_Bool>("commit", "commit the staged changes", false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
}

TEST(BranchFlagConstructor2, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag<Arg_Bool>("c", "commit the staged changes", false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())), std::invalid_argument);
#endif // _DEBUG
#if RELEASE_ERROR_MSG
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag<Arg_Bool>("c", "commit the staged changes", false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#elif !defined(_DEBUG) && !RELEASE_ERROR_MSG
	EXPECT_NO_THROW(BranchFlag<Arg_Bool>("c", "commit the staged changes", false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor2, VerifySubFlags)
{
	BranchFlag<Arg_Bool> branch("commit", "commit the staged changes", false, Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String() });
	auto& _nestedFlags = branch.NestedFlags();

	EXPECT_TRUE(_nestedFlags.size() == 1);
	EXPECT_EQ(_nestedFlags[0]->FlagDescription(), "The message to associate with the commit");
}

TEST(BranchFlagConstructor3, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag("commit", "commit the staged changes", Arg_Bool(), false, false, Flag({"m", "msg", "message"}, "The message to associate with the commit", Arg_String())));
}

TEST(BranchFlagConstructor3, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag("c", "commit the staged changes", Arg_Bool(), false, false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())), std::invalid_argument);
#endif // _DEBUG
#if RELEASE_ERROR_MSG
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag("c", "commit the staged changes", Arg_Bool(), false, false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#elif !defined(_DEBUG) && !RELEASE_ERROR_MSG
	EXPECT_NO_THROW(BranchFlag<Arg_Bool>("c", "commit the staged changes", Arg_Bool(), false, false, Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor3, VerifySubFlags)
{
	BranchFlag branch("commit", "commit the staged changes", Arg_Bool(), false, false, Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String() });
	auto& _nestedFlags = branch.NestedFlags();

	EXPECT_TRUE(_nestedFlags.size() == 1);
	EXPECT_EQ(_nestedFlags[0]->FlagDescription(), "The message to associate with the commit");
}

TEST(BranchFlagMoveConstructor, ValidateMove)
{
	BranchFlag flagOne("commit", "commit the staged changes", Arg_Bool(), false, false, Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String() });
	
	BranchFlag flagTwo(std::move(flagOne));

	auto& _nestedFlagsOne = flagOne.NestedFlags();
	auto& _nestedFlagsTwo = flagTwo.NestedFlags();

	EXPECT_TRUE(_nestedFlagsOne.size() == 0);

	EXPECT_TRUE(_nestedFlagsTwo.size() == 1);
	EXPECT_EQ(_nestedFlagsTwo[0]->FlagDescription(), "The message to associate with the commit");
}

TEST(BranchFlagMoveAssignment, ValidateMove)
{
	BranchFlag flagOne("commit", "commit the staged changes", Arg_Bool(), false, false, Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String() });

	BranchFlag<Arg_Bool> flagTwo("temp", "Temp Description");
	flagTwo = std::move(flagOne);

	auto& _nestedFlagsOne = flagOne.NestedFlags();
	auto& _nestedFlagsTwo = flagTwo.NestedFlags();

	EXPECT_TRUE(_nestedFlagsOne.size() == 0);

	EXPECT_TRUE(_nestedFlagsTwo.size() == 1);
	EXPECT_EQ(_nestedFlagsTwo[0]->FlagDescription(), "The message to associate with the commit");
}

TEST(BranchFlagMethods, SetSubFlags)
{
	BranchFlag flag("commit", "commit the staged changes", Arg_Bool());

	flag.SetSubFlags(Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String() });
	auto& _nestedFlags = flag.NestedFlags();

	EXPECT_TRUE(_nestedFlags.size() == 1);
	EXPECT_EQ(_nestedFlags[0]->FlagDescription(), "The message to associate with the commit");
}

TEST(BranchFlagRaise, VerifySuccessOnValid)
{
	const char* cmdArgs[] = { "log.exe", "commit", "-m", "Commit message"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), true);

	ASSERT_NO_THROW(flag.Raise(itr, args.end()));

	EXPECT_EQ(itr, args.end());
	EXPECT_EQ(flag.FlagArgument().as<std::string>(), "/.ssh/named_key.pub");
}
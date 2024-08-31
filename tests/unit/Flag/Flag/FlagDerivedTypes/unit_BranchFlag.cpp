#include <gtest/gtest.h>
#include <BranchFlag.h>

#include <flag_argument_specializations.h>

using namespace TokenValueParser;

TEST(BranchFlagConstructor1, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag("out", "The file to output the operation too"));
}

TEST(BranchFlagConstructor1, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag("o", "The file to output the operation too"), std::invalid_argument);
#endif // _DEBUG
#if RELEASE_ERROR_MSG
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag("o", "The file to output the operation too"));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#elif !defined(_DEBUG) && !RELEASE_ERROR_MSG
	EXPECT_NO_THROW(BranchFlag("o", "The file to output the operation too"));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor2, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag("commit", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
}

TEST(BranchFlagConstructor2, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag("c", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())), std::invalid_argument);
#endif // _DEBUG
#if RELEASE_ERROR_MSG
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag("c", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#elif !defined(_DEBUG) && !RELEASE_ERROR_MSG
	EXPECT_NO_THROW(BranchFlag("c", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor2, VerifySubFlags)
{
	Flag posParseable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_String() };
	posParseable.SetFlagIsPosParsable(true);

	BranchFlag branch("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParseable));

	auto& _optionalFlags = branch.OptionalFlags();
	auto& _requiredFlags = branch.RequiredFlags();
	auto& _posParseableFlags = branch.PosParseableFlags();

	EXPECT_TRUE(_optionalFlags.size() == 4);
	ASSERT_TRUE(_optionalFlags.contains("-d"));
	ASSERT_TRUE(_optionalFlags.contains("--debug"));
	ASSERT_TRUE(_optionalFlags.contains("--debug-mode"));
	auto flagOne = _optionalFlags.at("-d");
	EXPECT_EQ(flagOne->FlagDescription(), "Set the operation to perform debug features");

	ASSERT_TRUE(_optionalFlags.contains("--help"));

	EXPECT_TRUE(_requiredFlags.size() == 3);
	ASSERT_TRUE(_requiredFlags.contains("-m"));
	ASSERT_TRUE(_requiredFlags.contains("--msg"));
	ASSERT_TRUE(_requiredFlags.contains("--message"));
	auto flagTwo = _requiredFlags.at("-m");
	EXPECT_EQ(flagTwo->FlagDescription(), "The message to associate with the commit");

	EXPECT_TRUE(_posParseableFlags.size() == 1);
	EXPECT_EQ(_posParseableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
}

TEST(BranchFlagMoveConstructor, ValidateMove)
{
	Flag posParseable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_String() };
	posParseable.SetFlagIsPosParsable(true);

	BranchFlag flagOne("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParseable));

	BranchFlag flagTwo(std::move(flagOne));

	auto& _optionalFlagsOne = flagOne.OptionalFlags();
	auto& _requiredFlagsOne = flagOne.RequiredFlags();
	auto& _posParseableFlagsOne = flagOne.PosParseableFlags();

	auto& _optionalFlagsTwo = flagTwo.OptionalFlags();
	auto& _requiredFlagsTwo = flagTwo.RequiredFlags();
	auto& _posParseableFlagsTwo = flagTwo.PosParseableFlags();

	EXPECT_TRUE(_optionalFlagsOne.size() == 0);
	EXPECT_TRUE(_requiredFlagsOne.size() == 0);
	EXPECT_TRUE(_posParseableFlagsOne.size() == 0);

	EXPECT_TRUE(_optionalFlagsTwo.size() == 4);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParseableFlagsTwo.size() == 1);
}

TEST(BranchFlagMoveAssignment, ValidateMove)
{
	Flag posParseable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_String() };
	posParseable.SetFlagIsPosParsable(true);

	BranchFlag flagOne("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParseable));

	BranchFlag flagTwo = std::move(flagOne);

	auto& _optionalFlagsOne = flagOne.OptionalFlags();
	auto& _requiredFlagsOne = flagOne.RequiredFlags();
	auto& _posParseableFlagsOne = flagOne.PosParseableFlags();

	auto& _optionalFlagsTwo = flagTwo.OptionalFlags();
	auto& _requiredFlagsTwo = flagTwo.RequiredFlags();
	auto& _posParseableFlagsTwo = flagTwo.PosParseableFlags();

	EXPECT_TRUE(_optionalFlagsOne.size() == 0);
	EXPECT_TRUE(_requiredFlagsOne.size() == 0);
	EXPECT_TRUE(_posParseableFlagsOne.size() == 0);

	EXPECT_TRUE(_optionalFlagsTwo.size() == 4);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParseableFlagsTwo.size() == 1);
}

TEST(BranchFlagMethods, SetSubFlags)
{
	BranchFlag branch("test", "Test branch");

	Flag posParseable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_String() };
	posParseable.SetFlagIsPosParsable(true);

	branch.SetSubFlags(Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParseable));

	auto& _optionalFlags = branch.OptionalFlags();
	auto& _requiredFlags = branch.RequiredFlags();
	auto& _posParseableFlags = branch.PosParseableFlags();

	EXPECT_TRUE(_optionalFlags.size() == 4);
	ASSERT_TRUE(_optionalFlags.contains("-d"));
	ASSERT_TRUE(_optionalFlags.contains("--debug"));
	ASSERT_TRUE(_optionalFlags.contains("--debug-mode"));
	auto flagOne = _optionalFlags.at("-d");
	EXPECT_EQ(flagOne->FlagDescription(), "Set the operation to perform debug features");

	ASSERT_TRUE(_optionalFlags.contains("--help"));

	EXPECT_TRUE(_requiredFlags.size() == 3);
	ASSERT_TRUE(_requiredFlags.contains("-m"));
	ASSERT_TRUE(_requiredFlags.contains("--msg"));
	ASSERT_TRUE(_requiredFlags.contains("--message"));
	auto flagTwo = _requiredFlags.at("-m");
	EXPECT_EQ(flagTwo->FlagDescription(), "The message to associate with the commit");

	EXPECT_TRUE(_posParseableFlags.size() == 1);
	EXPECT_EQ(_posParseableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
}

TEST(BranchFlagRaise, VerifySuccessOnValid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "-f", "/.ssh/named_key" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr ++;

	BranchFlag branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	EXPECT_EQ(itr, args.end());
	EXPECT_EQ(branch.OptionalFlags().at("-f")->FlagArgument().as<std::string>(), "/.ssh/named_key");
}
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

	EXPECT_TRUE(_optionalFlags.size() == 6);
	ASSERT_TRUE(_optionalFlags.contains("-d"));
	ASSERT_TRUE(_optionalFlags.contains("--debug"));
	ASSERT_TRUE(_optionalFlags.contains("--debug-mode"));
	ASSERT_TRUE(_optionalFlags.contains("-r"));
	ASSERT_TRUE(_optionalFlags.contains("--recipient"));
	auto& flagOne = _optionalFlags.at("-d");
	EXPECT_EQ(flagOne->FlagDescription(), "Set the operation to perform debug features");

	ASSERT_TRUE(_optionalFlags.contains("--help"));

	EXPECT_TRUE(_requiredFlags.size() == 3);
	ASSERT_TRUE(_requiredFlags.contains("-m"));
	ASSERT_TRUE(_requiredFlags.contains("--msg"));
	ASSERT_TRUE(_requiredFlags.contains("--message"));
	auto& flagTwo = _requiredFlags.at("-m");
	EXPECT_EQ(flagTwo->FlagDescription(), "The message to associate with the commit");

	EXPECT_TRUE(_posParseableFlags.size() == 1);
	EXPECT_EQ(_posParseableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
}

#pragma warning(push)
#pragma warning(disable:26800)
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

	EXPECT_TRUE(_optionalFlagsTwo.size() == 6);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParseableFlagsTwo.size() == 1);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:26800)
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

	EXPECT_TRUE(_optionalFlagsTwo.size() == 6);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParseableFlagsTwo.size() == 1);
}
#pragma warning(pop)

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

	EXPECT_TRUE(_optionalFlags.size() == 6);
	ASSERT_TRUE(_optionalFlags.contains("-d"));
	ASSERT_TRUE(_optionalFlags.contains("--debug"));
	ASSERT_TRUE(_optionalFlags.contains("--debug-mode"));
	ASSERT_TRUE(_optionalFlags.contains("-r"));
	ASSERT_TRUE(_optionalFlags.contains("--recipient"));
	auto& flagOne = _optionalFlags.at("-d");
	EXPECT_EQ(flagOne->FlagDescription(), "Set the operation to perform debug features");

	ASSERT_TRUE(_optionalFlags.contains("--help"));

	EXPECT_TRUE(_requiredFlags.size() == 3);
	ASSERT_TRUE(_requiredFlags.contains("-m"));
	ASSERT_TRUE(_requiredFlags.contains("--msg"));
	ASSERT_TRUE(_requiredFlags.contains("--message"));
	auto& flagTwo = _requiredFlags.at("-m");
	EXPECT_EQ(flagTwo->FlagDescription(), "The message to associate with the commit");

	EXPECT_TRUE(_posParseableFlags.size() == 1);
	EXPECT_EQ(_posParseableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifySuccessOnValid_OptionalFlag)
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

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: Ignoring unknown token \'--object\' provided\n");
}

TEST(StrictStrictVerbose_BranchFlag_Raise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: Ignoring unknown token \'--object\' provided\n");
}

TEST(StrictStrictSilent_BranchFlag_Raise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict, ExceptionMode::Strict, VerbositySetting::Silent> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "");
}

TEST(LenientStrictSilent_BranchFlag_Raise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Silent> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "");
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyFailOnInvalidFlagArg_OptionalFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(LenientLenientVerbose_BranchFlag_Raise, VerifyFailOnInvalidFlagArg_OptionalFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();
	std::string truncOutput = output.substr(0, 107);

	EXPECT_EQ(truncOutput, "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'-w\'.\nError: ");
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyFailOnInvalidFlagArg_RequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(LenientLenientVerbose_BranchFlag_Raise, VerifyFailOnInvalidFlagArg_RequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();
	std::string truncOutput = output.substr(0, 107);

	EXPECT_EQ(truncOutput, "Warning: Ignoring parsing error from options branch sub-option. Thrown from option with token \'-w\'.\nError: ");
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyValidPosParsable)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "}"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyValidPosParsable_LimitedSet_SpecificsPrepend)
{
	const char* cmdArgs[] = { "terrainGen.exe", "--strength", ".56", "-i", "2", "{", "55.6", "234.78", "43.0", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyValidPosParsable_LimitedSet_SpecificsApend)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", "}", "--strength", ".56", "-i", "2" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));
}
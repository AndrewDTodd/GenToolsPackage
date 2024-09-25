#include <gtest/gtest.h>
#include <BranchFlag.h>

#include <flag_argument_specializations.h>

using namespace GenTools::GenParse;

TEST(BranchFlagConstructor1, VerifyValidInput)
{
	EXPECT_NO_THROW(BranchFlag("out", "The file to output the operation too"));
}

TEST(BranchFlagConstructor1, VerifyThrowOnInvalid)
{
#ifdef _DEBUG
	EXPECT_THROW(BranchFlag("o", "The file to output the operation too"), std::invalid_argument);
#endif // _DEBUG
#ifdef _RELEASE_DEV
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag("o", "The file to output the operation too"));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with description 'The file to output the operation too'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#endif
#ifdef _RELEASE
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
#ifdef _RELEASE_DEV
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(BranchFlag("c", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with description 'commit the staged changes'\n >>>A Branch Flag should have one recognizable token that is a long token, not a single character short token\n");
#endif
#ifdef _RELEASE
	EXPECT_NO_THROW(BranchFlag("c", "commit the staged changes", Flag({ "m", "msg", "message" }, "The message to associate with the commit", Arg_String())));
#endif // RELEASE_ERROR_MSG
}

TEST(BranchFlagConstructor2, VerifySubFlags)
{
	Flag posParsable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_String() };
	posParsable.SetFlagIsPosParsable(true);

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	BranchFlag branch("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParsable));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

#if defined(_DEBUG) or defined(_RELEASE_DEV)
	EXPECT_EQ(output, "Warning: In BranchFlag instance with name \'test\'\n >>>Flag with \'Arg_String\' argument type may cause ambiguous parsing when used in non-delimited position parsable sequences. It is recommended to use the \'Arg_DelString\' type instead\n");
#elif defined(_RELEASE)
	EXPECT_EQ(output, "");
#endif

	auto& _optionalFlags = branch.OptionalFlags();
	auto& _requiredFlags = branch.RequiredFlags();
	auto& _posParsableFlags = branch.PosParsableFlags();

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

	EXPECT_TRUE(_posParsableFlags.size() == 1);
	EXPECT_EQ(_posParsableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
}

#pragma warning(push)
#pragma warning(disable:26800)
TEST(BranchFlagMoveConstructor, ValidateMove)
{
	Flag posParsable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_DelString() };
	posParsable.SetFlagIsPosParsable(true);

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	BranchFlag flagOne("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParsable));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "");

	BranchFlag flagTwo(std::move(flagOne));

	auto& _optionalFlagsOne = flagOne.OptionalFlags();
	auto& _requiredFlagsOne = flagOne.RequiredFlags();
	auto& _posParsableFlagsOne = flagOne.PosParsableFlags();

	auto& _optionalFlagsTwo = flagTwo.OptionalFlags();
	auto& _requiredFlagsTwo = flagTwo.RequiredFlags();
	auto& _posParsableFlagsTwo = flagTwo.PosParsableFlags();

	EXPECT_TRUE(_optionalFlagsOne.size() == 0);
	EXPECT_TRUE(_requiredFlagsOne.size() == 0);
	EXPECT_TRUE(_posParsableFlagsOne.size() == 0);

	EXPECT_TRUE(_optionalFlagsTwo.size() == 6);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParsableFlagsTwo.size() == 1);
}
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:26800)
TEST(BranchFlagMoveAssignment, ValidateMove)
{
	Flag posParsable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_DelString() };
	posParsable.SetFlagIsPosParsable(true);

	BranchFlag flagOne("test", "Test branch",
		Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParsable));

	BranchFlag flagTwo = std::move(flagOne);

	auto& _optionalFlagsOne = flagOne.OptionalFlags();
	auto& _requiredFlagsOne = flagOne.RequiredFlags();
	auto& _posParsableFlagsOne = flagOne.PosParsableFlags();

	auto& _optionalFlagsTwo = flagTwo.OptionalFlags();
	auto& _requiredFlagsTwo = flagTwo.RequiredFlags();
	auto& _posParsableFlagsTwo = flagTwo.PosParsableFlags();

	EXPECT_TRUE(_optionalFlagsOne.size() == 0);
	EXPECT_TRUE(_requiredFlagsOne.size() == 0);
	EXPECT_TRUE(_posParsableFlagsOne.size() == 0);

	EXPECT_TRUE(_optionalFlagsTwo.size() == 6);
	EXPECT_TRUE(_requiredFlagsTwo.size() == 3);
	EXPECT_TRUE(_posParsableFlagsTwo.size() == 1);
}
#pragma warning(pop)

TEST(BranchFlagMethods, SetSubFlags)
{
	BranchFlag branch("test", "Test branch");

	Flag posParsable{ {"r", "recipient"}, "The user and hostname (user:host-ip/name) to connect too", Arg_DelString() };
	posParsable.SetFlagIsPosParsable(true);

	branch.SetSubFlags(Flag{ {"d", "debug", "debug-mode"}, "Set the operation to perform debug features", Arg_Bool() },
		Flag{ {"m", "msg", "message"}, "The message to associate with the commit", Arg_String(), true, true },
		std::move(posParsable));

	auto& _optionalFlags = branch.OptionalFlags();
	auto& _requiredFlags = branch.RequiredFlags();
	auto& _posParsableFlags = branch.PosParsableFlags();

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

	EXPECT_TRUE(_posParsableFlags.size() == 1);
	EXPECT_EQ(_posParsableFlags[0]->FlagDescription(), "The user and hostname (user:host-ip/name) to connect too");
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

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyWarningOnInvalid_OptionalFlag)
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

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name \'test\'\n >>>Ignoring unknown token \'--object\' provided\n");
}

TEST(StrictStrictVerbose_BranchFlag_Raise, VerifyThrowOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(StrictStrictSilent_BranchFlag_Raise, VerifyThrowOnInvalid_OptionalFlag)
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

TEST(LenientStrictSilent_BranchFlag_Raise, VerifySuccessOnInvalid_OptionalFlag)
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

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyThrowOnInvalidFlagArg_OptionalFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(LenientLenientVerbose_BranchFlag_Raise, VerifyWarningOnInvalidFlagArg_OptionalFlag)
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
#ifdef _LINUX_TARGET
	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub-option. Thrown from option with token '-w'.\nError: stof\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
#elif defined(_WIN_TARGET)
	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub-option. Thrown from option with token '-w'.\nError: invalid stof argument\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
#endif
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyThrowOnInvalidFlagArg_RequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(LenientLenientVerbose_BranchFlag_Raise, VerifyThrowOnInvalidFlagArg_RequiredFlag)
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
#ifdef _LINUX_TARGET
	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub - option.Thrown from option with token '-w'.\nError: stof\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
#elif defined(_WIN_TARGET)
	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub - option.Thrown from option with token '-w'.\nError: invalid stof argument\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
#endif
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyValidPosParsable)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "}"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
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

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
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

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyWarningPosParsable_ExcessArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring excess arguments in the position parsable sequence\n");
}

TEST(StrictStrictVerbose_BranchFlag_Raise, VerifyThrowPosParsable_ExcessArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));


	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyThrowPosParsable_MissingRightDelimiter)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::runtime_error);
}

TEST(LenientStrictVerbose_BranchFlag_Raise, VerifyThrowOnMissingRequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "--strength", ".56", "{", "55.6", "234.78", "}"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_THROW(branch.Raise(itr, args.end()), std::invalid_argument);
}

TEST(BranchFlag_Raise, VerifyParseNonDelimitedPosParse_OnlyPosParseStream)
{
	const char* cmdArgs[] = { "terrainGen.exe", "22.5", "54.2", "12.345", ".56", "12" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 22.5f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 54.2f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 12.345f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 12);
}

TEST(BranchFlag_Raise, VerifyParseNonDelimitedPosParse_PartialPosParseStream)
{
	const char* cmdArgs[] = { "terrainGen.exe", "22.5", "54.2", "12.345", ".56" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, false, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 22.5f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 54.2f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 12.345f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 0);
}

TEST(BranchFlag_Raise, VerifyParseNonDelimitedPosParse_MixedArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "22.5", "54.2", "}","--depth", "12.345", "30", "55", "25", ".56", "-i", "12"};
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, false, true));

	ASSERT_NO_THROW(branch.Raise(itr, args.end()));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 30.0f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 55.0f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 25.0f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 12);
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifySuccessOnValid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "-f", "/.ssh/named_key" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	EXPECT_EQ(itr, args.end());
	EXPECT_EQ(branch.OptionalFlags().at("-f")->FlagArgument().as<std::string>(), "/.ssh/named_key");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyWarningOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name \'test\'\n >>>Ignoring unknown token \'--object\' provided\n");
}

TEST(StrictStrictVerbose_BranchFlag_TryRaise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with name \'test\'\n >>>Unknown token \'--object\' provided\n");
}

TEST(StrictStrictSilent_BranchFlag_TryRaise, VerifyFailOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict, ExceptionMode::Strict, VerbositySetting::Silent> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with name \'test\'\n >>>Unknown token \'--object\' provided\n");
}

TEST(LenientStrictSilent_BranchFlag_TryRaise, VerifySuccessOnInvalid_OptionalFlag)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "--object", "/.ssh/obj_file.obj" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Silent> branch("test", "test branch", Flag({ "f", "out-file" }, "Specify where to make the key files, and what name to give the key", Arg_String(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyFailOnInvalidFlagArg_OptionalFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true));

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));
}

TEST(LenientLenientVerbose_BranchFlag_TryRaise, VerifyFailOnInvalidFlagArg_OptionalFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub-option. Thrown from option with token '-w'\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyFailOnInvalidFlagArg_RequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true));

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));
}

TEST(LenientLenientVerbose_BranchFlag_TryRaise, VerifyFailOnInvalidFlagArg_RequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "-w", "Fifty Five Point Zero" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Lenient, VerbositySetting::Verbose> branch("test", "test branch", Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name 'test'\n >>>Ignoring parsing error from options branch sub-option. Thrown from option with token '-w'\nWarning: In BranchFlag instance with name 'test'\n >>>Ignoring unknown token 'Fifty Five Point Zero' provided\n");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyValidPosParsable)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyValidPosParsable_LimitedSet_SpecificsPrepend)
{
	const char* cmdArgs[] = { "terrainGen.exe", "--strength", ".56", "-i", "2", "{", "55.6", "234.78", "43.0", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyValidPosParsable_LimitedSet_SpecificsApend)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", "}", "--strength", ".56", "-i", "2" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyWarningPosParsable_ExcessArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In BranchFlag instance with name \'test\'\n >>>Ignoring excess arguments in the position parsable sequence\n");
}

TEST(StrictStrictVerbose_BranchFlag_TryRaise, VerifyFailPosParsable_ExcessArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Strict, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with name \'test\'\n >>>Excess arguments will not be ignored. Options branch has 5 position parsable flags configured. 6 arguments were provided in the position parsable sequence\n");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyFailPosParsable_MissingRightDelimiter)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "55.6", "234.78", "43.0", ".56", "2", "Sven" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In BranchFlag instance with name \'test\'\n >>>Expected matching right delimiter \'}\' to the left delimiter \'{\' that denote the boundaries of the position parsable flags arguments\n");
}

TEST(LenientStrictVerbose_BranchFlag_TryRaise, VerifyFailOnMissingRequiredFlag)
{
	const char* cmdArgs[] = { "terrainGen.exe", "--strength", ".56", "{", "55.6", "234.78", "}" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_NO_THROW(ASSERT_FALSE(branch.TryRaise(itr, args.end())));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();
	std::string truncOutput = output.substr(0, 92);

	EXPECT_EQ(output, "Error: In BranchFlag instance with name \'test\'\n >>>Options branch requires that the, (-w, --width), (-h, --height), (-d, --depth), (-s, --strength), (-i, --iterations), flag tokens all be set with valid arguments\n");
}

TEST(BranchFlag_TryRaise, VerifyParseNonDelimitedPosParse_OnlyPosParseStream)
{
	const char* cmdArgs[] = { "terrainGen.exe", "22.5", "54.2", "12.345", ".56", "12" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, true, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 22.5f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 54.2f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 12.345f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 12);
}

TEST(BranchFlag_TryRaise, VerifyParseNonDelimitedPosParse_PartialPosParseStream)
{
	const char* cmdArgs[] = { "terrainGen.exe", "22.5", "54.2", "12.345", ".56" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, false, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 22.5f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 54.2f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 12.345f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 0);
}

TEST(BranchFlag_TryRaise, VerifyParseNonDelimitedPosParse_MixedArgs)
{
	const char* cmdArgs[] = { "terrainGen.exe", "{", "22.5", "54.2", "}","--depth", "12.345", "30", "55", "25", ".56", "-i", "12" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr++;

	BranchFlag<ParsingMode::Lenient, ExceptionMode::Strict, VerbositySetting::Verbose> branch("test", "test branch",
		Flag({ "w", "width" }, "Specify the width of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "h", "height" }, "Specify the height of the terrain generation field", Arg_Float(), true, true, true),
		Flag({ "d", "depth" }, "Specify the depth of the terrain generation field", Arg_Float(), true, true).SetFlagIsPosParsable(true),
		Flag({ "s", "strength", "noise-strength" }, "Specify the strength value for the noise function, 0-1", Arg_Float()).SetFlagArgRequired(true).SetFlagRequired(true).SetFlagIsPosParsable(true),
		Flag({ "i", "iterations" }, "Set the number of iterations/passes for the generation", Arg_UInt32(), true, false, true));

	ASSERT_NO_THROW(ASSERT_TRUE(branch.TryRaise(itr, args.end())));

	auto& flags = branch.PosParsableFlags();

	EXPECT_EQ(flags[0]->FlagArgument().as<float>(), 30.0f);
	EXPECT_EQ(flags[1]->FlagArgument().as<float>(), 55.0f);
	EXPECT_EQ(flags[2]->FlagArgument().as<float>(), 25.0f);
	EXPECT_EQ(flags[3]->FlagArgument().as<float>(), .56f);
	EXPECT_EQ(flags[4]->FlagArgument().as<uint32_t>(), 12);
}
#include <gtest/gtest.h>
#include <Flag.h>

#include <flag_argument_specializations.h>

using namespace GenTools::GenParse;

TEST(TokensConstructor, VerifyShortToken)
{
	Tokens tokens{ "d" };
	EXPECT_EQ(tokens._shortToken, "d");
}

TEST(TokensConstructor, VerifyLongToken)
{
	std::string testInput("debug");
	Tokens tokens{ std::move(testInput) };
	EXPECT_EQ(tokens._longTokens[0], "debug");
	EXPECT_EQ(testInput, "");
}

TEST(TokensConstructor, VerifyMultiToken)
{
	Tokens tokens{ "d", "debug", "debug-mode" };
	EXPECT_EQ(tokens._shortToken, "d");
	EXPECT_EQ(tokens._longTokens[0], "debug");
	EXPECT_EQ(tokens._longTokens[1], "debug-mode");
}

TEST(TokensConstructor, MultiShortToken)
{
#if defined(_DEBUG) or _RELEASE_DEV
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	EXPECT_NO_THROW(Tokens( "d", "c" ));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Warning: In Flag with short token 'd'\n >>>Short token has already been set. Another token, 'c', is overriding previous short token\n");

#else
	EXPECT_NO_THROW(Tokens("d", "c"));
#endif // _DEBUG
}

TEST(TokensConstructor, ThrowOnEmptyToken)
{
#ifdef _DEBUG
	EXPECT_THROW(Tokens{ "" }, std::logic_error);
#endif // _DEBUG
#if _RELEASE_DEV
	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	EXPECT_NO_THROW(Tokens(""));

	std::cerr.rdbuf(oldCerr);

	std::string output = buffer.str();

	EXPECT_EQ(output, "Error: In Flag instantiation\n >>>Empty token provided! Run in debug mode for more details on this error\n");
#elif _RELEASE
	EXPECT_NO_THROW(Tokens(""));
#endif // _RELEASE_DEV
}

TEST(FlagConstructor1, VerifyFlagDescription)
{
	Flag<Arg_Bool> flag({ "test" }, "This flag is for testing");
	EXPECT_EQ(flag.FlagDescription(), "This flag is for testing");
}

TEST(FlagConstructor1, VerifyDefaultFlagRequired)
{
	Flag<Arg_Bool> flag({ "test" }, "This flag is for testing");
	EXPECT_FALSE(flag.FlagRequired);
}

TEST(FlagConstructor1, VerifySetFlagRequired)
{
	Flag<Arg_Bool> flag({ "test" }, "This flag is for testing", true);
	EXPECT_TRUE(flag.FlagRequired);
}

TEST(FlagConstructor2, VerifyFlagDescription)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default") };
	EXPECT_EQ(flag.FlagDescription(), "This flag is for testing");
}

TEST(FlagConstructor2, VerifyDefaultFlagRequired)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default") };
	EXPECT_FALSE(flag.FlagRequired);
}

TEST(FlagConstructor2, VerifyDefaultArgRequired)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default") };
	EXPECT_TRUE(flag.ArgRequired);
}

TEST(FlagConstructor2, VerifySetFlagRequired)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default"), false, true };
	EXPECT_TRUE(flag.FlagRequired);
}

TEST(FlagConstructor2, VerifySetArgRequired)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default"), true };
	EXPECT_TRUE(flag.ArgRequired);
}

TEST(FlagConstructor2, VerifySetFlagArg)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_String("default") };
	const flag_argument& arg = flag.FlagArgument();
	EXPECT_EQ(arg.as<std::string>(), "default");
}

TEST(FlagMoveConstructor, ValidateMove)
{
	Flag flagOne({ "d", "debug", "debug-mode" }, "Set the system to run in debug mode", Arg_Bool(true));

	EXPECT_TRUE(flagOne.FlagArgument().as<bool>());

	Flag flagTwo(std::move(flagOne));

	EXPECT_EQ(flagOne.ShortToken(), "");
	EXPECT_EQ(flagOne.LongTokens().size(), 0);
	EXPECT_EQ(flagOne.FlagDescription(), "");
	EXPECT_FALSE(flagOne.ArgumentSet());

	EXPECT_EQ(flagTwo.ShortToken(), "d");
	EXPECT_EQ(flagTwo.LongTokens().size(), 2);
	EXPECT_EQ(flagTwo.LongTokens()[0], "debug");
	EXPECT_EQ(flagTwo.LongTokens()[1], "debug-mode");
	EXPECT_EQ(flagTwo.FlagDescription(), "Set the system to run in debug mode");
	ASSERT_TRUE(flagTwo.ArgumentSet());
	EXPECT_TRUE(flagTwo.FlagArgument().as<bool>());
}

TEST(FlagMoveAssignment, ValidateMove)
{
	Flag flagOne({ "d", "debug", "debug-mode" }, "Set the system to run in debug mode", Arg_Bool(true));
	Flag<Arg_Bool> flagTwo{ {"temp"}, "" };
	flagTwo = std::move(flagOne);

	EXPECT_EQ(flagOne.ShortToken(), "");
	EXPECT_EQ(flagOne.LongTokens().size(), 0);
	EXPECT_EQ(flagOne.FlagDescription(), "");
	EXPECT_FALSE(flagOne.ArgumentSet());

	EXPECT_EQ(flagTwo.ShortToken(), "d");
	EXPECT_EQ(flagTwo.LongTokens().size(), 2);
	EXPECT_EQ(flagTwo.LongTokens()[0], "debug");
	EXPECT_EQ(flagTwo.LongTokens()[1], "debug-mode");
	EXPECT_EQ(flagTwo.FlagDescription(), "Set the system to run in debug mode");
	ASSERT_TRUE(flagTwo.ArgumentSet());
	EXPECT_TRUE(flagTwo.FlagArgument().as<bool>());
}

TEST(FlagMethods, SetFlagRequired)
{
	Flag<Arg_Bool> flag({ "test" }, "This flag is for testing", false);
	flag.SetFlagRequired(true);
	EXPECT_TRUE(flag.FlagRequired);
}

TEST(FlagMethods, SetFlagArgRequired)
{
	Flag flag{ {"test"}, "This flag is for testing", Arg_Bool(), false };
	flag.SetFlagArgRequired(true);
	EXPECT_TRUE(flag.ArgRequired);
}

TEST(FlagMethods, SetFlagIsPosParsable)
{
	Flag<Arg_Bool> flag({ "test" }, "This flag is for testing");
	flag.SetFlagIsPosParsable(true);
	EXPECT_TRUE(flag.PosParsable);
}

TEST(FlagMethods, SetFlagArgument)
{
	Flag<Arg_String> flag({ "test" }, "This flag is for testing");
	flag.SetFlagArgument(Arg_String("default"));
	const flag_argument& arg = flag.FlagArgument();
	EXPECT_EQ(arg.as<std::string>(), "default");
}

TEST(FlagRaise, VerifySuccessOnValid)
{
	const char* cmdArgs[] = { "ssh-keygen.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0])};
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), true);

	ASSERT_NO_THROW(flag.Raise(itr, args.end()));

	EXPECT_EQ(itr, args.end());
	EXPECT_EQ(flag.FlagArgument().as<std::string>(), "/.ssh/named_key.pub");
}

TEST(FlagRaise, OnNoArg)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag<Arg_String> flag({ "f", "out-file" }, "Specify what file to save to");

#if defined(_DEBUG) or _RELEASE_DEV
	EXPECT_THROW(flag.Raise(itr, args.end()), std::logic_error);
#else
	EXPECT_NO_THROW(flag.Raise(itr, args.end()));
#endif
}

TEST(FlagRaise, ThrowsOnEnd_ArgRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 3;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), true);
	
	ASSERT_TRUE(flag.ArgRequired);
	EXPECT_THROW(flag.Raise(itr, args.end()), std::logic_error);
}

TEST(FlagRaise, NoThrowOnEnd_ArgNOTRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 3;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), false);

	ASSERT_FALSE(flag.ArgRequired);
	EXPECT_NO_THROW(flag.Raise(itr, args.end()));
}

TEST(FlagRaise, ThrowsOnBadParse_ArgRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag flag({ "i", "intensity" }, "Specify the illumination intensity value", Arg_Double(), true);

	ASSERT_TRUE(flag.ArgRequired);
	EXPECT_THROW(flag.Raise(itr, args.end()), std::invalid_argument);
}

TEST(FlagRaise, NoThrowOnBadParse_ArgNOTRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;
	auto preRaiseItr = itr;

	Flag flag({ "i", "intensity" }, "Specify the illumination intensity value", Arg_Double(), false);

	ASSERT_FALSE(flag.ArgRequired);
	EXPECT_NO_THROW(flag.Raise(itr, args.end()));

	EXPECT_EQ(itr, preRaiseItr);
}

TEST(FlagTryRaise, VerifySuccessOnValid)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), true);

	ASSERT_TRUE(flag.TryRaise(itr, args.end()));

	EXPECT_EQ(itr, args.end());
	EXPECT_EQ(flag.FlagArgument().as<std::string>(), "/.ssh/named_key.pub");
}

TEST(FlagTryRaise, OnNoArg)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag<Arg_String> flag({ "f", "out-file" }, "Specify what file to save to");

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

#if defined(_DEBUG) or _RELEASE_DEV
	EXPECT_FALSE(flag.TryRaise(itr, args.end()));
#else
	EXPECT_TRUE(flag.TryRaise(itr, args.end()));
#endif

	std::cerr.rdbuf(oldCerr);
}

TEST(FlagTryRaise, ThrowsOnEnd_ArgRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 3;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), true);

	std::stringstream buffer;
	std::streambuf* oldCerr = std::cerr.rdbuf(buffer.rdbuf());

	ASSERT_TRUE(flag.ArgRequired);
	EXPECT_FALSE(flag.TryRaise(itr, args.end()));

	std::cerr.rdbuf(oldCerr);
}

TEST(FlagTryRaise, NoThrowOnEnd_ArgNOTRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 3;

	Flag flag({ "f", "out-file" }, "Specify what file to save to", Arg_String(), false);

	ASSERT_FALSE(flag.ArgRequired);
	EXPECT_TRUE(flag.TryRaise(itr, args.end()));
}

TEST(FlagTryRaise, ThrowsOnBadParse_ArgRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;

	Flag flag({ "i", "intensity" }, "Specify the illumination intensity value", Arg_Double(), true);

	ASSERT_TRUE(flag.ArgRequired);
	EXPECT_FALSE(flag.TryRaise(itr, args.end()));
}

TEST(FlagTryRaise, NoThrowOnBadParse_ArgNOTRequired)
{
	const char* cmdArgs[] = { "log.exe", "-f", "/.ssh/named_key.pub" };
	std::vector<std::string_view> args{ cmdArgs, cmdArgs + sizeof(cmdArgs) / sizeof(cmdArgs[0]) };
	std::vector<std::string_view>::const_iterator itr = args.begin();
	itr += 2;
	auto preRaiseItr = itr;

	Flag flag({ "i", "intensity" }, "Specify the illumination intensity value", Arg_Double(), false);

	ASSERT_FALSE(flag.ArgRequired);
	EXPECT_TRUE(flag.TryRaise(itr, args.end()));

	EXPECT_EQ(itr, preRaiseItr);
}

TEST(ExtensionMethods, Validate_MakeFlagsPositionParsable)
{
	Flag flagOne{ {"test", "t"}, "Test flag one", Arg_Int32() };
	Flag flagTwo{ {"d"}, "Test flag two", Arg_Double() };
	Flag flagThree{ {"f", "file"}, "Test flag three", Arg_String() };

	ASSERT_NO_THROW(MakeFlagsPositionParsable(flagOne, flagTwo, flagThree));

	EXPECT_TRUE(flagOne.PosParsable);
	EXPECT_TRUE(flagTwo.PosParsable);
	EXPECT_TRUE(flagThree.PosParsable);
}
#include <gtest/gtest.h>
#include <flag_argument_specializations.h>

using namespace GenTools::GenParse;

#define CREATE_SPEC_TESTS(SpecTypeName, ArgSetValue, UnderlyingType) \
	TEST(SpecTypeName, VerifyAsFunctionality) \
	{ \
		const flag_argument* argPtr = nullptr; \
		SpecTypeName arg(ArgSetValue); \
		argPtr = &arg; \
		EXPECT_EQ(argPtr->as<UnderlyingType>(), ArgSetValue); \
	} \
    \
	TEST(SpecTypeName, VerifyGetArg) \
	{ \
		SpecTypeName arg(ArgSetValue); \
		EXPECT_EQ(arg.GetArg(), ArgSetValue); \
	} \
	\
	TEST(SpecTypeName, VerifySetDefaultValue) \
	{ \
		SpecTypeName arg; \
		UnderlyingType before = arg.GetArg(); \
		arg.SetDefaultValue(ArgSetValue); \
		UnderlyingType after = arg.GetArg(); \
		EXPECT_NE(before, after); \
		EXPECT_EQ(after, ArgSetValue); \
	}

CREATE_SPEC_TESTS(Arg_Bool, true, bool);

CREATE_SPEC_TESTS(Arg_Int32, -5, int32_t);

CREATE_SPEC_TESTS(Arg_UInt32, 5, uint32_t);

CREATE_SPEC_TESTS(Arg_Int64, -10, int64_t);

CREATE_SPEC_TESTS(Arg_UInt64, 10, uint64_t);

CREATE_SPEC_TESTS(Arg_Float, 3.14159f, float);

CREATE_SPEC_TESTS(Arg_Double, 3.14159, double);

CREATE_SPEC_TESTS(Arg_LongDouble, 3.14159L, long double);

CREATE_SPEC_TESTS(Arg_String, "Test string", std::string);

CREATE_SPEC_TESTS(Arg_DelString, "Test string", std::string);

TEST(Arg_List_Arg_String, VerifyAsFunctionality)
{
	const flag_argument* argPtr = nullptr;
	Arg_List<Arg_String> arg(std::vector<std::string>{ "FirstString", "SecondString" });
	argPtr = &arg;
	const std::vector<std::string>& argument = argPtr->as<std::vector<std::string>>();
	std::vector<std::string> value{ "FirstString", "SecondString" };
	EXPECT_EQ(argument, value);
}

TEST(Arg_List_Arg_String, VerifyGetArg)
{
	Arg_List<Arg_String> arg(std::vector<std::string>{ "FirstString", "SecondString" });
	std::vector<std::string> value{ "FirstString", "SecondString" };
	EXPECT_EQ(arg.GetArg(), value);
}

TEST(Arg_List_Arg_String, VerifySetDefaultValue)
{
	Arg_List<Arg_String> arg;
	std::vector<std::string> before = arg.GetArg();
	arg.SetDefaultValue(std::vector<std::string>{ "FirstString", "SecondString" });
	std::vector<std::string> after = arg.GetArg();
	EXPECT_NE(before, after);
	std::vector<std::string> value{ "FirstString", "SecondString" };
	EXPECT_EQ(after, value);
}
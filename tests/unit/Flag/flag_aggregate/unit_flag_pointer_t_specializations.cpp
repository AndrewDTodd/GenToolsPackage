#include <gtest/gtest.h>
#include <flag_argument_specializations.h>

using namespace CmdLineParser;

#define CREATE_SPEC_TESTS(SpecTypeName, ArgSetValue, UnderlyingType) \
	TEST(SpecTypeName, VerifyAsFunctionality) \
	{ \
		UnderlyingType target; \
		target = ArgSetValue; \
		const flag_argument* argPtr = nullptr; \
		SpecTypeName arg(&target); \
		argPtr = &arg; \
		EXPECT_EQ(argPtr->as<UnderlyingType*>(), &target); \
		EXPECT_EQ(*(argPtr->as<UnderlyingType*>()), ArgSetValue); \
	} \
    \
	TEST(SpecTypeName, VerifyGetArg) \
	{ \
		UnderlyingType target; \
		target = ArgSetValue; \
		SpecTypeName arg(&target); \
		EXPECT_EQ(arg.GetArg(), &target); \
		EXPECT_EQ(*(arg.GetArg()), ArgSetValue); \
	} \
	\
	TEST(SpecTypeName, VerifySetLinkedValue) \
	{ \
		UnderlyingType target; \
		target = ArgSetValue; \
		SpecTypeName arg; \
		const UnderlyingType* before = arg.GetArg(); \
		arg.SetLinkedValue(&target); \
		const UnderlyingType* after = arg.GetArg(); \
		EXPECT_NE(before, after); \
		EXPECT_EQ(after, &target); \
		EXPECT_EQ(*after, ArgSetValue); \
	}

CREATE_SPEC_TESTS(Ptr_Bool, true, bool);

CREATE_SPEC_TESTS(Ptr_Int32, -5, int32_t);

CREATE_SPEC_TESTS(Ptr_UInt32, 5, uint32_t);

CREATE_SPEC_TESTS(Ptr_Int64, -10, int64_t);

CREATE_SPEC_TESTS(Ptr_UInt64, 10, uint64_t);

CREATE_SPEC_TESTS(Ptr_Float, 3.14159f, float);

CREATE_SPEC_TESTS(Ptr_Double, 3.14159, double);

CREATE_SPEC_TESTS(Ptr_LongDouble, 3.14159L, long double);

CREATE_SPEC_TESTS(Ptr_String, "Test string", std::string);

CREATE_SPEC_TESTS(Ptr_DilString, "Test string", std::string);
#include <gtest/gtest.h>
#include <flag_event.h>

using namespace GenTools::GenParse;

bool triggered = false;

int TestFunctOne()
{
	return 1;
}

int TestFuncTwo(int* num)
{
	*num = 3;

	return 0;
}

void TestFuncThree(int num, float* fltNum)
{
	*fltNum = num;
	triggered = true;
}

TEST(flag_event_t, Constructor1)
{
	int number;
	ASSERT_NO_THROW(flag_event_t(std::function(TestFuncTwo), &number));
}

TEST(flag_event_t, ValidateGoodRun)
{
	float number;

	flag_event_t<void, int, float*> flgEvnt;

	ASSERT_NO_THROW(flgEvnt.SetTriggeredFunc(std::function(TestFuncThree)).SetFuncArguments(3, &number));

	ASSERT_NO_THROW(flgEvnt.Run());

	EXPECT_EQ(3.0f, number);

	EXPECT_TRUE(triggered);

	triggered = false;
}

TEST(flag_event_t, ValidateRunThrowOnNullFUNC)
{
	flag_event_t<int> flgEvnt;

#if defined(_DEBUG) or defined(_RELEASE_DEV)
	ASSERT_THROW(flgEvnt.Run(), std::logic_error);
#elif defined(_RELEASE)
	//ASSERT_NO_THROW(flgEvnt.Run()); 
	// Not running in Release mode as it causes a Segmentation Fault, not exception. 
	// This is at least on GCC compiler, unsure of behavior otherwise. Haven't decided what Release behavior should be
#endif
}

TEST(flag_event_t, ValidateGoodTryRun)
{
	float number;

	flag_event_t flgEvnt(std::function<void(int, float*)>(TestFuncThree), 3, &number);

	ASSERT_NO_THROW(EXPECT_TRUE(flgEvnt.TryRun()));

	EXPECT_EQ(3.0f, number);

	EXPECT_TRUE(triggered);

	triggered = false;
}

TEST(flag_event_t, ValidateTryRunThrowOnNullFUNC)
{
	std::string errorMsg;

	flag_event_t<int> flgEvnt;

#if defined(_DEBUG) or defined(_RELEASE_DEV)
	ASSERT_NO_THROW(EXPECT_FALSE(flgEvnt.TryRun(&errorMsg)));

	EXPECT_EQ(errorMsg, "Error: Target function not set for flag_event_t instance. Can't trigger the event");
#elif defined(_RELEASE)

#endif

}

TEST(flag_event_t, VerifyTryRunErrorMsgOnException)
{
	auto testFunc = [](int x, int y) -> int {
		if (x == 0) throw std::runtime_error("Invalid input");
		return x + y;
	};

	flag_event_t<int, int, int> flgEvnt(testFunc, 0, 5);

	std::string errorMsg;
	ASSERT_FALSE(flgEvnt.TryRun(&errorMsg));
	EXPECT_EQ(errorMsg, "Invalid input");
}
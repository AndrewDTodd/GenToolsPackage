#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace CmdLineParser;

TEST(ParseString, Parse_valid)
{
	std::string str;
	EXPECT_NO_THROW({ str = ParseString("\"Test String\""); });
	EXPECT_EQ(str, "Test String");

	EXPECT_NO_THROW({ str = ParseString("\"The \"world\" is your oyster\""); });
	EXPECT_EQ(str, "The \"world\" is your oyster");
}

TEST(ParseString, Parse_invalid)
{
	EXPECT_THROW(ParseString("Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseString("\"Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseString("Test invalid\""), std::invalid_argument);
	EXPECT_THROW(ParseString("Test \"this\" invalid"), std::invalid_argument);
}
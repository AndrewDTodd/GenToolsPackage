#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace TokenValueParser;

TEST(ParseStringDelimited, Parse_valid)
{
	std::string str;
	EXPECT_NO_THROW({ str = ParseStringDelimited("\"Test String\""); });
	EXPECT_EQ(str, "Test String");

	EXPECT_NO_THROW({ str = ParseStringDelimited("\"The \"world\" is your oyster\""); });
	EXPECT_EQ(str, "The \"world\" is your oyster");
}

TEST(ParseStringDelimited, Parse_invalid)
{
	EXPECT_THROW(ParseStringDelimited("Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseStringDelimited("\"Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseStringDelimited("Test invalid\""), std::invalid_argument);
	EXPECT_THROW(ParseStringDelimited("Test \"this\" invalid"), std::invalid_argument);
}
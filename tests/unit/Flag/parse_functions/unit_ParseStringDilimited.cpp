#include <gtest/gtest.h>
#include <parse_functions.h>

using namespace TokenValueParser;

TEST(ParseStringDilimited, Parse_valid)
{
	std::string str;
	EXPECT_NO_THROW({ str = ParseStringDilimited("\"Test String\""); });
	EXPECT_EQ(str, "Test String");

	EXPECT_NO_THROW({ str = ParseStringDilimited("\"The \"world\" is your oyster\""); });
	EXPECT_EQ(str, "The \"world\" is your oyster");
}

TEST(ParseStringDilimited, Parse_invalid)
{
	EXPECT_THROW(ParseStringDilimited("Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseStringDilimited("\"Test invalid"), std::invalid_argument);
	EXPECT_THROW(ParseStringDilimited("Test invalid\""), std::invalid_argument);
	EXPECT_THROW(ParseStringDilimited("Test \"this\" invalid"), std::invalid_argument);
}
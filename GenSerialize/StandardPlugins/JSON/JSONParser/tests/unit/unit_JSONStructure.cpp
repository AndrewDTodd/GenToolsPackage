#include <gtest/gtest.h>
#include <JSONStructure.h>

using namespace GenTools::GenSerialize;

TEST(JSONStructureTests, ParseSimpleObject)
{
	std::stringstream jsonStream(R"({"name": "Test", "age": 30})");
	JSONStructure json = JSONStructure::Parse(jsonStream);

	ASSERT_EQ(json["name"]->as<JSONString>()->value, "Test");
	ASSERT_EQ(json["age"]->as<JSONNumber>()->value, 30);
}

TEST(JSONStructureTests, ParseNestedObject)
{
	std::stringstream jsonStream(R"({"person": {"name": "Alice", "age": 25}})");
	JSONStructure json = JSONStructure::Parse(jsonStream);

	auto& person = *json["person"]->as<JSONObject>();
	ASSERT_EQ(person["name"]->as<JSONString>()->value, "Alice");
	ASSERT_EQ(person["age"]->as<JSONNumber>()->value, 25.0);
}

TEST(JSONStructureTests, ParseArray)
{
	std::stringstream jsonStream(R"({"numbers": [1, 2, 3, 4]})");
	JSONStructure json = JSONStructure::Parse(jsonStream);

	auto& numbers = json["numbers"]->as<JSONArray>()->GetItems();
	std::vector<double> expected = { 1, 2, 3, 4 };
	for (size_t i = 0; i < expected.size(); i++)
	{
		EXPECT_EQ(numbers[i]->as<JSONNumber>()->value, expected[i]);
	}
}

TEST(JSONStructureTests, ParseInvalidJSON)
{
	std::stringstream jsonStream(R"({"name": "Test", "age": )"); // Invalid JSON

	ASSERT_THROW(JSONStructure::Parse(jsonStream), std::invalid_argument);
}

TEST(JSONStructureTests, StringifyTest)
{
	std::stringstream jsonStream(R"({"name": "John", "age": 40})");
	JSONStructure json = JSONStructure::Parse(jsonStream);

	std::string jsonString = json.Stringify();
	ASSERT_EQ(jsonString, R"({
    "name": "John",
    "age": 40
})"); // Expected format with indentation
}

TEST(JSONStructureTests, WriteToFileTest)
{
	JSONStructure json;
	json["message"] = std::make_unique<JSONString>("Hello, World!");

	json.WriteToFile("test.json");

	std::ifstream file("test.json");
	ASSERT_TRUE(file.is_open());

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	ASSERT_EQ(buffer.str(), json.Stringify());
}
#include <JSONObject.h>

#include <JSONStructure.h>
#include <JSONArray.h>

namespace GenTools::GenSerialize
{
	void JSONObject::Parse(std::string_view& jsonView)
	{
		auto itr = jsonView.begin();
		auto end = jsonView.end();

		// Validate opening brace
		SkipWhitespace(jsonView, itr);
		if (itr == end || *itr != '{')
			throw std::invalid_argument("Invalid JSON: Expected '{' at beginning");
		++itr;

		// Clear any previous members
		m_members.clear();

		while (true)
		{
			SkipWhitespace(jsonView, itr);
			if (itr == end)
				throw std::invalid_argument("Invalid JSON: Unexpected end in object");
			if (*itr == '}')
			{
				++itr; // consume '}'
				break;
			}

			// Key must be a string
			if (*itr != '"')
				throw std::invalid_argument("Invalid JSON: Expected '\"' at key start");
			++itr;
			auto keyStart = itr;
			while (itr != end && *itr != '"')
				++itr;
			if (itr == end)
				throw std::invalid_argument("Invalid JSON: Unterminated key string");
			std::string key(keyStart, itr);
			++itr; // consume closing '"'

			SkipWhitespace(jsonView, itr);
			if (itr == end || *itr != ':')
				throw std::invalid_argument("Invalid JSON: Expected ':' after key");
			++itr; // skip colon

			SkipWhitespace(jsonView, itr);
			if (itr == end)
				throw std::invalid_argument("Invalid JSON: Missing value after ':'");

			// Determine the value type
			std::unique_ptr<JSONValue> arg;
			if (*itr == '"')
			{
				// string value
				++itr;
				auto valStart = itr;
				while (itr != end && *itr != '"')
					++itr;
				if (itr == end)
					throw std::invalid_argument("Invalid JSON: Unterminated string value");
				std::string value(valStart, itr);
				++itr; // consume closing '"'
				arg = std::make_unique<JSONString>(std::move(value));
			}
			else if (std::isdigit(*itr) || *itr == '-' || *itr == '.')
			{
				// Numeric value
				auto numStart = itr;
				while (itr != end && (std::isdigit(*itr) || *itr == '.' || *itr == '-'))
					++itr;
				std::string numStr(numStart, itr);
				arg = std::make_unique<JSONNumber>(std::stod(numStr));
			}
			else if (*itr == 't' || *itr == 'f')
			{
				// Compute the offset and create a substring view
				size_t offset = std::distance(jsonView.begin(), itr);
				std::string_view boolCandidate = jsonView.substr(offset);

				if (boolCandidate.starts_with("true"))
				{
					arg = std::make_unique<JSONBool>(true);
					itr += 4; // Move past "true"
				}
				else if (boolCandidate.starts_with("false"))
				{
					arg = std::make_unique<JSONBool>(false);
					itr += 5; // Move past "false"
				}
				else
				{
					throw std::runtime_error("Invalid boolean literal encountered");
				}
			}
			else if (*itr == '[')
			{
				// array value: extract the entire array string
				std::string_view arrayStrview = ExtractBalanced(jsonView, itr, '[', ']');
				auto argArr = std::make_unique<JSONArray>();
				argArr->Parse(arrayStrview);
				arg = std::move(argArr);
			}
			else if (*itr == '{')
			{
				// nested object
				std::string_view objStrview = ExtractBalanced(jsonView, itr, '{', '}');
				auto argObj = std::make_unique<JSONObject>();
				argObj->Parse(objStrview);
				arg = std::move(argObj);
			}
			else
				throw std::invalid_argument("Invalid JSON: Unexpected token when parsing value");

			m_members.emplace(std::move(key), std::move(arg));

			// Skip whitespace and look for a comma or the end-of-object
			SkipWhitespace(jsonView, itr);
			if (itr != end && *itr == ',')
				++itr;
			else if (itr != end && *itr == '}')
			{
				++itr; // consume '}'
				break;
			}
			else if (itr == end)
				throw std::invalid_argument("Invalid JSON: Unexpected and of object");
			else
				throw std::invalid_argument("Invalid JSON: Expected ',' or '}' after value");
		}
	}
}
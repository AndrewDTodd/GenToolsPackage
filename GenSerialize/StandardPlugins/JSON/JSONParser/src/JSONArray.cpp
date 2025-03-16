#include <JSONArray.h>

#include <JSONStructure.h>
#include <JSONObject.h>

namespace GenTools::GenSerialize
{
	void JSONArray::Parse(std::string_view& jsonView)
	{
		auto itr = jsonView.begin();
		auto end = jsonView.end();

		SkipWhitespace(jsonView, itr);
		if (itr == end || *itr != '[')
			throw std::invalid_argument("Invalid JSON: Expected '[' at beginning of array");
		++itr; // consume '['

		m_items.clear();

		while (true)
		{
			SkipWhitespace(jsonView, itr);
			if (itr == end)
				throw std::invalid_argument("Invalid JSON: Unexpected end in array");
			if (*itr == ']')
			{
				++itr; // consume ']'
				break;
			}

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

			m_items.push_back(std::move(arg));

			// Skip whitespace then check for comma or the end of array
			SkipWhitespace(jsonView, itr);
			if (itr != end && *itr == ',')
				++itr;
			else if (itr != end && *itr == ']')
			{
				++itr;
				break;
			}
		}
	}
}
#include <StringifyValue.h>

#include <string_view>
#include <sstream>
#include <iomanip>

#include <JSONValue.h>

#include <JSONObject.h>
#include <JSONArray.h>

namespace GenTools::GenSerialize::JSON
{
    std::string StringifyValue(const std::unique_ptr<JSONValue>& value, int level, int indent)
    {
        if (!value)
            return "null"; // Handle null values

        switch (value->Type())
        {
        case JSONType::String:
            return "\"" + value->as<JSONString>().value + "\"";

        case JSONType::Number: {
            double num = value->as<JSONNumber>().value;
            if (num == static_cast<int64_t>(num)) {
                return std::to_string(static_cast<int64_t>(num)); // Print as integer if whole
            }
            else {
                std::ostringstream oss;
                oss << std::setprecision(15) << num; // Avoid unnecessary trailing zeros
                return oss.str();
            }
        }

        case JSONType::Bool:
            return value->as<JSONBool>().value ? "true" : "false";

        case JSONType::Array:
            return value->as<JSONArray>().StringifyHelper(level, indent);

        case JSONType::Object:
            return value->as<JSONObject>().StringifyHelper(level, indent);

        case JSONType::Null:
            return "null";

        default:
            throw std::runtime_error("Unsupported JSON type in Stringify");
        }
    }
}
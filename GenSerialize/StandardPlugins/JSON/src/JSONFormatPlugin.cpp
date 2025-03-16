#include <JSONFormatPlugin.h>

#include <sstream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <memory>
#include <vector>

#include <JSONStructure.h>

namespace GenTools::GenSerialize
{
	DECLARE_FORMAT_PLUGIN(JSONFormatPlugin)

	std::string JSONFormatPlugin::GenerateArraySerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& elementName, size_t depth)
	{
		std::ostringstream oss;

		std::string newElementName = elementName + "_" + std::to_string(depth);

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << jsonObjName << ".AddMember(JSONNumber(static_cast<double>(" << elementName << "))); \n";
			break;

		case SASTType::Bool:
			oss << "\t" << jsonObjName << ".AddMember(JSONBool(" << elementName << "));\n";
			break;

		case SASTType::String:
			oss << "\t" << jsonObjName << ".AddMember(JSONString(" << elementName << "))\n";
			break;

		case SASTType::POD:
			oss << "\t\tJSONObject json_" << elementName << ";\n";
			for (auto& podField : field.objectNode->fields)
			{
				oss << "\t\t" << GenerateFieldSerializeCode(podField, "json_" + elementName, elementName);
			}
			oss << "\t\t" << jsonObjName << ".AddMember(std::move(json_" << elementName << "));\n";
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tJSONObject json_" << field.name << ";\n";
			oss << "\tSerialize(" << elementName << ", json_" << field.name << ");\n";
			oss << "\t" << jsonObjName << ".AddMember(std::move(json_" << field.name << "));\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tJSONArray arr_" << field.name << ";\n";
			oss << "\tfor(const auto& " << newElementName << " : " << elementName << ")\n\t{\n";
			oss << GenerateArraySerializeCode(field, "arr_" + field.name, newElementName, depth++) << "\n";
			oss << "\t}\n";
			oss << "\tarr" << field.name << ".AddMember(std::move(arr_" << field.name << "));\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tJSONObject json_" << field.name << ";\n";
			oss << "\tfor(const auto& [key, value] : " << elementName << ")\n\t{\n";
			oss << GenerateMapSerializeCode(field, "json_" + field.name, "key", "value") << "\n";
			oss << "\t}\n";
			oss << "\tarr" << field.name << ".AddMember(std::move(json_" << field.name << "));\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateMapSerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& keyName, const std::string& valueName, size_t depth)
	{
		std::ostringstream oss;

		std::string newKeyName = keyName + "_" + std::to_string(depth);
		std::string newValueName = valueName + "_" + std::to_string(depth);

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), JSONNumber(static_cast<double>(" << valueName << ")));\n";
			break;

		case SASTType::Bool:
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), JSONBool(" << valueName << "));\n";
			break;

		case SASTType::String:
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), JSONString(" << valueName << "))\n";
			break;

		case SASTType::POD:
			oss << "\t\tJSONObject json_" << valueName << ";\n";
			for (auto& podField : field.objectNode->fields)
			{
				oss << "\t\t" << GenerateFieldSerializeCode(podField, "json_" + valueName, valueName);
			}
			oss << "\t\t" << jsonObjName << ".AddMember(std::move(json_" << valueName << "));\n";
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tJSONObject json_" << field.name << ";\n";
			oss << "\tSerialize(" << valueName << ", json_" << field.name << ");\n";
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), std::move(json_" << field.name << "));\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tJSONArray arr_" << field.name << ";\n";
			oss << "\tfor(const auto& item : " << valueName << ")\n\t{\n";
			oss << GenerateArraySerializeCode(field, "arr_" + field.name, "item") << "\n";
			oss << "\t}\n";
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), std::move(arr_" << field.name << "));\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tJSONObject json_" << field.name << ";\n";
			oss << "\tfor(const auto& [" << newKeyName << "," << newValueName << "] : " << valueName << ")\n\t{\n";
			oss << GenerateMapSerializeCode(field, "json" + field.name, newKeyName, newValueName, depth++) << "\n";
			oss << "\t}\n";
			oss << "\t" << jsonObjName << ".AddMember(std::to_string(" << keyName << "), std::move(json_" << field.name << "));\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateArrayDeserializeCode(const SASTField& field, const std::string& receiverName, const std::string& elementName, size_t depth)
	{
		std::ostringstream oss;

		std::string newElementName = elementName + "_" + std::to_string(depth);

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << receiverName << " = " << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONNumber>().value;\n";
			break;

		case SASTType::Bool:
			oss << "\t" << receiverName << " = " << elementName << ".GetMembers(\"" << field.formattedName << "\").as<JSONBool>().value;\n";
			break;

		case SASTType::String:
			oss << "\t" << receiverName << " = " << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONString>().value;\n";
			break;

		case SASTType::POD:
			//oss << "\tconst JSONObject& json_" << field.name << " = *" << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			for (auto& podField : field.objectNode->fields)
			{
				podField.formattedName = field.name + "." + podField.name;
				oss << GenerateFieldDeserializeCode(podField, receiverName + "." + podField.formattedName, elementName);
			}
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tconst JSONObject& json_" << field.name << " = *" << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tDeserialize(" << receiverName << ", json_" << field.name << ");\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tconst JSONArray& arr_" << field.name << " = *" << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONArray>();\n";
			oss << "\tfor(const auto& " << newElementName << " : arr_" << field.name << ".GetItems())\n\t{\n";
			oss << GenerateArrayDeserializeCode(*field.elementType, receiverName + "." + field.name, newElementName, depth++) << "\n";
			oss << "\t}\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tconst JSONObject& json_" << field.name << " = *" << elementName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tfor(const auto& [key, value] : json_" << field.name << ".GetMembers())\n\t{\n";
			oss << GenerateMapDeserializeCode(*field.valueType, receiverName + "." + field.name, "key", "value") << "\n";
			oss << "\t}\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateMapDeserializeCode(const SASTField& field, const std::string& receiverName, const std::string& keyName, const std::string& valueName, size_t depth)
	{
		std::ostringstream oss;

		std::string newKeyName = keyName + "_" + std::to_string(depth);
		std::string newValueName = valueName + "_" + std::to_string(depth);

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << receiverName << " = " << valueName << ".GetMember(\"" << field.formattedName << "\").as<JSONNumber>().value;\n";
			break;

		case SASTType::Bool:
			oss << "\t" << receiverName << " = " << valueName << ".GetMembers(\"" << field.formattedName << "\").as<JSONBool>().value;\n";
			break;

		case SASTType::String:
			oss << "\t" << receiverName << " = " << valueName << ".GetMember(\"" << field.formattedName << "\").as<JSONString>().value;\n";
			break;

		case SASTType::POD:
			for (auto& podField : field.objectNode->fields)
			{
				podField.formattedName = field.name + "." + podField.name;
				oss << GenerateFieldDeserializeCode(podField, receiverName + "[" + podField.formattedName + "]", valueName);
			}
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tconst JSONObject& json_" << field.name << " = *" << valueName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tDeserialize(" << receiverName << ", json_" << field.name << ");\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tconst JSONArray& arr_" << field.name << " = *" << valueName << ".GetMember(\"" << field.formattedName << "\").as<JSONArray>();\n";
			oss << "\tfor(const auto& item : arr_" << field.name << ".GetItems())\n\t{\n";
			oss << GenerateArrayDeserializeCode(*field.elementType, receiverName + "." + field.name, "item") << "\n";
			oss << "\t}\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tconst JSONObject& json" << field.name << " = *" << valueName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tfor(const auto& [" << newKeyName << "," << newValueName << "] : json" << field.name << ".GetMembers())\n\t{\n";
			oss << "\t\t" << GenerateMapDeserializeCode(*field.valueType, receiverName + "." + field.name, newKeyName, newValueName, depth++) << "\n";
			oss << "\t}\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateFieldSerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& objName)
	{
		std::ostringstream oss;

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", JSONNumber(static_cast<double>(" << objName << "." << field.name << ")));\n";
			break;

		case SASTType::Bool:
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", JSONBool(" << objName << "." << field.name << "));\n";
			break;

		case SASTType::String:
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", JSONString(" << objName << "." << field.name << "));\n";
			break;

		case SASTType::POD:
			for (auto& podField : field.objectNode->fields)
			{
				podField.formattedName = field.name + "." + podField.name;
				oss << GenerateFieldSerializeCode(podField, jsonObjName, field.name);
			}
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tJSONObject json_" << field.name << ";\n";
			oss << "\tSerialize(" << objName << "." << field.name << ", json" << field.name << ");\n";
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", std::move(json_" << field.name << "));\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tJSONArray arr_" << field.name << ";\n";
			oss << "\tfor(const auto& item : " << objName << "." << field.name << ")\n\t{\n";
			oss << GenerateArraySerializeCode(*field.elementType, "arr_" + field.name, "item") << "\n";
			oss << "\t}\n";
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", std::move(arr_" << field.name << "));\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tJSONObject json" << field.name << ";\n";
			oss << "\tfor(const auto& [key, value] : " << objName << "." << field.name << ")\n\t{\n";
			oss << GenerateMapSerializeCode(*field.valueType, "json" + field.name, "key", "value") << "\n";
			oss << "\t}\n";
			oss << "\t" << jsonObjName << ".AddMember(\"" << field.formattedName << "\", std::move(json" << field.name << "));\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateFieldDeserializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& objName)
	{
		std::ostringstream oss;

		// Generate code based on the field's type
		// For primitives, we add a JSONNumber/JSONBool/JSONString directly
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << "\t" << objName << "." << field.name << " = " << jsonObjName << ".GetMember(\"" << field.formattedName << "\").as<JSONNumber>().value;\n";
			break;

		case SASTType::Bool:
			oss << "\t" << objName << "." << field.name << " = " << jsonObjName << ".GetMembers(\"" << field.formattedName << "\").as<JSONBool>().value;\n";
			break;

		case SASTType::String:
			oss << "\t" << objName << "." << field.name << " = " << jsonObjName << ".GetMember(\"" << field.formattedName << "\").as<JSONString>().value;\n";
			break;

		case SASTType::POD:
			for (auto& podField : field.objectNode->fields)
			{
				podField.formattedName = field.name + "." + podField.name;
				oss << GenerateFieldDeserializeCode(podField, jsonObjName, field.name);
			}
			break;

		case SASTType::Object:
			// Call the Serialize to JSONObject function for the contained object (field)
			oss << "\tconst JSONObject& json" << field.name << " = *" << jsonObjName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tDeserialize(" << objName << "." << field.name << ", json" << field.name << ");\n";
			break;

		case SASTType::Array:
		case SASTType::Vector:
		case SASTType::Set:
			// Loop over the items in the container and add then to a JSONArray
			oss << "\tconst JSONArray& arr" << field.name << " = *" << jsonObjName << ".GetMember(\"" << field.formattedName << "\").as<JSONArray>();\n";
			oss << "\tfor(const auto& item : arr" << field.name << ".GetItems())\n\t{\n";
			oss << "\t\t" << GenerateArrayDeserializeCode(*field.elementType, objName + "." + field.name, "item") << "\n";
			oss << "\t}\n";
			break;

		case SASTType::Map:
		case SASTType::Unordered_Map:
			oss << "\tconst JSONObject& json" << field.name << " = *" << jsonObjName << ".GetMember(\"" << field.formattedName << "\").as<JSONObject>();\n";
			oss << "\tfor(const auto& [key, value] : json" << field.name << ".GetMembers())\n\t{\n";
			oss << "\t\t" << GenerateMapDeserializeCode(*field.valueType, objName + "." + field.name, "key", "value") << "\n";
			oss << "\t}\n";
			break;

		default:
			throw std::runtime_error("Unsupported field type");
			break;
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateCode(const std::shared_ptr<SASTNode> sastNode)
	{
		std::ostringstream oss;

		// Function signature for the serialization function
		oss << "#include <fstream>\n\n";


		oss << "#include <JSONStructure.h>\n\n";

		// Generate the Serialize to JSONObject function
		oss << "static void JSONSerialize(const " << sastNode->name << "& obj, JSONObject& json)\n{\n";

		// Build a flattened list of fields: for POD types use only the node's fields,
		// otherwise add base class fields (only if accessible) then own fields
		std::vector<const SASTField*> flattenedFields;
		for (const auto& baseNode : sastNode->baseNodes)
		{
			for (const auto& field : baseNode->fields)
			{
				if (field.access != SASTField::Access::Private)
					flattenedFields.push_back(&field);
			}
		}
		for (const auto& field : sastNode->fields)
		{
			flattenedFields.push_back(&field);
		}

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldSerializeCode(*field, "json", "obj");
		}

		oss << "}\n\n";

		// Generate the Deserialize to JSONObject function
		oss << "static void JSONDeserialize(" << sastNode->name << "& obj, const JSONObject& json)\n{\n";

		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldDeserializeCode(*field, "json", "obj");
		}

		oss << "}\n\n";

		// Generate the Serialize to stream function
		oss << "static void JSONSerialize(const " << sastNode->name << "& obj, std::ostream& os)\n{\n";
		oss << "\tJSONStructure json;\n";

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldSerializeCode(*field, "json", "obj");
		}

		oss << "\tos << json.Stringify();\n";
		oss << "}\n\n";

		// Generate the Deserialize from stream function
		oss << "static void JSONDeserialize(" << sastNode->name << "& obj, const std::istream& is)\n{\n";
		oss << "\tJSONStructure json = JSONStructure::Parse(is);\n";

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldDeserializeCode(*field, "json", "obj");
		}

		oss << "}\n";

		return oss.str();
	}

	std::string JSONFormatPlugin::GetFormatName() const noexcept
	{
		return "JSON";
	}
}
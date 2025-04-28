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
	REGISTER_STATIC_PLUGIN(JSONFormatPlugin, 0);

	namespace
	{
		std::string GenerateKeyConversionFromString(const SASTField& keyField, const std::string& strExpr)
		{
			switch (keyField.type)
			{
			case SASTType::Int:
				return "std::stoi(" + strExpr + ")";

			case SASTType::Float:
				return "std::stof(" + strExpr + ")";

			case SASTType::Bool:
				return "(" + strExpr + " == \"true\")";

			case SASTType::String:
				return strExpr;

			default:
				throw std::runtime_error("Unsupported key type for conversion from string: " + keyField.originalTypeName);
			}
		}

		std::string GenerateKeyConversionToString(const SASTField& keyField, const std::string& keyExpr)
		{
			switch (keyField.type)
			{
			case SASTType::Int:
			case SASTType::Float:
			case SASTType::Bool:
				return "std::to_string(" + keyExpr + ")";

			case SASTType::String:
				return keyExpr;

			default:
				throw std::runtime_error("Unsupported key type for conversion to string: " + keyField.originalTypeName);
			}
		}

		std::string GenerateJsonObjInsertCode(const SASTField& field, const std::string& jsonReceiver, const std::string& fieldAccessor, size_t depth)
		{
			std::string indent(depth, '\t');

			switch (field.type)
			{
			case SASTType::Int:
			case SASTType::Float:
				return indent + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", JSONNumber(static_cast<double>(" + fieldAccessor + ")));\n";

			case SASTType::Bool:
				return indent + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", JSONBool(" + fieldAccessor + "));\n";

			case SASTType::String:
				return indent + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", JSONString(" + fieldAccessor + "));\n";

			case SASTType::POD:
			{
				std::string nestedPOD = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedPOD + "));\n" + indent + "}\n";
			}
			case SASTType::Object:
			{
				std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedObj + "));\n" + indent + "}\n";
			}
			case SASTType::Array:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Dynamic_Array:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Vector:
			case SASTType::Set:
			case SASTType::Unordered_Set:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Map:
			case SASTType::Unordered_Map:
			{
				std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(\"" + field.formattedName + "\", std::move(" + nestedObj + "));\n" + indent + "}\n";
			}
			default:
				throw std::runtime_error("Unsupported field type");
				break;
			}
		}

		std::string GenerateJsonArrayInsertCode(const SASTField& field, const std::string& jsonReceiver, const std::string& fieldAccessor, size_t depth)
		{
			std::string indent(depth, '\t');

			switch (field.type)
			{
			case SASTType::Int:
			case SASTType::Float:
				return indent + jsonReceiver + ".AddMember(JSONNumber(static_cast<double>(" + fieldAccessor + ")));\n";

			case SASTType::Bool:
				return indent + jsonReceiver + ".AddMember(JSONBool(" + fieldAccessor + "));\n";

			case SASTType::String:
				return indent + jsonReceiver + ".AddMember(JSONString(" + fieldAccessor + "))\n";

			case SASTType::POD:
			{
				std::string nestedPOD = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedPOD + "));\n" + indent + "}\n";
			}
			case SASTType::Object:
			{
				std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedObj + "));\n" + indent + "}\n";
			}
			case SASTType::Array:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Dynamic_Array:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Vector:
			case SASTType::Set:
			case SASTType::Unordered_Set:
			{
				std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedArray + "));\n" + indent + "}\n";
			}
			case SASTType::Map:
			case SASTType::Unordered_Map:
			{
				std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
				return indent + "\t" + jsonReceiver + ".AddMember(std::move(" + nestedObj + "));\n" + indent + "}\n";
			}
			default:
				throw std::runtime_error("Unsupported field type");
				break;
			}
		}
	}

	std::string JSONFormatPlugin::GenerateArrayAllocationCode(const SASTField& field, const std::string& arrayName, const std::string& jsonArrayName)
	{
		return arrayName + " = new " + field.elementType->originalTypeName + "[" + jsonArrayName + ".GetItems().size()];\n";
	}

	std::string JSONFormatPlugin::GenerateNewContainerElementCode(const SASTField& field, const std::string& containerName, const std::string& newElementReferenceName, const std::string& key)
	{
		switch (field.type)
		{
		case SASTType::Vector:
			return "auto& " + newElementReferenceName + " = " + containerName + ".emplace_back();\n";

		case SASTType::Set:
		case SASTType::Unordered_Set:
			return "auto& " + newElementReferenceName + " = " + containerName + ".emplace();\n";

		case SASTType::Map:
		case SASTType::Unordered_Map:
			if (key.empty())
				throw std::runtime_error("Key is required for Map or Unordered_Map element creation");
			return "auto& " + newElementReferenceName + " = " + containerName + "[" + GenerateKeyConversionFromString(*field.keyType, key) + "];\n";

		default:
			throw std::runtime_error("Unsupported container type for insertion");
		}
	}

	std::string JSONFormatPlugin::GenerateMemoryCleanupCode(const std::string& pointerName)
	{
		return "if (" + pointerName + ") delete[] " + pointerName + ";\n";
	}

	std::string JSONFormatPlugin::GenerateFieldSerializeCode(const SASTField& field, const std::string& jsonReceiver, const std::string& objSource, size_t depth, bool receiverIsJsonObj, bool generateInsertion)
	{
		std::ostringstream oss;

		std::string indent(depth, '\t');

		std::string fieldAccessor = objSource;
		fieldAccessor += field.name.empty() ? "" : "." + field.name;

		// Generate code based on the field's type
		switch (field.type)
		{
		case SASTType::POD:
		{
			std::string nestedPOD = (field.name.empty() ? "nested" : field.name) + "_json";
			oss << indent << "{\n";
			oss << indent << "\tJSONObject " << nestedPOD << ";\n";
			for (auto& podField : field.objectNode->fields)
			{
				oss << GenerateFieldSerializeCode(podField, nestedPOD, fieldAccessor, depth + 1);
			}
			break;
		}
		case SASTType::Object:
		{
			// Call the Serialize to JSONObject function for the contained object (field)
			std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
			oss << indent << "{\n";
			oss << indent << "\tJSONObject " << nestedObj << ";\n";
			oss << indent << "\tJSONSerialize(" << nestedObj << ", " << fieldAccessor << ");\n";
			break;
		}
		case SASTType::Array:
		{
			std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string i = "i_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tJSONArray " << nestedArray << ";\n";
			oss << indent << "\tfor(size_t " << i << " = 0; " << i << " < sizeof(" << fieldAccessor << ") / sizeof(" << fieldAccessor << "[0]); " << i << "++)\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldSerializeCode(*field.elementType, nestedArray, fieldAccessor + "[" + i + "]", depth + 2, false);
			oss << indent << "\t}\n";
			break;
		}
		case SASTType::Dynamic_Array:
		{
			std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string i = "i_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tJSONArray " << nestedArray << ";\n";
			oss << indent << "\tfor(size_t " << i << " = 0; " << i << " < " << field.lengthVar << "; " << i << "++)\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldSerializeCode(*field.elementType, nestedArray, fieldAccessor + "[" + i + "]", depth + 2, false);
			oss << indent << "\t}\n";
			break;
		}
		case SASTType::Vector:
		case SASTType::Set:
		case SASTType::Unordered_Set:
		{
			std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string item = "item_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tJSONArray " << nestedArray << ";\n";
			oss << indent << "\tfor(const auto& " << item << " : " << fieldAccessor << ")\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldSerializeCode(*field.elementType, nestedArray, item, depth + 2, false);
			oss << indent << "\t}\n";
			break;
		}
		case SASTType::Map:
		case SASTType::Unordered_Map:
		{
			std::string nestedObj = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string valueObj = (field.valueType->name.empty() ? "nested" : field.valueType->name) + "_json";
			std::string key = "key_" + std::to_string(depth);
			std::string value = "value_" + std::to_string(depth);
			field.keyType->formattedName = key;
			field.valueType->formattedName = value;
			oss << indent << "{\n";
			oss << indent << "\tJSONObject " << nestedObj << ";\n";
			oss << indent << "\tfor(const auto& [" << key << ", " << value << "] : " << fieldAccessor << ")\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldSerializeCode(*field.valueType, nestedObj, value, depth + 2, true, false);
			oss << indent << "\t\t\t" << nestedObj << ".AddMember(" << GenerateKeyConversionToString(*field.keyType, field.keyType->formattedName) << ", std::move(" << valueObj << "));\n";
			oss << indent << "\t\t}\n";
			oss << indent << "\t}\n";
			break;
		}
		default:
			break;
		}

		if (generateInsertion)
		{
			if (receiverIsJsonObj)
				oss << GenerateJsonObjInsertCode(field, jsonReceiver, fieldAccessor, depth);
			else
				oss << GenerateJsonArrayInsertCode(field, jsonReceiver, fieldAccessor, depth);
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateFieldDeserializeCode(const SASTField& field, const std::string& objReceiver, const std::string& jsonSource, size_t depth, bool sourceIsJsonObj, bool treatKeyAsString)
	{
		std::ostringstream oss;

		std::string indent(depth, '\t');

		std::string fieldAccessor = objReceiver;
		fieldAccessor += field.name.empty() ? "" : "." + field.name;
		std::string jsonAccessor = sourceIsJsonObj ? (treatKeyAsString ? jsonSource + ".GetMember(\"" + field.formattedName + "\")" : jsonSource + ".GetMember(" + field.formattedName + ")") : jsonSource;

		// Generate code based on the field's type
		switch (field.type)
		{
		case SASTType::Int:
		case SASTType::Float:
			oss << indent << fieldAccessor << " = " << jsonAccessor << ".as<JSONNumber>().value;\n";
			break;

		case SASTType::Bool:
			oss << indent << fieldAccessor << " = " << jsonAccessor << ".as<JSONBool>().value;\n";
			break;

		case SASTType::String:
			oss << indent << fieldAccessor << " = " << jsonAccessor << ".as<JSONString>().value;\n";
			break;

		case SASTType::POD:
		{
			for (auto& podField : field.objectNode->fields)
			{
				oss << GenerateFieldDeserializeCode(podField, fieldAccessor, jsonAccessor + ".as<JSONObject>()", depth);
			}
			break;
		}
		case SASTType::Object:
			oss << indent << "JSONDeserialize(" << fieldAccessor << ", " << jsonAccessor << ".as<JSONObject>());\n";
			break;

		case SASTType::Array:
		{
			std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string i = "i_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tJSONArray " << nestedArray << " = " << jsonAccessor << ".as<JSONArray>();\n";
			oss << indent << "\tfor(size_t " << i << " = 0; " << i << " < sizeof(" << fieldAccessor << ") / sizeof(" << fieldAccessor << "[0]); " << i << "++)\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldDeserializeCode(*field.elementType, fieldAccessor + "[" + i + "]", nestedArray + "[" + i + "]", depth + 2, false);
			oss << indent << "\t}\n";
			oss << indent << "}\n";
			break;
		}
		case SASTType::Dynamic_Array:
		{
			std::string nestedArray = (field.name.empty() ? "nested" : field.name) + "_json";
			std::string i = "i_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tJSONArray " << nestedArray << " = " << jsonAccessor << ".as<JSONArray>();\n";
			oss << indent << "\t" << GenerateMemoryCleanupCode(fieldAccessor);
			oss << indent << "\t" << GenerateArrayAllocationCode(field, fieldAccessor, nestedArray);
			oss << indent << "\t" << field.lengthVar << " = " << nestedArray << ".GetItems().size();\n";
			oss << indent << "\tfor(size_t " << i << " = 0; " << i << " < " << field.lengthVar << "; " << i << "++)\n";
			oss << indent << "\t{\n";
			oss << GenerateFieldDeserializeCode(*field.elementType, fieldAccessor + "[" + i + "]", nestedArray + "[" + i + "]", depth + 2, false);
			oss << indent << "\t}\n";
			oss << indent << "}\n";
			break;
		}
		case SASTType::Vector:
		case SASTType::Set:
		case SASTType::Unordered_Set:
		{
			std::string item = "item_" + std::to_string(depth);
			std::string temp = "elem_" + std::to_string(depth);
			oss << indent << "{\n";
			oss << indent << "\tfor(const auto& " << item << " : " << jsonAccessor << ".as<JSONArray>().GetItems())\n";
			oss << indent << "\t{\n";
			oss << indent << "\t\t" << GenerateNewContainerElementCode(field, fieldAccessor, temp);
			oss << GenerateFieldDeserializeCode(*field.elementType, temp, item, depth + 2, false);
			oss << indent << "\t}\n";
			oss << indent << "}\n";
			break;
		}
		case SASTType::Map:
		case SASTType::Unordered_Map:
		{
			std::string key = "key_" + std::to_string(depth);
			std::string value = "value_" + std::to_string(depth);
			std::string temp = "elem_" + std::to_string(depth);
			field.valueType->formattedName = key;
			oss << indent << "{\n";
			oss << indent << "\tfor(const auto& [" << key << ", " << value << "] : " << jsonAccessor << ".as<JSONObject>().GetMembers())\n";
			oss << indent << "\t{\n";
			oss << indent << "\t\t" << GenerateNewContainerElementCode(field, fieldAccessor, temp, key);
			oss << GenerateFieldDeserializeCode(*field.valueType, temp, value, depth + 2, false, false);
			oss << indent << "\t}\n";
			oss << indent << "}\n";
			break;
		}
		default:
			throw std::runtime_error("Unsupported field type");
		}

		return oss.str();
	}

	std::string JSONFormatPlugin::GenerateCode(const std::shared_ptr<SASTNode> sastNode)
	{
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

		std::ostringstream oss;

		// Function signature for the serialization function
		oss << "#include <fstream>\n\n";


		oss << "#include <JSONStructure.h>\n\n";

		// Generate the Serialize to JSONObject function
		oss << "static void JSONSerialize(JSONObject& jsonReceiver, const " << sastNode->name << "& objSource)\n";
		oss << "{\n";

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldSerializeCode(*field, "jsonReceiver", "objSource") << "\n";
		}

		oss << "}\n\n";

		// Generate the Deserialize to JSONObject function
		oss << "static void JSONDeserialize(" << sastNode->name << "& objReceiver, const JSONObject& jsonSource)\n";
		oss << "{\n";

		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldDeserializeCode(*field, "objReceiver", "jsonSource") << "\n";
		}

		oss << "}\n\n";

		// Generate the Serialize to stream function
		oss << "static void JSONSerialize(std::ostream& osReceiver, const " << sastNode->name << "& objSource)\n";
		oss << "{\n";
		oss << "\tJSONStructure jsonRep;\n";

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldSerializeCode(*field, "jsonRep", "objSource") << "\n";
		}

		oss << "\tosReceiver << jsonRep.Stringify();\n";
		oss << "}\n\n";

		// Generate the Deserialize from stream function
		oss << "static void JSONDeserialize(" << sastNode->name << "& objReceiver, const std::istream& isSource)\n";
		oss << "{\n";
		oss << "\tJSONStructure jsonRep = JSONStructure::Parse(isSource);\n";

		// Generate code for each flattened field
		for (const auto& field : flattenedFields)
		{
			oss << GenerateFieldDeserializeCode(*field, "objReceiver", "jsonRep") << "\n";
		}

		oss << "}\n";

		return oss.str();
	}

	std::string JSONFormatPlugin::GetFormatName() const noexcept
	{
		return "JSON";
	}

	uint8_t JSONFormatPlugin::GetPluginPriority() const noexcept
	{
		return 0;
	}
}
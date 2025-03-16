#ifndef GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H
#define GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H

#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	class FORMAT_PLUGIN_ABI JSONFormatPlugin : public IFormatPlugin
	{
	private:
		std::string GenerateArraySerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& elementName, size_t depth = 1);

		std::string GenerateMapSerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& keyName, const std::string& valueName, size_t depth = 1);

		std::string GenerateArrayDeserializeCode(const SASTField& field, const std::string& receiverName, const std::string& elementName, size_t depth = 1);

		std::string GenerateMapDeserializeCode(const SASTField& field, const std::string& receiverName, const std::string& keyName, const std::string& valueName, size_t depth = 1);

		// Helper function to generate code for serializing one field
		std::string GenerateFieldSerializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& objName);

		// Helper function to generate code for deserializing one field
		std::string GenerateFieldDeserializeCode(const SASTField& field, const std::string& jsonObjName, const std::string& objName);

	public:
		/// <summary>
		/// Generate serialization code for the given SAST node
		/// </summary>
		/// <param name="sastNode">The SAST node to generate code for</param>
		/// <returns>The generated code</returns>
		std::string FORMAT_PLUGIN_CALL GenerateCode(const std::shared_ptr<SASTNode> sastNode) final;

		/// <summary>
		/// Get the name of the format (for instance JSON)
		/// </summary>
		/// <returns>Name of the format this plugin hangles</returns>
		std::string FORMAT_PLUGIN_CALL GetFormatName() const noexcept final;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H

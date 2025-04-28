#ifndef GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H
#define GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H

#include <IFormatPlugin.h>
#include <FileFormatRegistry.h>

#include <string>

namespace GenTools::GenSerialize
{
	class FORMAT_PLUGIN_ABI JSONFormatPlugin : public IFormatPlugin
	{
	protected:
		// Extention points for polymorphic behavior
		virtual std::string GenerateArrayAllocationCode(const SASTField& field, const std::string& arrayName, const std::string& jsonArrayName);
		virtual std::string GenerateNewContainerElementCode(const SASTField& field, const std::string& containerName, const std::string& newElementReferenceName, const std::string& key = "");
		virtual std::string GenerateMemoryCleanupCode(const std::string& pointerName);

		/// <summary>
		/// Helper for generating the serialization code for a given field
		/// </summary>
		/// <param name="field">The field in the source object to generate serialization logic for</param>
		/// <param name="jsonReceiver">The literal text to access the json object to serialize the field into</param>
		/// <param name="objSource">The literal text to access the field in the source object</param>
		/// <param name="depth">Indicates the level of recursion for this call to this function</param>
		/// <param name="receiverIsJsonObj">Indicates if the jsonReceiver is a JSON object, if false, it is a JSON array</param>
		/// <param name="generateInsertion">Indicates that this call to the funciton should generate json object insertion code, or stop after generating field logic</param>
		/// <returns>The serialization logic for the field entered</returns>
		virtual std::string GenerateFieldSerializeCode(const SASTField& field, const std::string& jsonReceiver, const std::string& objSource, size_t depth = 1, bool receiverIsJsonObj = true, bool generateInsertion = true);

		/// <summary>
		/// Helper for generating the deserialization code for a given field
		/// </summary>
		/// <param name="field">The field in the source object to generate serialization logic for</param>
		/// <param name="objReceiver">The literal text to access the field in the object to be deserialized into</param>
		/// <param name="jsonSource">The literal text to access the json object which is the source for the deserialization</param>
		/// <param name="depth">Indicates the level of recursion for this call of this function</param>
		/// <param name="sourceIsJsonObj">Indicates if the jsonSource is a JSON object, if false, it is a JSON array</param>
		/// <param name="treatKeyAsString">Indicates if the key used to retrieve an element from a JSON object is treated as a string literal (true), or a string reference (false)</param>
		/// <returns>The deserialization logic for the field entered</returns>
		virtual std::string GenerateFieldDeserializeCode(const SASTField& field, const std::string& objReceiver, const std::string& jsonSource, size_t depth = 1, bool sourceIsJsonObj = true, bool treatKeyAsString = true);

	public:
		/// <summary>
		/// Generate serialization code for the given SAST node
		/// </summary>
		/// <param name="sastNode">The SAST node to generate code for</param>
		/// <returns>The generated code</returns>
		std::string FORMAT_PLUGIN_CALL GenerateCode(const std::shared_ptr<SASTNode> sastNode) override;

		/// <summary>
		/// Get the name of the format (for instance JSON)
		/// </summary>
		/// <returns>Name of the format this plugin hangles</returns>
		std::string FORMAT_PLUGIN_CALL GetFormatName() const noexcept override;

		/// <summary>
		/// Get the priority level assigned to the plugin. Higher priorities can override lower priority plugins with the same format name
		/// </summary>
		/// <returns>Priority level (default = 0)</returns>
		virtual uint8_t FORMAT_PLUGIN_CALL GetPluginPriority() const noexcept override;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_JSON_FORMAT_PLUGIN_H

#ifndef GENTOOLS_GENSERIALIZE_JSON_OBJECT_H
#define GENTOOLS_GENSERIALIZE_JSON_OBJECT_H

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <string_view>

#include <JSONValue.h>

#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	class JSONStructure;
	class Arg_JSONArray;

	class FORMAT_PLUGIN_ABI JSONObject : public JSONValue
	{
	public:
		using ObjectType = std::unordered_map<std::string, std::unique_ptr<JSONValue>>;

	private:
		ObjectType m_members;

		// Helper: Skip whitespace and update the iterator
		static void SkipWhitespace(std::string_view& view, std::string_view::const_iterator& itr) noexcept;

		// Helper: Extract a balanced substring starting with openCh until matching closeCh
		static std::string_view ExtractBalanced(std::string_view view, std::string_view::const_iterator& itr, char openCh, char closeCh);

	public:
		JSONObject() = default;
		JSONObject(JSONObject&&) = default;

		JSONObject& operator=(JSONObject&&) = default;

		template<IsJSONValue Value>
		void AddMember(std::string&& key, Value&& value) noexcept;

		const ObjectType& GetMembers() const noexcept;
		ObjectType TakeMembers() noexcept;

		JSONType Type() const noexcept final;

		std::string StringifyHelper(int level, int indent) const;

		// Parse a JSON object from a complete string (including the leading '{')
		void Parse(std::string_view& jsonView);

		std::unique_ptr<JSONValue>& GetMember(const std::string& key);
		std::unique_ptr<JSONValue>& operator[](const std::string& key);

		JSONObject(const JSONObject&) = delete;
		JSONObject& operator=(const JSONObject&) = delete;
	};
}

#include <JSONObject.inl>

#endif // !GENTOOLS_GENSERIALIZE_JSON_OBJECT_H

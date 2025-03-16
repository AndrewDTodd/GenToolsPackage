#ifndef GENTOOLS_GENSERIALIZE_JSON_ARRAY_H
#define GENTOOLS_GENSERIALIZE_JSON_ARRAY_H

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <stdexcept>

#include <JSONValue.h>

#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	class FORMAT_PLUGIN_ABI JSONArray : public JSONValue
	{
	public:
		using ArrayType = std::vector<std::unique_ptr<JSONValue>>;

	private:
		ArrayType m_items;

		static void SkipWhitespace(std::string_view& view, std::string_view::const_iterator& itr) noexcept;

		static std::string_view ExtractBalanced(std::string_view view, std::string_view::const_iterator& itr, char openCh, char closeCh);

	public:
		JSONArray() = default;
		JSONArray(JSONArray&&) = default;

		JSONArray& operator=(JSONArray&&) = default;

		template<IsJSONValue... Value>
		JSONArray(Value&&... value) noexcept;

		template<IsJSONValue Value>
		void AddMember(Value&& value) noexcept;

		const ArrayType& GetItems() const noexcept;

		std::string StringifyHelper(int level, int indent) const;

		// Parse a JSON array from a complete string (including the leading '[')
		void Parse(std::string_view& jsonView);

		JSONType Type() const noexcept final;

		std::unique_ptr<JSONValue>& GetMember(size_t index);
		std::unique_ptr<JSONValue>& operator[](size_t index);

		JSONArray(const JSONArray&) = delete;
		JSONArray& operator=(const JSONArray&) = delete;
	};
}

#include <JSONArray.inl>

#endif // !GENTOOLS_GENSERIALIZE_JSON_ARRAY_H

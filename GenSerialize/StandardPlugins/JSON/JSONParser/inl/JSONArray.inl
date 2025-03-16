#ifndef GENTOOLS_GENSERIALIZE_JSON_ARRAY_INL
#define GENTOOLS_GENSERIALIZE_JSON_ARRAY_INL

#include <StringifyValue.h>

#if defined(__GNUC__) or defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif

namespace GenTools::GenSerialize
{
	FORCE_INLINE void JSONArray::SkipWhitespace(std::string_view& view, std::string_view::const_iterator& itr) noexcept
	{
		while (itr != view.end() && std::isspace(*itr))
			++itr;
	}

	FORCE_INLINE std::string_view JSONArray::ExtractBalanced(std::string_view view, std::string_view::const_iterator& itr, char openCh, char closeCh)
	{
		auto start = itr; // Includes the opening delimiter
		int count = 0;
		bool inString = false;
		for (; itr != view.end(); ++itr)
		{
			char c = *itr;
			if (c == '"')
				inString = !inString;
			else if (!inString)
			{
				if (c == openCh)
					++count;
				else if (c == closeCh)
				{
					--count;
					if (count == 0)
					{
						++itr;
						// Calculate the starting index and length for the substring.
						auto offset = std::distance(view.begin(), start);
						auto length = std::distance(start, itr);
						return view.substr(offset, length);
					}
				}
			}
		}

		throw std::invalid_argument("Invalid JSON: Unterminated structure");
	}

	template<IsJSONValue Value>
	FORCE_INLINE void JSONArray::AddMember(Value&& value) noexcept
	{
		m_items.emplace_back(std::move(value));
	}

	template<IsJSONValue... Value>
	FORCE_INLINE JSONArray::JSONArray(Value&&... value) noexcept
	{
		(AddValueToArray(std::forward<Value>(value)), ...);
	}

	FORCE_INLINE const JSONArray::ArrayType& JSONArray::GetItems() const noexcept
	{
		return m_items;
	}

	FORCE_INLINE JSONType JSONArray::Type() const noexcept
	{
		return JSONType::Array;
	}

	FORCE_INLINE std::string JSONArray::StringifyHelper(int level, int indent) const
	{
		std::string result = "[\n";
		std::string pad((level + 1) * indent, ' ');
		bool first = true;

		for (const auto& item : m_items)
		{
			if (!first)
				result += ",\n";
			first = false;

			result += pad + JSON::StringifyValue(item, level + 1, indent);
		}

		result += "\n" + std::string(level * indent, ' ') + "]";
		return result;
	}

	FORCE_INLINE std::unique_ptr<JSONValue>& JSONArray::GetMember(size_t index)
	{
		if (index >= m_items.size())
			throw std::out_of_range("Index out of range");
		return m_items[index];
	}

	FORCE_INLINE std::unique_ptr<JSONValue>& JSONArray::operator[](size_t index)
	{
		return GetMember(index);
	}
}

#endif // !GENTOOLS_GENSERIALIZE_JSON_ARRAY_INL

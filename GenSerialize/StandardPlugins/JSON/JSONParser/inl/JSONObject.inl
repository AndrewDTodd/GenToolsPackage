#ifndef GENTOOLS_GENSERIALIZE_JSON_OBJECT_INL
#define GENTOOLS_GENSERIALIZE_JSON_OBJECT_INL

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
	FORCE_INLINE void JSONObject::SkipWhitespace(std::string_view& view, std::string_view::const_iterator& itr) noexcept
	{
		while (itr != view.end() && std::isspace(*itr))
			++itr;
	}

	FORCE_INLINE std::string_view JSONObject::ExtractBalanced(std::string_view view, std::string_view::const_iterator& itr, char openCh, char closeCh)
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
	FORCE_INLINE void JSONObject::AddMember(std::string&& key, Value&& value) noexcept
	{
		m_members[key] = std::move(std::make_unique<Value>(std::move(value)));
	}

	FORCE_INLINE const JSONObject::ObjectType& JSONObject::GetMembers() const noexcept
	{
		return m_members;
	}

	FORCE_INLINE JSONObject::ObjectType JSONObject::TakeMembers() noexcept
	{
		return std::move(m_members);
	}

	FORCE_INLINE JSONType JSONObject::Type() const noexcept
	{
		return JSONType::Object;
	}

	FORCE_INLINE std::string JSONObject::StringifyHelper(int level, int indent) const
	{
		std::string result = "{\n";
		std::string pad((level + 1) * indent, ' '); // Indentation for current level
		bool first = true;

		for (const auto& [key, value] : m_members)
		{
			if (!first)
				result += ",\n";
			first = false;

			result += pad + "\"" + key + "\": " + JSON::StringifyValue(value, level + 1, indent);
		}

		result += "\n" + std::string(level * indent, ' ') + "}";
		return result;
	}

	FORCE_INLINE std::unique_ptr<JSONValue>& JSONObject::GetMember(const std::string& key)
	{
		auto it = m_members.find(key);
		if (it != m_members.end())
			return it->second;
		
		throw std::runtime_error("Key not found in JSON object: " + key);
	}

	FORCE_INLINE std::unique_ptr<JSONValue>& JSONObject::operator[](const std::string& key)
	{
		auto it = m_members.find(key);
		if (it == m_members.end())
		{
			// Create and insert a new flag_argument (default to some derived type)
			it = m_members.emplace(key, nullptr).first;
		}
		return it->second;
	}
}

#endif // !GENTOOLS_GENSERIALIZE_ARG_JSON_OBJECT_INL

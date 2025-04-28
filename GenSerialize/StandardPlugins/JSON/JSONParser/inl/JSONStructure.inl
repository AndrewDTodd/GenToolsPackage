#ifndef GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_INL
#define GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_INL

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
    template<IsJSONValue Value>
    FORCE_INLINE void JSONStructure::AddMember(std::string&& key, Value&& value) noexcept
    {
        m_members[key] = std::move(std::make_unique<Value>(std::move(value)));
    }

    FORCE_INLINE JSONStructure JSONStructure::Parse(std::istream& stream)
    {
        JSONStructure jsonRoot;
        std::string jsonText((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        std::string_view jsonView(jsonText);
        // Pass the complete JSON text (which should start with '{') to the Parse routine
        JSONObject obj;
        obj.Parse(jsonView);
        jsonRoot.m_members = obj.TakeMembers();
        return jsonRoot;
    }

    FORCE_INLINE JSONStructure JSONStructure::FromFile(std::filesystem::path& path)
    {
        if (std::filesystem::exists(path))
        {
            std::ifstream fstream(path);
            if (fstream.is_open())
                return Parse(fstream);
            else
                throw std::runtime_error("Failed to open file at " + path.string());
        }
        else
            throw std::invalid_argument("Invalid file path provided. " + path.string() + " does not exist");
    }

    FORCE_INLINE std::unique_ptr<JSONValue>& JSONStructure::GetMember(const std::string& key)
    {
        auto it = m_members.find(key);
        if (it != m_members.end())
            return it->second;

		throw std::runtime_error("Key not found in JSON object: " + key);
    }

    FORCE_INLINE std::unique_ptr<JSONValue>& JSONStructure::operator[](const std::string& key)
    {
        auto it = m_members.find(key);
        if (it == m_members.end())
        {
            // Create and insert a new flag_argument (default to some derived type)
            it = m_members.emplace(key, nullptr).first;
        }
        return it->second;
    }

    FORCE_INLINE std::string JSONStructure::StringifyHelper(int level, int indent) const
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

    FORCE_INLINE std::string JSONStructure::Stringify(int indent) const
    {
        return StringifyHelper(0, indent);
    }

    FORCE_INLINE void JSONStructure::WriteToFile(const std::filesystem::path& path) const
    {
        std::ofstream file(path, std::ios::out | std::ios::trunc); // Open file for writing (overwrite if it exists)
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + path.string());

        file << Stringify(); // Write formatted JSON string to file
        file.close();
    }
}

#endif // !GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_INL

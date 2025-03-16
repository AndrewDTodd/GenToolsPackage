#ifndef GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_H
#define GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_H

#include <unordered_map>
#include <memory>
#include <string>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdint.h>

#include <JSONValue.h>

#include <JSONArray.h>
#include <JSONObject.h>

#include <PlatformInterface.h>

#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	class FORMAT_PLUGIN_ABI JSONStructure
	{
	private:
		std::unordered_map<std::string, std::unique_ptr<JSONValue>> m_members;

	public:
		JSONStructure() = default;
		JSONStructure(JSONStructure&&) = default;

		JSONStructure& operator=(JSONStructure&&) = default;

		template<IsJSONValue Value>
		void AddMember(std::string&& key, Value&& value) noexcept;

		static JSONStructure Parse(std::istream& stream);
		static JSONStructure FromFile(std::filesystem::path& path);

		std::unique_ptr<JSONValue>& GetMember(const std::string& key);
		std::unique_ptr<JSONValue>& operator[](const std::string& key);

		std::string Stringify(int indent = 4) const;
		std::string StringifyHelper(int level, int indent) const;

		void WriteToFile(const std::filesystem::path& path) const;

		JSONStructure(const JSONStructure&) = delete;
		JSONStructure& operator=(const JSONStructure&) = delete;
	};
}

#include <JSONStructure.inl>

#endif // !GENTOOLS_GENSERIALIZE_JSON_STRUCTURE_H

#include <FileValidator.h>

#include <fstream>
#include <sstream>
#include <regex>
#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
	FileValidator::FileValidator(const std::filesystem::path& sourceFilePath)
		: m_sourceFilePath(sourceFilePath)
	{}

	bool FileValidator::ValidateFile()
	{
		std::ifstream file(m_sourceFilePath);

		if (!file)
		{
			TERMINAL::PRINT_ERROR("Error: Could not open file" + m_sourceFilePath.string() + " for validation");
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string fileContents = buffer.str();

		std::string headerName = m_sourceFilePath.stem().string();

		// Regex to match an #include directive that contains headerName.generated.h
		std::regex includeRegex(R"regex(#include\s*[<"].*)" + headerName + R"regex(\.generated\.h[>"].*)regex");

		// Regex to match GENERATED_SERIALIZATION_BODY() (with optional spaces)
		std::regex bodyRegex(R"regex(GENERATED_SERIALIZATION_BODY\s*\(\s*\))regex");

		bool foundGeneratedHeader = std::regex_search(fileContents, includeRegex);
		bool foundGeneratedBody = std::regex_search(fileContents, bodyRegex);

		if (!foundGeneratedHeader)
		{
			TERMINAL::PRINT_ERROR("ERROR: File " + m_sourceFilePath.filename().string() + " contains serializable objects but does not include the generated header file");
			return false;
		}

		if (!foundGeneratedBody)
		{
			TERMINAL::PRINT_ERROR("ERROR: File " + m_sourceFilePath.filename().string() + " contains serializable objects but does not contain the GENERATED_SERIALIZATION_BODY() macro");
			return false;
		}

		return true;
	}
}
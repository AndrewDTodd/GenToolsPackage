#include <FileValidator.h>

#include <fstream>
#include <sstream>
#include <regex>
#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
	bool FileValidator::ValidateFile(const std::filesystem::path& sourceFilePath)
	{
		std::ifstream file(sourceFilePath);

		if (!file)
		{
			TERMINAL::PRINT_ERROR("Error: Could not open file " + sourceFilePath.string() + " for validation");
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string fileContents = buffer.str();

		std::string headerName = sourceFilePath.stem().string();

		// Regex to match #include <dir/sub/FileName.generated.h> or #include "FileName.generated.h"
		std::regex includeRegex("#include\\s*[<\"]([a-zA-Z0-9_/]*)" + headerName + "\\.generated\\.h[>\"]");

		// Regex to match GENERATED_SERIALIZATION_BODY() with optional spaces
		std::regex bodyRegex(R"(_SERIALIZATION_BODY\s*\(\s*\))");

		bool foundGeneratedHeader = std::regex_search(fileContents, includeRegex);
		bool foundGeneratedBody = std::regex_search(fileContents, bodyRegex);

		if (!foundGeneratedHeader)
		{
			TERMINAL::PRINT_ERROR("ERROR: File " + sourceFilePath.filename().string() + " contains serializable objects but does not include the generated header file");
			return false;
		}

		if (!foundGeneratedBody)
		{
			TERMINAL::PRINT_ERROR("ERROR: File " + sourceFilePath.filename().string() + " contains serializable objects but does not contain the GENERATED_SERIALIZATION_BODY() macro");
			return false;
		}

		return true;
	}
}
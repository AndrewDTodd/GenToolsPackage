#ifndef GENTOOLS_GENSERIALIZE_GENERATED_FILE_MANAGER_H
#define GENTOOLS_GENSERIALIZE_GENERATED_FILE_MANAGER_H

#include <GeneratedCode.h>
#include <filesystem>
#include <string>

namespace GenTools::GenSerialize
{
	class GeneratedFileManager
	{
	private:
		std::filesystem::path m_sourceFilePath;

		// Utility function to compute the path of the generated file
		std::filesystem::path GetGeneratedHeaderPath() const;

	public:
		explicit GeneratedFileManager(const std::filesystem::path& sourceFilePath);

		bool UpdateGeneratedFile(const GeneratedCode& generatedCode) const;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_GENERATED_FILE_MANAGER_H

#ifndef GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H
#define GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H

#include <filesystem>
#include <string>

namespace GenTools::GenSerialize
{
	class FileValidator
	{
	private:
		std::filesystem::path m_sourceFilePath;

	public:
		explicit FileValidator(const std::filesystem::path& sourceFilePath);

		bool ValidateFile();
	};
}

#endif // !GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H

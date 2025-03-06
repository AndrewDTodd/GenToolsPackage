#ifndef GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H
#define GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H

#include <filesystem>
#include <string>

namespace GenTools::GenSerialize
{
	class FileValidator
	{
	private:

	public:
		explicit FileValidator() = default;

		bool ValidateFile(const std::filesystem::path& sourceFilePath);
	};
}

#endif // !GENTOOLS_GENSERIALIZE_FILE_VALIDATOR_H

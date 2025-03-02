#include <GeneratedFileManager.h>

#include <fstream>
#include <sstream>
#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
	std::filesystem::path GeneratedFileManager::GetGeneratedHeaderPath() const
	{
		// For example, if source file is "MyClass.cpp", then header is "generated/MyClass.generated.h"
		return m_sourceFilePath.parent_path() / "generated" / (m_sourceFilePath.stem().string() + ".generated.h");
	}

	GeneratedFileManager::GeneratedFileManager(const std::filesystem::path& sourceFilePath)
		: m_sourceFilePath(sourceFilePath)
	{}

	bool GeneratedFileManager::UpdateGeneratedFile(const GeneratedCode& generatedCode) const
	{
		auto genPath = GetGeneratedHeaderPath();
		std::ofstream outFile(genPath);
		if (!outFile.is_open())
		{
			TERMINAL::PRINT_ERROR("ERROR: Could not open " + genPath.string() + " for writing.");
			return false;
		}

		// Write out a file header and then the generated code
		outFile << "// Auto-generated serialization code for " << m_sourceFilePath.filename().string() << ". Do not modify manually\n\n";
		for (const auto& pair : generatedCode.code)
		{
			const auto& format = pair.first;
			const auto& code = pair.second;
			outFile << "// Format: " << format << "\n";
			outFile << code << "\n\n";
		}

		outFile.close();
		return true;
	}
}
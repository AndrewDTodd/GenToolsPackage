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
		std::filesystem::create_directories(genPath.parent_path());
		std::ofstream outFile(genPath);
		if (!outFile.is_open())
		{
			TERMINAL::PRINT_ERROR("ERROR: Could not open " + genPath.string() + " for writing.");
			return false;
		}

		// Write put file header
		outFile << "// Auto-generated serialization code for " << m_sourceFilePath.filename().string() << ". DO NOT MODIFY MANUALLY\n\n";

		if (generatedCode.code.size() == 1)
		{
			// Single type ? standard GENERATED_SERIALIZATION_BODY macro
			auto& [typeName, formatMap] = *generatedCode.code.begin();

			outFile << "#define GENERATED_SERIALIZATION_BODY() \\\n";
			for (const auto& [format, code] : formatMap)
			{
				outFile << "/* Format: " << format << " */ \\\n";
				outFile << code << "\n";
			}
		}
		else
		{
			// Multiple types ? create macro per type
			for (const auto& [typeName, formatMap] : generatedCode.code)
			{
				outFile << "#define " << typeName << "_SERIALIZATION_BODY() \\\n";
				for (const auto& [format, code] : formatMap)
				{
					outFile << "/* Format: " << format << " */ \\\n";
					outFile << code << "\n";
				}
				outFile << "\n";
			}
		}

		outFile.close();
		return true;
	}
}
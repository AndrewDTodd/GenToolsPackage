#include <CodeGenerator.h>

#include <sstream>

#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
	CodeGenerator::CodeGenerator(const std::vector<std::shared_ptr<SASTNode>>& SASTNodes)
		: m_SASTNodes(SASTNodes)
	{}

	GeneratedCode CodeGenerator::GenerateCode()
	{
		GeneratedCode generated;

		// For each SAST node (each marked type)
		for (const auto& node : m_SASTNodes)
		{
			std::string typeName = node->name;

			// For each format specified for the type
			for (const auto& format : node->formats)
			{
				auto plugin = FileFormatRegistry::GetInstance().GetPlugin(format);
				if (plugin)
				{
					std::string code = plugin->GenerateCode(node);

					std::ostringstream escapedStream;
					std::istringstream rawStream(code);
					std::string line;
					while (std::getline(rawStream, line))
					{
						escapedStream << line << " \\\n"; // Escape newlines for macro continuation
					}

					generated.code[typeName][format] = escapedStream.str();
					
				}
				else
				{
					TERMINAL::PRINT_ERROR("Error: No plugin found for format: " + format);
				}
			}
		}

		return generated;
	}
}
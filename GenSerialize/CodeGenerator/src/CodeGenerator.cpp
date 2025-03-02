#include <CodeGenerator.h>

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
			// For each format specified for the type
			for (const auto& format : node->formats)
			{
				auto plugin = FileFormatRegistry::GetInstance().GetPlugin(format);
				if (plugin)
				{
					std::string code = plugin->GenerateCode(node);

					generated.code[format] += code;
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
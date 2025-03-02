#ifndef GENTOOLS_GENSERIALIZE_CODE_GENERATOR_H
#define GENTOOLS_GENSERIALIZE_CODE_GENERATOR_H

#include <SAST.h>
#include <GeneratedCode.h>
#include <FileFormatRegistry.h>
#include <vector>
#include <memory>
#include <string>

namespace GenTools::GenSerialize
{
	class CodeGenerator
	{
	private:
		const std::vector<std::shared_ptr<SASTNode>>& m_SASTNodes;

	public:
		CodeGenerator(const std::vector<std::shared_ptr<SASTNode>>& SASTNodes);

		GeneratedCode GenerateCode();
	};
}

#endif // !GENTOOLS_GENSERIALIZE_CODE_GENERATOR_H

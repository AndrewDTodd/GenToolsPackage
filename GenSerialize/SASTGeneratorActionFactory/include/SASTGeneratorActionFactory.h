#ifndef GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_FACTORY_H
#define GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_FACTORY_H

#include <SASTGeneratorAction.h>
#include <SAST.h>

#include <clang/Tooling/Tooling.h>

namespace GenTools::GenSerialize
{
	class SASTGeneratorActionFactory : public clang::tooling::FrontendActionFactory
	{
	private:
		std::mutex m_resultsMutex;
		std::vector<SASTResult> m_results;

	public:
		SASTGeneratorActionFactory() = default;

		std::unique_ptr<clang::FrontendAction> create() override;

		void MergeResults(
			std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>>& globalSASTTrees,
			std::unordered_map<std::string, std::shared_ptr<SASTNode>>& globalSASTMap
		);
	};
}

#endif // !GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_FACTORY_H

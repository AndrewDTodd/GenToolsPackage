#include <SASTGeneratorActionFactory.h>

namespace GenTools::GenSerialize
{
	std::unique_ptr<clang::FrontendAction> SASTGeneratorActionFactory::create()
	{
		std::lock_guard<std::mutex> lock(m_resultsMutex);

		m_results.emplace_back();

		return std::make_unique<SASTGeneratorAction>(m_results.back());
	}

	void SASTGeneratorActionFactory::MergeResults(
		std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>>& globalSASTTrees,
		std::unordered_map<std::string, std::shared_ptr<SASTNode>>& globalSASTMap
	)
	{
		for (auto& result : m_results)
		{
			globalSASTTrees[result.filePath] = result.SASTTree;

			for (const auto& [name, node] : result.SASTMap)
			{
				globalSASTMap[name] = node;
			}
		}
	}
}
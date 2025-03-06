#ifndef GENTOOLS_GENSERIALIZE_SAST_LINKER_H
#define GENTOOLS_GENSERIALIZE_SAST_LINKER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <SAST.h>

namespace GenTools::GenSerialize
{
	class SASTLinker
	{
	private:
		std::vector<std::shared_ptr<SASTNode>>& m_localSASTTree;
		const std::unordered_map<std::string, std::shared_ptr<SASTNode>>& m_globalSASTMap;

	public:
		SASTLinker(std::vector<std::shared_ptr<SASTNode>>& localSASTTree, const std::unordered_map<std::string, std::shared_ptr<SASTNode>>& globalSASTMap);

		void Link();
	};
}

#endif // !GENTOOLS_GENSERIALIZE_SAST_LINKER_H

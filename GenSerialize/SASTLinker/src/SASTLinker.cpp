#include <SASTLinker.h>

namespace GenTools::GenSerialize
{
	SASTLinker::SASTLinker(std::vector<std::shared_ptr<SASTNode>>& localSASTTree, const std::unordered_map<std::string, std::shared_ptr<SASTNode>>& globalSASTMap)
		: m_localSASTTree(localSASTTree), m_globalSASTMap(globalSASTMap)
	{}

	void SASTLinker::Link()
	{
		for (auto& node : m_localSASTTree)
		{
			for (auto& field : node->fields)
			{
				if (field.type == SASTType::Object && !field.objectNode)
				{
					auto it = m_globalSASTMap.find(field.originalTypeName);
					if (it != m_globalSASTMap.end())
					{
						field.objectNode = it->second;
					}
				}
			}
		}
	}
}
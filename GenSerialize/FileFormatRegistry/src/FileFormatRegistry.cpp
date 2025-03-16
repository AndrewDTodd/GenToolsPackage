#include <FileFormatRegistry.h>

namespace GenTools::GenSerialize
{
	FileFormatRegistry& FileFormatRegistry::GetInstance()
	{
		static FileFormatRegistry instance;
		return instance;
	}

	void FileFormatRegistry::RegisterPlugin(std::shared_ptr<IFormatPlugin> plugin)
	{
		m_formatPlugins[plugin->GetFormatName()] = plugin;
	}

	std::shared_ptr<IFormatPlugin> FileFormatRegistry::GetPlugin(const std::string& formatName) const
	{
		auto it = m_formatPlugins.find(formatName);
		if (it != m_formatPlugins.end())
		{
			return it->second;
		}
		return nullptr;
	}
}
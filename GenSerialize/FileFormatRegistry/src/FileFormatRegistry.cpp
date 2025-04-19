#include <FileFormatRegistry.h>

namespace GenTools::GenSerialize
{
	FileFormatRegistry& FileFormatRegistry::GetInstance()
	{
		static FileFormatRegistry instance;
		return instance;
	}

	void FileFormatRegistry::RegisterPlugin(std::shared_ptr<IFormatPlugin> plugin, uint8_t priority)
	{
		const auto& name = plugin->GetFormatName();
		auto it = m_formatPlugins.find(name);
		if (it == m_formatPlugins.end() || priority >= it->second.priority)
		{
			m_formatPlugins[name] = { plugin, priority };
		}
	}

	std::shared_ptr<IFormatPlugin> FileFormatRegistry::GetPlugin(const std::string& formatName) const
	{
		auto it = m_formatPlugins.find(formatName);
		if (it != m_formatPlugins.end())
		{
			return it->second.plugin;
		}
		return nullptr;
	}
}
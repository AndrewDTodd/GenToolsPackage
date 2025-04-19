#ifndef GENTOOLS_GENSERIALIZE_FILE_FORMAT_REGISTRY_H
#define GENTOOLS_GENSERIALIZE_FILE_FORMAT_REGISTRY_H

#include <string>
#include <unordered_map>
#include <memory>
#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	/// <summary>
	/// Singleton that contains a registry of file format plugins for serialization logic generation
	/// </summary>
	class FileFormatRegistry
	{
	private:
		/// <summary>
		/// Simple structure that pairs a plugin instance to a priority for plugin overriding in the registry
		/// </summary>
		struct PluginInfo
		{
			std::shared_ptr<IFormatPlugin> plugin;
			uint8_t priority;
		};

		std::unordered_map<std::string, PluginInfo> m_formatPlugins;

		FileFormatRegistry() = default;

	public:
		static FileFormatRegistry& GetInstance();

		void RegisterPlugin(std::shared_ptr<IFormatPlugin> plugin, uint8_t priority = 0);

		std::shared_ptr<IFormatPlugin> GetPlugin(const std::string& formatName) const;
	};

	class PluginRegistrar
	{
	public:
		PluginRegistrar(std::shared_ptr<IFormatPlugin> plugin, uint8_t priority = 0)
		{
			FileFormatRegistry::GetInstance().RegisterPlugin(plugin, priority);
		}
	};

#define REGISTER_STATIC_PLUGIN(PluginClass, Priority) \
	static PluginRegistrar registrar_##PluginClass(std::make_shared<PluginClass>(), Priority)
}

#endif // !GENTOOLS_GENSERIALIZE_FILE_FORMAT_REGISTRY_H

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
		std::unordered_map<std::string, std::shared_ptr<IFormatPlugin>> m_formatPlugins;

		FileFormatRegistry() = default;

	public:
		static FileFormatRegistry& GetInstance();

		void RegisterPlugin(std::shared_ptr<IFormatPlugin> plugin);

		std::shared_ptr<IFormatPlugin> GetPlugin(const std::string& formatName) const;
	};

	class PluginRegistrar
	{
	public:
		PluginRegistrar(std::shared_ptr<IFormatPlugin> plugin)
		{
			FileFormatRegistry::GetInstance().RegisterPlugin(plugin);
		}
	};

#define REGISTER_STATIC_PLUGIN(PluginClass) \
	static PluginRegistrar registrar_##PluginClass(std::make_shared<PluginClass>())
}

#endif // !GENTOOLS_GENSERIALIZE_FILE_FORMAT_REGISTRY_H

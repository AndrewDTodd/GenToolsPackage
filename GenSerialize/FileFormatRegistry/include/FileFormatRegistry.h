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

		void RegisterPlugin(const std::string& formatName, std::shared_ptr<IFormatPlugin> plugin);

		std::shared_ptr<IFormatPlugin> GetPlugin(const std::string& formatName) const;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_FILE_FORMAT_REGISTRY_H

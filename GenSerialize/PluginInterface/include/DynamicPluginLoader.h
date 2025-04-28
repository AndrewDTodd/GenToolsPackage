#ifndef GENTOOLS_GENSERIALIZE_DYNAMIC_PLUGIN_LOADER_H
#define GENTOOLS_GENSERIALIZE_DYNAMIC_PLUGIN_LOADER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <iostream>

#include <IFormatPlugin.h>
#include <FileFormatRegistry.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
using LibraryHandle = HMODULE;
#else
#include <dlfcn.h>
using LibraryHandle = void*;
#endif

namespace GenTools::GenSerialize
{
	class DynamicPluginLoader
	{
	private:
		struct PluginHandle
		{
			std::string path;
			LibraryHandle handle = nullptr;
			IFormatPlugin* instance = nullptr;
		};

		std::vector<PluginHandle> m_loadedPlugins;

		LibraryHandle LoadSharedLibrary(const std::string& path);
		void UnloadSharedLibrary(LibraryHandle handle);
		IFormatPlugin* GetPluginInstance(LibraryHandle handle);

	public:
		DynamicPluginLoader() = default;
		~DynamicPluginLoader();

		void LoadPluginsFromDirectory(const std::string& path);
		bool LoadPluginFromFile(const std::string& path);
	};
}

#endif // !GENTOOLS_GENSERIALIZE_DYNAMIC_PLUGIN_LOADER_H

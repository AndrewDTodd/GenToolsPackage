#include <DynamicPluginLoader.h>

#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
	DynamicPluginLoader::~DynamicPluginLoader()
	{
		for (auto& plugin : m_loadedPlugins)
		{
			if (plugin.handle)
				UnloadSharedLibrary(plugin.handle);
		}
	}

	void DynamicPluginLoader::LoadPluginsFromDirectory(const std::string& path)
	{
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_regular_file())
			{
				const auto& filePath = entry.path();
				std::string extension = filePath.extension().string();
#if defined(_WIN32)
				if(extension == ".dll")
#else
				if (extension == ".so" || extension == ".dylib")
#endif
				{
					LoadPluginFromFile(filePath.string());
				}
			}
		}
	}

	bool DynamicPluginLoader::LoadPluginFromFile(const std::string& path)
	{
		auto handle = LoadSharedLibrary(path);
		if (!handle)
		{
			TERMINAL::PRINT_WARNING("Failed to load plugin from file: " + path);
			return false;
		}

		IFormatPlugin* pluginInstance = GetPluginInstance(handle);
		if (!pluginInstance)
		{
			TERMINAL::PRINT_WARNING("Failed to locate CreatePlugin in: " + path);
			UnloadSharedLibrary(handle);
			return false;
		}

		auto shared = std::shared_ptr<IFormatPlugin>(pluginInstance, [](IFormatPlugin*) {
			// Do nothing, lifetime managed statically inside the plugin
			});

		FileFormatRegistry::GetInstance().RegisterPlugin(shared, shared->GetPluginPriority());
		m_loadedPlugins.push_back({ path, handle, pluginInstance });

		return true;
	}

	LibraryHandle DynamicPluginLoader::LoadSharedLibrary(const std::string& path)
	{
#if defined(_WIN32)
		return LoadLibrary(path.c_str());
#else
		return dlopen(path.c_str(), RTLD_NOW);
#endif
	}

	void DynamicPluginLoader::UnloadSharedLibrary(LibraryHandle handle)
	{
#if defined(_WIN32)
		FreeLibrary(handle);
#else
		dlclose(handle);
#endif
	}

	IFormatPlugin* DynamicPluginLoader::GetPluginInstance(LibraryHandle handle)
	{
#if defined(_WIN32)
		FARPROC func = GetProcAddress(handle, "CreatePlugin");
#else
		void* func = dlsym(handle, "CreatePlugin");
#endif
		if (!func) return nullptr;

		using CreatePluginFunc = IFormatPlugin* (*)();
		return reinterpret_cast<CreatePluginFunc>(func)();
	}
}
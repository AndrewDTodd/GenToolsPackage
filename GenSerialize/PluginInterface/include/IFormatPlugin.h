#ifndef GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H
#define GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H

#include <SAST.h>

#if defined(_WIN32)
#if defined(FORMAT_PLUGIN_EXPORTS)
#define FORMAT_PLUGIN_ABI __declspec(dllexport)
#else
#define FORMAT_PLUGIN_ABI __declspec(dllimport)
#endif

#define FORMAT_PLUGIN_CALL __cdecl
#elif defined(__linux__) || defined(__APPLE__)
#if defined(FORMAT_PLUGIN_EXPORTS)
#define FORMAT_PLUGIN_ABI __attribute__((visibility("default")))
#else
#define FORMAT_PLUGIN_ABI __attribute__((visibility("default")))
#endif

#define FORMAT_PLUGIN_CALL __cdecl
#endif

#define DECLARE_FORMAT_PLUGIN(classname)											\
	extern "C" FORMAT_PLUGIN_ABI IFormatPlugin* CreatePlugin() {					\
		return std::make_shared<classname>().get();									\
	}

namespace GenTools::GenSerialize
{
	/// <summary>
	/// Interface for plugins that generate serialization code in a specific format
	/// </summary>
	class FORMAT_PLUGIN_ABI IFormatPlugin
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual FORMAT_PLUGIN_CALL ~IFormatPlugin() = default;
		
		/// <summary>
		/// Generate serialization code for the given SAST node
		/// </summary>
		/// <param name="sastNode">The SAST node to generate code for</param>
		/// <returns>The generated code</returns>
		virtual std::string FORMAT_PLUGIN_CALL GenerateCode(const std::shared_ptr<SASTNode> sastNode) = 0;

		/// <summary>
		/// Get the name of the format (for instance JSON)
		/// </summary>
		/// <returns>Name of the format this plugin hangles</returns>
		virtual std::string FORMAT_PLUGIN_CALL GetFormatName() const noexcept = 0;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H

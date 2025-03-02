#ifndef GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H
#define GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H

#include <string>
#include <SAST.h>

namespace GenTools::GenSerialize
{
	/// <summary>
	/// Interface for plugins that generate serialization code in a specific format
	/// </summary>
	class IFormatPlugin
	{
	public:
		/// <summary>
		/// Virtual destructor
		/// </summary>
		virtual ~IFormatPlugin() = default;
		/// <summary>
		/// Generate serialization code for the given SAST node
		/// </summary>
		/// <param name="sastNode">The SAST node to generate code for</param>
		/// <returns>The generated code</returns>
		virtual std::string GenerateCode(const std::shared_ptr<SASTNode> sastNode) = 0;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_FORMAT_PLUGIN_INTERFACE_H

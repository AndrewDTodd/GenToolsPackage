#include <ASTParser.h>
#include <FileValidator.h>
#include <CodeGenerator.h>
#include <GeneratedFileManager.h>
#include <SAST.h>

#include <PlatformInterface.h>

#include <CmdLineParser.h>
#include <ListFlag.h>

#include <filesystem>
#include <vector>
#include <memory>
#include <iostream>

using namespace GenTools;
using namespace GenTools::GenParse;

int main(int argc, const char** argv)
{
	try
	{
		// Create and get pointer to the CmdLineParser instance
		auto* parser = CmdLineParser<>::GetInstance();

		// Initialize the parser with program details
		parser->Initialize("GenSerialize", "Generates serialization code for types marked with SERIALIZABLE macros in source files from paths provided to program");

		// Define and set flags for the parser instance
		parser->SetSubFlags(
			ListFlag<Arg_String>({ "f", "file_paths" }, "List of file paths to source code to process", true, true)
		);

		std::vector<std::string_view> args{ argv, argv + argc };
		std::vector<std::string_view>::const_iterator itr = args.begin();
		itr++;

		parser->Raise(itr, args.end());

		auto filePathsOpt = parser->GetFlagValue<std::vector<std::string>>("file_paths");

		if (filePathsOpt)
		{

		}
	}
	catch (const std::exception& ex)
	{
		TERMINAL::PRINT_ERROR(ex.what());
	}
}
#include <SASTGeneratorActionFactory.h>
#include <SASTLinker.h>
#include <FileValidator.h>
#include <CodeGenerator.h>
#include <GeneratedFileManager.h>
#include <DynamicPluginLoader.h>

#include <PlatformInterface.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <algorithm>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>

#include <JSONFormatPlugin.h>

using namespace GenTools;
using namespace GenTools::GenSerialize;

using namespace llvm;
using namespace llvm::vfs;
using namespace clang::tooling;

static llvm::cl::OptionCategory AllCategories("GenSerialize command line options");

static llvm::cl::opt<unsigned>
ParseThreads("parse_threads",
	llvm::cl::desc("Number of parallel parse threads to use. 0 to use hardware concurrency level"),
	llvm::cl::init(1), llvm::cl::cat(AllCategories));

static llvm::cl::opt<unsigned>
GenThreads("gen_threads",
	llvm::cl::desc("Number of parallel code generation threads to use. 0 to use hardware concurrency level"),
	llvm::cl::init(0), llvm::cl::cat(AllCategories));

static llvm::cl::opt<std::string>
PluginDirectory("plugin_dir",
	llvm::cl::desc("Directory to scan for format plugins to be loaded dynamically"),
	llvm::cl::init(""), llvm::cl::cat(AllCategories));

static llvm::cl::list<std::string>
PluginFiles("plugin",
	llvm::cl::desc("Explicit path(s) to format plugins to be loaded dynamically"),
	llvm::cl::ZeroOrMore, llvm::cl::cat(AllCategories));

static llvm::cl::list<std::string>
IncludePaths("include_path",
	llvm::cl::desc("Additional include paths to pass to Clang"),
	llvm::cl::ZeroOrMore, llvm::cl::cat(AllCategories));

static llvm::cl::list<std::string> SourceFiles(
	llvm::cl::Positional,
	llvm::cl::desc("<source files>..."),
	llvm::cl::OneOrMore,
	llvm::cl::cat(AllCategories)
);

REGISTER_STATIC_PLUGIN(JSONFormatPlugin, 0);

int main(int argc, const char** argv)
{
	try
	{
		// Used to set virtual path for virtual files
		const std::string virtualIncludeDir = "/__virtual_includes"; // must be absolute or look like it

		// Compiler args for ClangTool
		std::vector<std::string> compilationArgs = {
			"-xc++",                            // Treat all input as C++
			"-std=c++20",                       // Use C++20
			"-fsyntax-only",                    // Don't generate code, just parse
			"-Wno-pragma-once-outside-header",  // Silence warnings for #pragma once
			"-nostdinc++",                      // Skip system C++ headers (for speed/stability)
			"-fno-exceptions",                  // Optional: disable exceptions
			"-fno-rtti",                        // Optional: disable RTTI
			"-I" + virtualIncludeDir,			// Include the virtual include directory
		};
		// Add include paths
		for (const auto& path : IncludePaths)
		{
			compilationArgs.push_back("-I" + path);
		}

		auto Compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", compilationArgs);

		llvm::cl::HideUnrelatedOptions(AllCategories);
		llvm::cl::ParseCommandLineOptions(argc, argv, "GenSerialize - Generate serialization code for C++ classes\n");

		if (SourceFiles.empty())
		{
			TERMINAL::PRINT_ERROR("No source files provided. Use --help for usage information.");
			return 1;
		}

		const auto& sourcePaths = SourceFiles;

		// --------------------------------------------------------------------------
		// 1. Create a shared virtual file system overlay
		// --------------------------------------------------------------------------
		// Get the real file system.
		llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> RealFS = llvm::vfs::getRealFileSystem();
		// Create an in-memory file system for virtual file mappings.
		llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemFS =
			llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();

		// Map "SerializationMacros.h" to an empty file.
		auto EmptyBuffer = llvm::MemoryBuffer::getMemBuffer("", "EmptyBuffer");
		InMemFS->addFile(virtualIncludeDir + "/SerializationMacros.h", /*ModificationTime=*/0, std::move(EmptyBuffer));

		// For each source file, map its corresponding .generated.h file to an empty file.
		for (const auto& sourcePath : sourcePaths)
		{
			std::filesystem::path path(sourcePath);

			// Relative path inside the virtual include dir
			std::filesystem::path virtualHeaderPath = virtualIncludeDir / path.filename();

			// Map the source file into InMemFS
			auto buffer = llvm::MemoryBuffer::getFileAsStream(sourcePath);
			if (std::error_code ec = buffer.getError())
			{
				TERMINAL::PRINT_WARNING("Warning: Failed to read " + sourcePath);
				continue;
			}

			InMemFS->addFile(virtualHeaderPath.string(), 0, std::move(*buffer));

			// Also map the associated .generated.h file to an empty buffer
			auto emptyGenHeader = llvm::MemoryBuffer::getMemBuffer("", "EmptyGeneratedHeader");
			std::filesystem::path virtualGenPath = virtualIncludeDir / path.filename().replace_extension(".generated.h");
			InMemFS->addFile(virtualGenPath.string(), 0, std::move(emptyGenHeader));
		}

		// Create an overlay that first consults the in-memory FS, then the real FS.
		auto OverlayFS = llvm::makeIntrusiveRefCnt<llvm::vfs::OverlayFileSystem>(InMemFS);
		OverlayFS->pushOverlay(RealFS);

		clang::FileSystemOptions fsOpts;
		auto FileMgr = llvm::makeIntrusiveRefCnt<clang::FileManager>(fsOpts, OverlayFS);

		// Global storage for per-file SAST trees
		std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
		std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;
		std::mutex globalsMutex;

		DynamicPluginLoader pluginLoader;

		// 1. Load plugins from directory, if provided
		if (!PluginDirectory.empty())
		{
			TERMINAL::GREEN_TEXT();
			std::cout << "[INFO] Scanning plugin directory: " << PluginDirectory << std::endl;
			TERMINAL::DEFAULT_TEXT();

			pluginLoader.LoadPluginsFromDirectory(PluginDirectory);
		}

		// 2. Load plugins from explicit paths, if provided
		for (const auto& pluginPath : PluginFiles)
		{
			TERMINAL::GREEN_TEXT();
			std::cout << "[INFO] Loading plugin from file: " << pluginPath << std::endl;
			TERMINAL::DEFAULT_TEXT();

			if (pluginLoader.LoadPluginFromFile(pluginPath))
			{
				TERMINAL::GREEN_TEXT_BRIGHT();
				std::cout << "\t>> Success" << std::endl;
				TERMINAL::DEFAULT_TEXT();
			}
		}

		// Check the ParseThreads and GenThreads config. If 0 set to hardware concurrency level
		if (ParseThreads == 0)
		{
			ParseThreads = std::thread::hardware_concurrency();
			if (ParseThreads == 0) // Fallback safety
				ParseThreads = 1;
		}

		// Step 1: Parallel Parsing
		std::vector<std::thread> parseThreads;
		size_t chunkSize = (sourcePaths.size() + ParseThreads - 1) / ParseThreads;
		for (size_t i = 0; i < sourcePaths.size(); i += chunkSize)
		{
			std::vector<std::string> fileChunk(sourcePaths.begin() + i, sourcePaths.begin() + std::min(i + chunkSize, sourcePaths.size()));

			parseThreads.emplace_back([&, fileChunk = std::move(fileChunk)]() {	
				SASTGeneratorActionFactory factory;
				ClangTool tool(*Compilations, fileChunk, std::make_shared<clang::PCHContainerOperations>(), OverlayFS, FileMgr);

				int result = tool.run(&factory);
				if (result)
					TERMINAL::PRINT_WARNING_S("Error while processing a file chunk");

				// Merge per-file SASTs into global storage
				std::lock_guard<std::mutex> lock(globalsMutex);
				factory.MergeResults(globalSASTTrees, globalSASTMap);
			});
		}

		// Wait for all parse threads to finish
		for (auto& t : parseThreads)
		{
			t.join();
		}

		if (GenThreads == 0)
		{
			GenThreads = std::thread::hardware_concurrency();
			if (GenThreads == 0) // Fallback safety
				GenThreads = 1;
		}

		// Step 2: Parallel Code Generation
		std::vector<std::thread> codeGenThreads;
		chunkSize = (globalSASTTrees.size() + GenThreads - 1) / GenThreads;
		auto iterator = globalSASTTrees.begin();
		for (size_t i = 0; i < globalSASTTrees.size(); i += chunkSize)
		{
			std::vector<std::pair<std::string, std::vector<std::shared_ptr<SASTNode>>>> localChunk;

			for (size_t j = 0; j < chunkSize && iterator != globalSASTTrees.end(); j++, iterator++)
			{
				localChunk.emplace_back(*iterator);
			}

			codeGenThreads.emplace_back([&, localChunk = std::move(localChunk)]() {
				for (const auto& [filePath, SASTTree] : localChunk)
				{
					if (SASTTree.empty())
						continue;

					CodeGenerator codeGen(SASTTree);
					GeneratedCode generatedCode = codeGen.GenerateCode();

					GeneratedFileManager fileManager(std::filesystem::path{filePath});
					if (!fileManager.UpdateGeneratedFile(generatedCode))
					{
						TERMINAL::PRINT_ERROR_S("ERROR: Failed to update generated header for " + filePath);
					}
				}
			});
		}

		for (auto& t : codeGenThreads)
		{
			t.join();
		}

		return 0;
	}
	catch (const std::exception& ex)
	{
		TERMINAL::PRINT_ERROR(ex.what());

		return 1;
	}
}
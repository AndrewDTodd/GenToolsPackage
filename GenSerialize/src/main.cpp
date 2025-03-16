#include <SASTGeneratorActionFactory.h>
#include <SASTLinker.h>
#include <FileValidator.h>
#include <CodeGenerator.h>
#include <GeneratedFileManager.h>

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

using namespace GenTools;
//using namespace GenTools::GenParse;
using namespace GenTools::GenSerialize;

using namespace llvm;
using namespace llvm::vfs;
using namespace clang::tooling;

static llvm::cl::OptionCategory myToolCategory("GenSerialize options");

static llvm::cl::opt<unsigned>
ParseThreads("parse_threads",
	llvm::cl::desc("Number of parallel parse threads to use. 0 to use hardware concurrency level"),
	llvm::cl::init(0), llvm::cl::cat(myToolCategory));

static llvm::cl::opt<unsigned>
LinkThreads("link_threads",
	llvm::cl::desc("Number of parallel link threads to use. 0 to use hardware concurrency level"),
	llvm::cl::init(0), llvm::cl::cat(myToolCategory));

static llvm::cl::opt<unsigned>
GenThreads("gen_threads",
	llvm::cl::desc("Number of parallel code generation threads to use. 0 to use hardware concurrency level"),
	llvm::cl::init(0), llvm::cl::cat(myToolCategory));

static llvm::cl::list<std::string>
FilePaths("file_paths",
	llvm::cl::desc("List of file paths to source code for processing"),
	llvm::cl::OneOrMore, llvm::cl::cat(myToolCategory));

int main(int argc, const char** argv)
{
	try
	{
		/*
		// Create and get pointer to the CmdLineParser instance
		auto* parser = CmdLineParser<>::GetInstance();

		// Initialize the parser with program details
		parser->Initialize("GenSerialize", "Generates serialization code for types marked with SERIALIZABLE macros in source files from paths provided to the program");

		// Define and set flags for the parser instance
		parser->SetSubFlags(
			Flag({ "p", "parse_threads" }, "Number of parallel parse threads to use. 0 to use hardware concurrency level", Arg_UInt32(0), true, false, false),
			Flag({"l", "link_threads"}, "Number of parallel link threads to use. 0 to use hardware concurrency level", Arg_UInt32(0), true, false, false),
			Flag({"g", "gen_threads"}, "Number of parallel code generation threads to use. 0 to use hardware concurrency level", Arg_UInt32(0), true, false, false),
			ListFlag<Arg_String>({ "f", "file_paths" }, "List of file paths to source code for processing", true, true)
		);

		std::vector<std::string_view> args{argv, argv + argc};
		std::vector<std::string_view>::const_iterator itr = args.begin();
		itr++;

		parser->Raise(itr, args.end());

		std::vector<std::string> filePaths = parser->GetFlagValue<std::vector<std::string>>("file_paths").value();
		uint32_t parallelParse = parser->GetFlagValue<uint32_t>("parse_threads").value();
		uint32_t parallelLink = parser->GetFlagValue<uint32_t>("link_threads").value();
		uint32_t parallelGen = parser->GetFlagValue<uint32_t>("gen_threads").value();

		uint32_t numCores = std::thread::hardware_concurrency();
		if (numCores == 0) numCores = 4;
		if (parallelParse == 0) parallelParse = numCores;
		if (parallelLink == 0) parallelLink = numCores;
		*/

		auto optionsParser = CommonOptionsParser::create(argc, argv, myToolCategory);
		if (!optionsParser)
		{
			TERMINAL::PRINT_ERROR("ERROR: Failed to parse options");
			return 1;
		}

		const auto& sourcePaths = optionsParser->getSourcePathList();

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
		InMemFS->addFile("SerializationMacros.h", /*ModificationTime=*/0, std::move(EmptyBuffer));

		// For each source file, map its corresponding .generated.h file to an empty file.
		for (const auto& sourcePath : sourcePaths) {
			std::filesystem::path path(sourcePath);
			std::string virtualPath = path.filename().replace_extension(".generated.h").string();
			auto EmptyBufferGen = llvm::MemoryBuffer::getMemBuffer("", "EmptyBufferGen");
			InMemFS->addFile(virtualPath, 0, std::move(EmptyBufferGen));
		}

		// Create an overlay that first consults the in-memory FS, then the real FS.
		auto OverlayFS = llvm::makeIntrusiveRefCnt<llvm::vfs::OverlayFileSystem>(InMemFS);
		OverlayFS->pushOverlay(RealFS);

		// Global storage for per-file SAST trees
		std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
		std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;
		std::mutex globalsMutex;

		// Step 1: Parallel Parsing
		std::vector<std::thread> parseThreads;
		size_t chunkSize = (sourcePaths.size() + ParseThreads - 1) / ParseThreads;
		for (size_t i = 0; i < sourcePaths.size(); i += chunkSize)
		{
			std::vector<std::string> fileChunk(sourcePaths.begin() + i, sourcePaths.begin() + std::min(i + chunkSize, sourcePaths.size()));

			parseThreads.emplace_back([&, fileChunk = std::move(fileChunk)]() {
				SASTGeneratorActionFactory factory;
				ClangTool tool(optionsParser->getCompilations(), fileChunk, std::make_shared<clang::PCHContainerOperations>(), OverlayFS);

				int result = tool.run(&factory);
				if (result)
					TERMINAL::PRINT_ERROR("Error while processing a file chunk");

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

		// Step 2: Parallel Linking
		//std::vector<std::thread> linkThreads;
		//size_t chunkSize = (globalSASTTrees.size() + parallelLink - 1) / parallelLink;
		//auto iterator = globalSASTTrees.begin();
		//for (size_t i = 0; i < globalSASTTrees.size(); i += chunkSize)
		//{
		//	std::vector<std::pair<std::string, std::vector<std::shared_ptr<SASTNode>>>> localChunk;

		//	for (size_t j = 0; j < chunkSize && iterator != globalSASTTrees.end(); j++, iterator++)
		//	{
		//		localChunk.emplace_back(*iterator);
		//	}

		//	linkThreads.emplace_back([&]() {
		//		for (auto& [filePath, SASTTree] : localChunk)
		//		{
		//			SASTLinker linker(SASTTree, globalSASTMap);
		//			linker.Link();
		//		}
		//	});
		//}

		//// Wait for all link threads to finish
		//for (auto& t : linkThreads)
		//{
		//	t.join();
		//}

		// Step 3: Parallel Code Generation
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
					CodeGenerator codeGen(SASTTree);
					GeneratedCode generatedCode = codeGen.GenerateCode();

					GeneratedFileManager fileManager(std::filesystem::path{filePath});
					if (!fileManager.UpdateGeneratedFile(generatedCode))
					{
						TERMINAL::PRINT_ERROR("ERROR: Failed to update generated header for " + filePath);
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

		return -1;
	}
}
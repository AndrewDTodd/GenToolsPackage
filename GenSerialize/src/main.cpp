#include <SASTGeneratorActionFactory.h>
#include <SASTLinker.h>
#include <FileValidator.h>
#include <CodeGenerator.h>
#include <GeneratedFileManager.h>

#include <PlatformInterface.h>

//#include <CmdLineParser.h>
//#include <ListFlag.h>

#include <iostream>
#include <vector>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <algorithm>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Basic/FileManager.h>

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
		}

		ClangTool tool(optionsParser->getCompilations(), optionsParser->getSourcePathList());

		uint32_t parallelParse = ParseThreads;
		uint32_t parallelLink = LinkThreads;
		uint32_t parallelGen = GenThreads;

		tool.mapVirtualFile("SerializationMacros.h", "");

		// For each source file, add a mapping for its corresponding .generated.h file.
		for (const auto& sourcePath : optionsParser->getSourcePathList())
		{
			std::filesystem::path path(sourcePath);
			tool.mapVirtualFile(path.filename().replace_extension(".generated.h").string(), "");
		}

		// Global storage for per-file SAST trees
		std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
		std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;

		// Step 1: Parallel Parsing
		/*d::vector<std::thread> parseThreads;
		size_t chunkSize = (filePaths.size() + parallelParse - 1) / parallelParse;
		for (size_t i = 0; i < filePaths.size(); i += chunkSize)
		{
			std::vector<std::string> fileBatch(filePaths.begin() + i, filePaths.begin() + std::min(i + chunkSize, filePaths.size()));

			parseThreads.emplace_back([&]() {
				SASTGenerator generator;
				generator.ProcessFiles(fileBatch);

				// Merge per-file SASTs into global storage
				std::lock_guard<std::mutex> lock(mapMutex);
				for (const auto& [filePath, tree] : generator.GetSASTTrees())
				{
					globalSASTTrees[filePath] = tree;
				}
				for (const auto& [name, node] : generator.GetSASTMap())
				{
					globalSASTMap[name] = node;
				}
			});
		}

		// Wait for all parse threads to finish
		for (auto& t : parseThreads)
		{
			t.join();
		}*/
		SASTGeneratorActionFactory factory;
		int result = tool.run(&factory);

		factory.MergeResults(globalSASTTrees, globalSASTMap);

		// Step 2: Parallel Linking
		std::vector<std::thread> linkThreads;
		size_t chunkSize = (globalSASTTrees.size() + parallelLink - 1) / parallelLink;
		auto iterator = globalSASTTrees.begin();
		for (size_t i = 0; i < globalSASTTrees.size(); i += chunkSize)
		{
			std::vector<std::pair<std::string, std::vector<std::shared_ptr<SASTNode>>>> localChunk;

			for (size_t j = 0; j < chunkSize && iterator != globalSASTTrees.end(); j++, iterator++)
			{
				localChunk.emplace_back(*iterator);
			}

			linkThreads.emplace_back([&]() {
				for (auto& [filePath, SASTTree] : localChunk)
				{
					SASTLinker linker(SASTTree, globalSASTMap);
					linker.Link();
				}
			});
		}

		// Wait for all link threads to finish
		for (auto& t : linkThreads)
		{
			t.join();
		}

		// Step 3: Parallel Code Generation
		std::vector<std::thread> codeGenThreads;
		chunkSize = (globalSASTTrees.size() + parallelGen - 1) / parallelGen;
		iterator = globalSASTTrees.begin();
		for (size_t i = 0; i < globalSASTTrees.size(); i += chunkSize)
		{
			std::vector<std::pair<std::string, std::vector<std::shared_ptr<SASTNode>>>> localChunk;

			for (size_t j = 0; j < chunkSize && iterator != globalSASTTrees.end(); j++, iterator++)
			{
				localChunk.emplace_back(*iterator);
			}

			codeGenThreads.emplace_back([&]() {
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
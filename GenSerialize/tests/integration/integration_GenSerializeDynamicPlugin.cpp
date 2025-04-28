#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <memory>
#include <thread>
#include <mutex>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Frontend/ASTUnit.h>

#include <SASTGeneratorActionFactory.h>
#include <GeneratedFileManager.h>
#include <CodeGenerator.h>
#include <DynamicPluginLoader.h>
#include <JSONFormatPlugin.h>

using namespace clang::tooling;
using namespace GenTools::GenSerialize;
namespace fs = std::filesystem;

#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

std::filesystem::path GetExecutableDirectory() {
#if defined(_WIN32)
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path();
#else
	char buffer[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (len != -1) {
		buffer[len] = '\0';
		return std::filesystem::path(buffer).parent_path();
	}
	else {
		throw std::runtime_error("Unable to determine executable path");
	}
#endif
}

struct OutputCapture
{
	std::stringstream outBuffer;
	std::stringstream errBuffer;

	std::streambuf* oldOut = nullptr;
	std::streambuf* oldErr = nullptr;

	void start() {
		oldOut = std::cout.rdbuf(outBuffer.rdbuf());
		oldErr = std::cerr.rdbuf(errBuffer.rdbuf());
	}

	void stop() {
		std::cout.rdbuf(oldOut);
		std::cerr.rdbuf(oldErr);
	}

	std::string getStdOut() const { return outBuffer.str(); }
	std::string getStdErr() const { return errBuffer.str(); }
};

static void AssertCodeEqual(const std::string& actual, const std::string& expected)
{
	std::string normA, normB;
	std::remove_copy(actual.begin(), actual.end(), std::back_inserter(normA), '\r');
	std::remove_copy(expected.begin(), expected.end(), std::back_inserter(normB), '\r');
	EXPECT_EQ(normA, normB);
}

TEST(IntegrationTest, DriverGeneratesHeaderWithDynamicPlugin)
{
	const std::string virtualIncludeDir = "/__virtual_includes"; // must be absolute or look like it

	// Setup temp directory
	fs::path tempDir = fs::temp_directory_path() / "GenSerialize_Integration";
	fs::create_directories(tempDir);

	fs::path headerPath = tempDir / "TestType.h";
	//fs::path pluginDir = fs::current_path() / "plugins"; // Adjust as needed

	// Write a simple serializable class header
	std::ofstream header(headerPath);
	header << R"cpp(#pragma once
#include <SerializationMacros.h>
#include <TestType.generated.h>

class SERIALIZABLE(JSON) TestType {
	SERIALIZE_FIELD
	int value;

	GENERATED_SERIALIZATION_BODY();
};
	)cpp";
	header.close();

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
	auto Compilations = std::make_unique<FixedCompilationDatabase>(".", compilationArgs);

	// --------------------------------------------------------------------------
	// 1. Create a shared virtual file system overlay
	// --------------------------------------------------------------------------
	// Get the real file system.
	auto RealFS = llvm::vfs::getRealFileSystem();
	// Create an in-memory file system for virtual file mappings.
	auto InMemFS = llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();

	// Map "SerializationMacros.h" to an empty file.
	auto EmptyBuffer = llvm::MemoryBuffer::getMemBuffer("", "EmptyBuffer");
	InMemFS->addFile(virtualIncludeDir + "/SerializationMacros.h", 0, std::move(EmptyBuffer));
	auto EmptyBufferGen = llvm::MemoryBuffer::getMemBuffer("", "EmptyBufferGen");
	InMemFS->addFile(virtualIncludeDir + "/TestType.generated.h", 0, std::move(EmptyBufferGen));

	// Create an overlay that first consults the in-memory FS, then the real FS.
	auto OverlayFS = llvm::makeIntrusiveRefCnt<llvm::vfs::OverlayFileSystem>(InMemFS);
	OverlayFS->pushOverlay(RealFS);

	/*We don't need this for this test as ne have statically linked the JSON plugin*/
	// Load dynamic plugins
	std::filesystem::path pluginPath = GetExecutableDirectory();

#if defined(_WIN32)
	pluginPath /= "JSON_FormatPlugin.dll";
#else
	pluginPath /= "JSON_FormatPlugin.so";
#endif

	DynamicPluginLoader loader;
	loader.LoadPluginFromFile(pluginPath.string());

	// Parse header and generate SAST
	std::unordered_map<std::string, std::vector<std::shared_ptr<GenTools::GenSerialize::SASTNode>>> trees;
	std::unordered_map<std::string, std::shared_ptr<GenTools::GenSerialize::SASTNode>> treeMap;

	SASTGeneratorActionFactory factory;
	clang::FileSystemOptions fsOpts;
	auto FileMgr = llvm::makeIntrusiveRefCnt<clang::FileManager>(fsOpts, OverlayFS);

	ClangTool tool(*Compilations, { headerPath.string() },
		std::make_shared<clang::PCHContainerOperations>(), OverlayFS, FileMgr);

	ASSERT_EQ(tool.run(&factory), 0);

	factory.MergeResults(trees, treeMap);

	// Generate code
	for (const auto& [filePath, nodes] : trees) {
		GenTools::GenSerialize::CodeGenerator codeGen(nodes);
		auto generatedCode = codeGen.GenerateCode();
		GenTools::GenSerialize::GeneratedFileManager manager(filePath);
		ASSERT_TRUE(manager.UpdateGeneratedFile(generatedCode));
	}

	// Verify output
	fs::path generatedPath = tempDir / "generated" / "TestType.generated.h";
	ASSERT_TRUE(fs::exists(generatedPath));

	std::ifstream in(generatedPath);
	std::stringstream buffer;
	buffer << in.rdbuf();
	in.close();
	std::string content = buffer.str();

	EXPECT_NE(content.find("JSONSerialize"), std::string::npos);
	EXPECT_NE(content.find("JSONDeserialize"), std::string::npos);

	const char* expected = R"cpp(// Auto-generated serialization code for TestType.h. DO NOT MODIFY MANUALLY

#define GENERATED_SERIALIZATION_BODY() \
/* Format: JSON */ \
#include <fstream> \
 \
#include <JSONStructure.h> \
 \
static void JSONSerialize(JSONObject& jsonReceiver, const TestType& objSource) \
{ \
	jsonReceiver.AddMember("value", JSONNumber(static_cast<double>(objSource.value))); \
 \
} \
 \
static void JSONDeserialize(TestType& objReceiver, const JSONObject& jsonSource) \
{ \
	objReceiver.value = jsonSource.GetMember("value").as<JSONNumber>().value; \
 \
} \
 \
static void JSONSerialize(std::ostream& osReceiver, const TestType& objSource) \
{ \
	JSONStructure jsonRep; \
	jsonRep.AddMember("value", JSONNumber(static_cast<double>(objSource.value))); \
 \
	osReceiver << jsonRep.Stringify(); \
} \
 \
static void JSONDeserialize(TestType& objReceiver, const std::istream& isSource) \
{ \
	JSONStructure jsonRep = JSONStructure::Parse(isSource); \
	objReceiver.value = jsonRep.GetMember("value").as<JSONNumber>().value; \
 \
} \


)cpp";

	AssertCodeEqual(content, expected);

	fs::remove_all(tempDir);
}

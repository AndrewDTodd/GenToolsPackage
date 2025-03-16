#include <gtest/gtest.h>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Frontend/ASTUnit.h>

#include <unordered_map>
#include <memory>
#include <string>

#include <SASTGeneratorActionFactory.h>
#include <SASTGeneratorAction.h>
#include <SAST.h>
#include <JSONFormatPlugin.h>

using namespace GenTools::GenSerialize;
using namespace clang::tooling;

TEST(JSONFormatPluginTests, GenerateCodeForSimpleType)
{
    // Your test code that includes PODType.h, SerializationMacros.h, and ValidHeader.generated.h.
    /*const char* code = R"cpp(
        #pragma once

        #include "PODType.h"
        #include "SerializationMacros.h"
        #include "ValidHeader.generated.h"

        class SERIALIZABLE(JSON, XML) ValidTestType
        {
        private:
            SERIALIZE_FIELD
            POD pod;
        public:
            GENERATED_SERIALIZATION_BODY();
        };
    )cpp";

    // Create a buffer containing the contents of PODType.h.
    std::string podTypeContents = R"cpp(
        #pragma once

        #include <stdint.h>
        #include <string>

        #include "SerializationMacros.h"

        struct SERIALIZABLE_POD POD
        {
            uint16_t one;
            uint32_t two;
            std::string str;
        };
    )cpp";

    clang::tooling::FileContentMappings fileMappings;
    fileMappings.push_back(std::make_pair("PODType.h", podTypeContents));
    fileMappings.push_back(std::make_pair("SerializationMacros.h", ""));
    fileMappings.push_back(std::make_pair("ValidHeader.generated.h", ""));

    // Prepare an external container for our SAST results.
    SASTResult result;

    // Create our test-specific action.
    auto action = std::make_unique<SASTGeneratorAction>(result);

    // Set up any required command-line arguments.
    std::vector<std::string> args = { "-std=c++20", "-fsyntax-only"};

    // Run the tool. Note that we pass the fileMappings as the last parameter.
    bool success = clang::tooling::runToolOnCodeWithArgs(
        std::move(action),
        code,
        args,
        "ValidHeader.hpp",            // FileName
        "GenSerialize",             // ToolName
        nullptr,
        fileMappings                // VirtualMappedFiles
    );*/

    const std::vector<std::string> sourcePaths = { "ValidHeader.h", "PODType.h", "OtherType.h"};

    // --------------------------------------------------------------------------
        // 1. Create a shared virtual file system overlay
        // --------------------------------------------------------------------------
        // Get the real file system.
    //llvm::IntrusiveRefCntPtr<llvm::vfs::FileSystem> RealFS = llvm::vfs::getRealFileSystem();
    //// Create an in-memory file system for virtual file mappings.
    //llvm::IntrusiveRefCntPtr<llvm::vfs::InMemoryFileSystem> InMemFS =
    //    llvm::makeIntrusiveRefCnt<llvm::vfs::InMemoryFileSystem>();

    //// Map "SerializationMacros.h" to an empty file.
    //auto SerializationMacros_H = llvm::MemoryBuffer::getMemBuffer("", "SerializationMacros.h Buffer");
    //bool success = InMemFS->addFile("SerializationMacros.h", /*ModificationTime=*/0, std::move(SerializationMacros_H));
    //assert(success && "Failed to add SerializationMacros.h to InMemFS!");

    //auto ValidHeaderGen_H = llvm::MemoryBuffer::getMemBuffer("", "ValidHeader.generated.h Buffer");
    //success = InMemFS->addFile("ValidHeader.generated.h", /*ModificationTime=*/0, std::move(ValidHeaderGen_H));
    //assert(success && "Failed to add SerializationMacros.h to InMemFS!");

    //auto ValidHeader_H = llvm::MemoryBuffer::getMemBuffer(R"cpp(
    //    #pragma once
    //    
    //    #include <vector>
    //    #include <string>
    //    #include <unordered_map>

    //    #include "OtherType.h"
    //    #include "PODType.h"
    //    #include "SerializationMacros.h"
    //    #include "ValidHeader.generated.h"

    //    class SERIALIZABLE(JSON, XML) ValidTestType
    //    {
    //    private:
    //        SERIALIZE_FIELD
    //        std::vector<POD> podVec;

    //        SERIALIZE_FIELD
    //        std::unordered_map<std::string, OtherType> otherMap;
    //    public:
    //        GENERATED_SERIALIZATION_BODY();
    //    };
    //)cpp", 
    //"ValidHeader_H Buffer");
    //success = InMemFS->addFile("ValidHeader.h", /*ModificationTime=*/0, std::move(ValidHeader_H));
    //assert(success && "Failed to add SerializationMacros.h to InMemFS!");

    //auto POD_H = llvm::MemoryBuffer::getMemBuffer(R"cpp(
    //    #pragma once

    //    #include <stdint.h>
    //    #include <string>

    //    #include "SerializationMacros.h"

    //    struct SERIALIZABLE_POD POD
    //    {
    //        uint16_t one;
    //        uint32_t two;
    //        std::string str;
    //    };
    //)cpp", 
    //"POD_H Buffer");
    //success = InMemFS->addFile("PODType.h", /*ModificationTime=*/0, std::move(POD_H));
    //assert(success && "Failed to add SerializationMacros.h to InMemFS!");

    //auto OtherType_H = llvm::MemoryBuffer::getMemBuffer(R"cpp(
    //    #pragma once

    //    #include <stdint.h>
    //    #include <string>

    //    #include "SerializationMacros.h"

    //    class SERIALIZABLE(JSON) OtherType
    //    {
    //    private:
    //        SERIALIZE_FIELD
    //        uint32_t otherField;

    //        SERIALIZE_FIELD
    //        std::string otherString;

    //   public:
    //        GENERATED_SERIALIZATION_BODY();
    //    };
    //)cpp",
    //    "OtherType_H Buffer");
    //success = InMemFS->addFile("OtherType.h", /*ModificationTime=*/0, std::move(OtherType_H));
    //assert(success && "Failed to add SerializationMacros.h to InMemFS!");

    //// Create an overlay that first consults the in-memory FS, then the real FS.
    //auto OverlayFS = llvm::makeIntrusiveRefCnt<llvm::vfs::OverlayFileSystem>(InMemFS);
    //OverlayFS->pushOverlay(RealFS);

    // Global storage for per-file SAST trees
    std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
    std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;
    
    std::vector<std::string> compilationArgs = {
        "-xc++",
        "-std=c++20",  // Use C++20 standard
        "-fsyntax-only",
    };

    std::unique_ptr<clang::tooling::CompilationDatabase> Compilations =
        std::make_unique<clang::tooling::FixedCompilationDatabase>(".", compilationArgs);

    SASTGeneratorActionFactory factory;
    ClangTool tool(*Compilations, sourcePaths);

	tool.mapVirtualFile("SerializationMacros.h", "");
	tool.mapVirtualFile("ValidHeader.generated.h", "");
    tool.mapVirtualFile("ValidHeader.h", R"cpp(
        #pragma once
        
        #include <vector>
        #include <string>
        #include <unordered_map>

        #include "OtherType.h"
        #include "PODType.h"
        #include "SerializationMacros.h"
        #include "ValidHeader.generated.h"

        class SERIALIZABLE(JSON, XML) ValidTestType
        {
        private:
            SERIALIZE_FIELD
            std::vector<POD> podVec;

            SERIALIZE_FIELD
            std::unordered_map<std::string, OtherType> otherMap;
        public:
            GENERATED_SERIALIZATION_BODY();
        };
    )cpp");
    tool.mapVirtualFile("PODType.h", R"cpp(
        #pragma once

        #include <stdint.h>
        #include <string>

        #include "SerializationMacros.h"

        struct SERIALIZABLE_POD POD
        {
            uint16_t one;
            uint32_t two;
            std::string str;
        };
    )cpp");
    tool.mapVirtualFile("OtherType.h", R"cpp(
        #pragma once

        #include <stdint.h>
        #include <string>

        #include "SerializationMacros.h"

        class SERIALIZABLE(JSON) OtherType
        {
        private:
            SERIALIZE_FIELD
            uint32_t otherField;

            SERIALIZE_FIELD
            std::string otherString;

       public:
            GENERATED_SERIALIZATION_BODY();
        };
    )cpp");

    int result = tool.run(&factory);
    ASSERT_FALSE(result) << "Clang tool run failed";

    // Merge per-file SASTs into global storage
    factory.MergeResults(globalSASTTrees, globalSASTMap);

    // Now check that our SAST map contains an entry for "ValidTestType".
    ASSERT_NE(globalSASTMap.find("ValidTestType"), globalSASTMap.end())
        << "ValidTestType was not detected in the SAST map";

    JSONFormatPlugin plugin;
    auto ValidTypeGen = plugin.GenerateCode(globalSASTMap["ValidTestType"]);

	auto OtherTypeGen = plugin.GenerateCode(globalSASTMap["OtherType"]);
}
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
    const std::vector<std::string> sourcePaths = { "ValidHeader.h", "PODType.h", "OtherType.h"};

    // Global storage for per-file SAST trees
    std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
    std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;
    
    std::vector<std::string> compilationArgs = {
        "-xc++",
        "-std=c++20",  // Use C++20 standard
        "-fsyntax-only",
        "-fmodules",
        "-fcxx-modules",
        "-fimplicit-modules",
        "-fimplicit-module-maps",
        "-fmodules-cache-path=./clang-modules-cache",
        "-nostdinc++",
        "-fno-use-cxa-atexit",
        "-fno-exceptions",
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
            std::unordered_map<uint32_t, OtherType> otherMap;
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
#include <gtest/gtest.h>

#include <clang/Tooling/Tooling.h>
#include <llvm/Support/MemoryBuffer.h>

#include <SASTGeneratorAction.h>
#include <SAST.h>

#include <unordered_map>
#include <memory>
#include <string>

using namespace GenTools::GenSerialize;
using namespace clang::tooling;

TEST(SASTToolingTests, DetectsSimpleSerializableClass)
{
    // Your test code that includes PODType.h, SerializationMacros.h, and ValidHeader.generated.h.
    const char* code = R"cpp(
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
    std::vector<std::string> args = { "-std=c++20" };

    // Run the tool. Note that we pass the fileMappings as the last parameter.
    bool success = clang::tooling::runToolOnCodeWithArgs(
        std::move(action),
        code,
        args,
        "ValidHeader.hpp",            // FileName
        "GenSerialize",             // ToolName
        std::make_shared<clang::PCHContainerOperations>(),
        fileMappings                // VirtualMappedFiles
    );
    EXPECT_TRUE(success) << "Clang tool run failed";

    // Now check that our SAST map contains an entry for "ValidTestType".
    EXPECT_NE(result.SASTMap.find("ValidTestType"), result.SASTMap.end())
        << "ValidTestType was not detected in the SAST map";
}
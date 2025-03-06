#include <gtest/gtest.h>

#include <SASTGeneratorActionFactory.h>
#include <SAST.h>

#include <clang/Tooling/Tooling.h>

#include <vector>
#include <unordered_map>

using namespace GenTools::GenSerialize;
using namespace clang::tooling;

TEST(SASTGeneratorFactoryTests, MergesMultipleFiles)
{
    // Two code snippets, each with a serializable type.
    const char* code1 = R"cpp(
        class SERIALIZABLE(XML) ClassOne {
        public:
        SERIALIZE_FIELD
            int a;
        };
    )cpp";

    const char* code2 = R"cpp(
        class SERIALIZABLE(XML, JSON) ClassTwo {
        public:
        SERIALIZE_FIELD
            float b;
        };
    )cpp";

    // Create the custom factory.
    SASTGeneratorActionFactory factory;

    // Run each snippet through the tool.
    bool success1 = runToolOnCode(factory.create(), code1, "file1.cpp");
    bool success2 = runToolOnCode(factory.create(), code2, "file2.cpp");

    EXPECT_TRUE(success1);
    EXPECT_TRUE(success2);

    // Merge local results.
    std::unordered_map<std::string, std::vector<std::shared_ptr<SASTNode>>> globalSASTTrees;
    std::unordered_map<std::string, std::shared_ptr<SASTNode>> globalSASTMap;
    factory.MergeResults(globalSASTTrees, globalSASTMap);

    // Check that both ClassOne and ClassTwo are present.
    EXPECT_NE(globalSASTMap.find("ClassOne"), globalSASTMap.end());
    EXPECT_NE(globalSASTMap.find("ClassTwo"), globalSASTMap.end());
}
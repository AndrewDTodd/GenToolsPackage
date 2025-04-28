#include <SASTGeneratorAction.h>

#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>

#include <filesystem>

#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
    SASTGeneratorAction::SASTGeneratorAction(SASTResult& result)
        : m_result(result)
    {}

    std::unique_ptr<clang::ASTConsumer> SASTGeneratorAction::CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef inFile)
    {
        m_result.filePath = inFile.str();

        return std::make_unique<ASTParser>(m_result);
    }

    bool SASTGeneratorAction::BeginInvocation(clang::CompilerInstance& CI)
    {
        auto& ppOpts = CI.getPreprocessorOpts();
        ppOpts.addMacroDef("SERIALIZABLE(...)=__attribute__((annotate(\"serializable:\" #__VA_ARGS__)))");
        ppOpts.addMacroDef("SERIALIZABLE_ALL(...)=__attribute__((annotate(\"serializable:all\")))");
        ppOpts.addMacroDef("SERIALIZABLE_PUBLIC(...)=__attribute__((annotate(\"serializable:public\")))");
        ppOpts.addMacroDef("SERIALIZABLE_PROTECTED(...)=__attribute__((annotate(\"serializable:protected\")))");
        ppOpts.addMacroDef("SERIALIZABLE_PRIVATE(...)=__attribute__((annotate(\"serializable:private\")))");
        ppOpts.addMacroDef("SERIALIZABLE_POD=__attribute__((annotate(\"serializable:pod\")))");
        ppOpts.addMacroDef("SERIALIZE_FIELD=__attribute__((annotate(\"serialize\")))");
        ppOpts.addMacroDef("SERIALIZE_FIELD_AS(name)=__attribute__((annotate(\"serialize:\" #name)))");
        ppOpts.addMacroDef("STATIC_ARRAY=__attribute__((annotate(\"static_array\")))");
        ppOpts.addMacroDef("DYNAMIC_ARRAY(lengthVar)=__attribute__((annotate(\"dynamic_array:\" #lengthVar)))");
        ppOpts.addMacroDef("SERIALIZE_EXCLUDE=__attribute__((annotate(\"serialize:exclude\")))");
        ppOpts.addMacroDef("GENERATED_SERIALIZATION_BODY()=");
        return clang::ASTFrontendAction::BeginInvocation(CI);
    }
}
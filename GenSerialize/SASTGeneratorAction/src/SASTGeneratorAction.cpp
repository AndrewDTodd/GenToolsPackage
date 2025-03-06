#include <SASTGeneratorAction.h>

/*
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/FileEntry.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>

#include <llvm/TargetParser/Host.h>
*/
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>

#include <filesystem>

#include <PlatformInterface.h>

namespace GenTools::GenSerialize
{
    SASTGeneratorAction::SASTGeneratorAction(SASTResult& result)
        : m_result(result)
    {}

    /*
	void SASTGenerator::ProcessFiles(std::vector<std::string>& paths)
	{
		if (paths.empty())
		{
			TERMINAL::PRINT_ERROR(std::string("ERROR: No files provided to generator"));
			return;
		}

		FileValidator validator;

        for (const auto& file : paths)
        {
            clang::CompilerInstance ci;
            // Create diagnostics with a TextDiagnosticPrinter.
            ci.createDiagnostics(new clang::TextDiagnosticPrinter(llvm::errs(), new clang::DiagnosticOptions()));
            ci.getDiagnostics().getIgnoreAllWarnings();

            // Set target details.
            std::shared_ptr<clang::TargetOptions> targetOptions = std::make_shared<clang::TargetOptions>();
            targetOptions->Triple = llvm::sys::getDefaultTargetTriple(); // Detect system target.
            ci.setTarget(clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), targetOptions));

            // Set language options.
            clang::LangOptions& langOpts = ci.getLangOpts();
            langOpts.CPlusPlus = true;
            langOpts.CPlusPlus20 = true;
            langOpts.DelayedTemplateParsing = true;

            // Create file manager and source manager.
            ci.createFileManager();
            ci.createSourceManager(ci.getFileManager());

            // Open the file.
            auto fileEntry = ci.getFileManager().getFileRef(file);
            if (!fileEntry)
            {
                TERMINAL::PRINT_ERROR("ERROR: Could not open file: " + file);
                continue;
            }

            ci.getSourceManager().setMainFileID(
                ci.getSourceManager().createFileID(fileEntry.get(), clang::SourceLocation(), clang::SrcMgr::C_User));

            // Configure the preprocessor.
            ci.createPreprocessor(clang::TranslationUnitKind::TU_Complete);
            ci.getPreprocessorOpts().UsePredefines = true;
            ci.getHeaderSearchOpts().UseStandardSystemIncludes = true;
            ci.getHeaderSearchOpts().UseLibcxx = true;

            // Create AST context.
            ci.createASTContext();

            // IMPORTANT: Inform the diagnostic client that we are starting source file processing.
            ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());

            // Create and run your AST consumer/visitor.
            ASTParser astParser(m_SASTMap, m_SASTTrees[file]);
            clang::ParseAST(ci.getPreprocessor(), &astParser, ci.getASTContext());

            // End the source file processing.
            ci.getDiagnosticClient().EndSourceFile();

            // (Optional) Validate the file if AST was generated.
            if (!m_SASTTrees[file].empty())
            {
                FileValidator validator;
                if (!validator.ValidateFile(file))
                {
                    TERMINAL::PRINT_WARNING("WARNING: Validation errors in: " + file);
                }
            }
        }
	}
    */

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
        ppOpts.addMacroDef("SERIALIZE_EXCLUDE=__attribute__((annotate(\"serialize:exclude\")))");
        ppOpts.addMacroDef("GENERATED_SERIALIZATION_BODY()=");
        return clang::ASTFrontendAction::BeginInvocation(CI);
    }

	/*const std::vector<std::shared_ptr<SASTNode>>& SASTGeneratorAction::GetSASTTree() const
	{
		return m_SASTTree;
	}

	const std::unordered_map<std::string, std::shared_ptr<SASTNode>>& SASTGeneratorAction::GetSASTMap() const
	{
		return m_SASTMap;
	}

    const std::string& SASTGeneratorAction::GetFilePath() const
    {
        return m_filePath;
    }*/
}
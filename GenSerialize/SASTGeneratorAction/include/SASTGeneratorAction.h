#ifndef GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_H
#define GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <SAST.h>
#include <ASTParser.h>

#include <clang/Frontend/FrontendAction.h>

namespace GenTools::GenSerialize
{
	class SASTGeneratorAction : public clang::ASTFrontendAction
	{
	private:
		SASTResult& m_result;

	protected:
		bool BeginInvocation(clang::CompilerInstance& CI) override;

	public:
		SASTGeneratorAction(SASTResult& result);

		//void ProcessFiles(std::vector<std::string>& paths);

		std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci, llvm::StringRef inFile) override;

		/*const std::vector<std::shared_ptr<SASTNode>>& GetSASTTree() const;
		const std::unordered_map<std::string, std::shared_ptr<SASTNode>>& GetSASTMap() const;
		const std::string& GetFilePath() const;*/
	};
}

#endif // !GENTOOLS_GENSERIALIZE_SAST_GENERATOR_ACTION_H

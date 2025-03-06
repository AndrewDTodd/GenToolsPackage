#ifndef GENTOOLS_GENSERIALIZE_AST_PARSER_H
#define GENTOOLS_GENSERIALIZE_AST_PARSER_H

#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Attr.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/CompilerInstance.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <filesystem>

#include <SAST.h>

namespace GenTools::GenSerialize
{
	struct SASTResult
	{
		std::vector<std::shared_ptr<SASTNode>> SASTTree;
		std::unordered_map<std::string, std::shared_ptr<SASTNode>> SASTMap;
		std::string filePath;
	};

	class ASTParser : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ASTParser>
	{
	private:
		SASTResult& m_result;

		void ProcessFields(clang::CXXRecordDecl* recordDecl, std::shared_ptr<SASTNode> sastNode);
		
	public:
		explicit ASTParser(SASTResult& result);

		void HandleTranslationUnit(clang::ASTContext& context) override;

		bool VisitCXXRecordDecl(clang::CXXRecordDecl* recordDecl);
	};
}

#endif // !GENTOOLS_GENSERIALIZE_AST_PARSER_H

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

#include <SAST.h>

namespace GenTools::GenSerialize
{
	class ASTParser : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ASTParser>
	{
	private:
		clang::CompilerInstance& m_compilerInstance;

		// Map of SAST nodes keyed by fully qualified type name
		std::unordered_map<std::string, std::shared_ptr<SASTNode>> m_SASTNodes;

		// List of SAST nodes (one per marked type) for the translation unit
		std::vector<std::shared_ptr<SASTNode>> m_SASTList;

		void ProcessFields(clang::CXXRecordDecl* recordDecl, std::shared_ptr<SASTNode> sastNode);

	public:
		explicit ASTParser(clang::CompilerInstance& compilerInstance);

		void HandleTranslationUnit(clang::ASTContext& context) override;

		bool VisitCXXRecordDecl(clang::CXXRecordDecl* recordDecl);
	};
}

#endif // !GENTOOLS_GENSERIALIZE_AST_PARSER_H

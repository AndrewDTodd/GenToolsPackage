#include <ASTParser.h>

namespace GenTools::GenSerialize
{
	ASTParser::ASTParser(clang::CompilerInstance& compilerInstance)
		: m_compilerInstance(compilerInstance)
	{}

	void ASTParser::HandleTranslationUnit(clang::ASTContext& context)
	{
		TraverseDecl(context.getTranslationUnitDecl());

		
	}

	bool ASTParser::VisitCXXRecordDecl(clang::CXXRecordDecl* recordDecl)
	{
		// Only process definitions (not forward declarations)
		if (!recordDecl->isThisDeclarationADefinition())
		{
			return true;
		}

		/// Look for an annotation that starts with "serializable"
		bool isSerializable = false;
		SASTNode::SerializationPolicy serializationPolicy = SASTNode::SerializationPolicy::Custom;
		std::vector<std::string> formats;

		for (auto attribute : recordDecl->specific_attrs<clang::AnnotateAttr>())
		{
			std::string annotation = attribute->getAnnotation().str();
			if (annotation.find("serializable") == 0)
			{
				isSerializable = true;
				std::string params = annotation.substr(12);
				
				// Determine the policy based upon the parameter
				if (params == "all")
				{
					serializationPolicy = SASTNode::SerializationPolicy::All;
				}
				else if (params == "public")
				{
					serializationPolicy = SASTNode::SerializationPolicy::Public;
				}
				else if (params == "protected")
				{
					serializationPolicy = SASTNode::SerializationPolicy::Protected;
				}
				else if (params == "private")
				{
					serializationPolicy = SASTNode::SerializationPolicy::Private;
				}
				else if (params == "pod")
				{
					serializationPolicy = SASTNode::SerializationPolicy::POD;
				}
				else if (!params.empty())
				{
					// If parameters are provided that don't match the above,
					// treat them as a comma-separated list of formats
					std::istringstream ss(params);
					std::string format;
					while (std::getline(ss, format, ','))
					{
						formats.push_back(format);
					}
				}
			}
		}

		if (!isSerializable)
		{
			return true; // Skip types not marked for serialization
		}

		// Create a new SAST node for this type
		std::shared_ptr<SASTNode> sastNode = std::make_shared<SASTNode>();
		sastNode->name = recordDecl->getQualifiedNameAsString();
		sastNode->serializationPolicy = serializationPolicy;
		sastNode->formats = formats;

		// Process inheritance: for each base class that is serializable,
		// add a pointer to its SAST node to the baseNodes vector
		for (const auto& base : recordDecl->bases())
		{
			if (auto baseDecl = base.getType()->getAsCXXRecordDecl())
			{
				// Check if the base class even has any annotations, if not its not marked for serialization
				if (baseDecl->hasAttr<clang::AnnotateAttr>())
				{
					// Find the SAST node for the base class, if one exists
					auto baseNode = m_SASTNodes.find(baseDecl->getQualifiedNameAsString());
					if (baseNode != m_SASTNodes.end())
					{
						sastNode->baseNodes.push_back(baseNode->second);
					}
				}
			}
		}

		// Process the fields of this type
		ProcessFields(recordDecl, sastNode);

		// Save the node for later lookup and processing
		m_SASTNodes[sastNode->name] = sastNode;
		m_SASTList.push_back(sastNode);

		return true;
	}

	void ASTParser::ProcessFields(clang::CXXRecordDecl* recordDecl, std::shared_ptr<SASTNode> sastNode)
	{
		for (const auto& field : recordDecl->fields())
		{
			bool includeField = false;
			std::string customName = "";

			// Check field annotations: SERIALIZE_FIELD, SERIALIZE_FIELD_AS, SERIALIZE_EXCLUDE
			for (auto attribute : field->specific_attrs<clang::AnnotateAttr>())
			{
				std::string annotation = attribute->getAnnotation().str();
				if (annotation == "serialize")
				{
					includeField = true;
				}
				else if (annotation.find("serialize:") == 0)
				{
					std::string param = annotation.substr(10);
					if (param == "exclude")
					{
						includeField = false;
					}
					else
					{
						customName = param;
						includeField = true;
					}
				}
			}

			// If not explicitly annotated but the node uses the "All" policy, include it
			if (!includeField && sastNode->serializationPolicy == SASTNode::SerializationPolicy::All)
			{
				includeField = true;
			}

			if (!includeField)
				continue;

			SASTField sastField;
			sastField.name = customName.empty() ? field->getNameAsString() : customName;

			auto fieldType = field->getType();
			if (fieldType->isIntegerType())
			{
				sastField.type = SASTType::Int;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else if (fieldType->isFloatingType())
			{
				if (fieldType->isSpecificBuiltinType(clang::BuiltinType::Float))
					sastField.type = SASTType::Float;
				else
					sastField.type = SASTType::Double;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else if (fieldType->isBooleanType())
			{
				sastField.type = SASTType::Bool;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else if (fieldType->isRecordType())
			{
				// For user-defined types, mark as object
				sastField.type = SASTType::Object;
				sastField.originalTypeName = fieldType.getAsString();

				if (auto recordDecl = fieldType->getAsCXXRecordDecl())
				{
					std::string recordName = recordDecl->getQualifiedNameAsString();
					if (m_SASTNodes.find(recordName) != m_SASTNodes.end())
					{
						sastField.objectNode = m_SASTNodes[recordName];
					}
				}
			}
			else
			{
				// Fallback for types not covered above
				sastField.type = SASTType::Object;
				sastField.originalTypeName = fieldType.getAsString();
			}

			sastNode->fields.push_back(sastField);
		}
	}
}
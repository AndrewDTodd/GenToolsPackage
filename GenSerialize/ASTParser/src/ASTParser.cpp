#include <ASTParser.h>

#include <FileValidator.h>
#include <PlatformInterface.h>

#include <unordered_set>

namespace GenTools::GenSerialize
{
	ASTParser::ASTParser(SASTResult& result)
		: m_result(result)
	{}

	void ASTParser::HandleTranslationUnit(clang::ASTContext& context)
	{
		TraverseDecl(context.getTranslationUnitDecl());

		if (!m_result.SASTTree.empty())
		{
			FileValidator validator;
			validator.ValidateFile(m_result.filePath);
		}
	}

	bool ASTParser::VisitCXXRecordDecl(clang::CXXRecordDecl* recordDecl)
	{
		// Get the source manager from the ASTContext
		clang::SourceManager& SM = recordDecl->getASTContext().getSourceManager();

		// Skip declarations in system headers
		if (SM.isInSystemHeader(recordDecl->getLocation()))
			return true; // Continue traversal, but skip processing this record

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
				std::string params = annotation.substr(13);
				
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
					auto baseNode = m_result.SASTMap.find(baseDecl->getQualifiedNameAsString());
					if (baseNode != m_result.SASTMap.end())
					{
						sastNode->baseNodes.push_back(baseNode->second);
					}
				}
			}
		}

		// Process the fields of this type
		ProcessFields(recordDecl, sastNode);

		// Save the node for later lookup and processing
		m_result.SASTMap[sastNode->name] = sastNode;
		m_result.SASTTree.push_back(sastNode);

		return true;
	}

	void ASTParser::ProcessFields(clang::CXXRecordDecl* recordDecl, std::shared_ptr<SASTNode> sastNode)
	{
		std::unordered_set<std::string> fieldNames;
		std::vector<std::pair<std::string, std::string>> dynamicLengthReferences; // {arrayFieldName, LengthVar}

		for (const auto& field : recordDecl->fields())
		{
			fieldNames.insert(field->getName().str());

			bool includeField = false;
			std::string customName = "";

			bool isStaticArray = false;
			std::optional<std::string> dynamicLengthVar;

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
				else if (annotation == "static_array")
				{
					isStaticArray = true;
				}
				else if (annotation.find("dynamic_array:") == 0)
				{
					dynamicLengthVar = annotation.substr(14);
				}
			}

			// If not explicitly annotated but the node uses the "All" policy, include it
			if (!includeField && sastNode->serializationPolicy == SASTNode::SerializationPolicy::All)
			{
				includeField = true;
			}
			// If not explicitly annotated but the node uses the "POD" policy, include it
			else if (!includeField && sastNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
			{
				includeField = true;
			}

			if (!includeField)
				continue;

			SASTField sastField;
			sastField.name = field->getName().str();
			sastField.formattedName = customName.empty() ? sastField.name : customName;

			switch (field->getAccess())
			{
			case clang::AS_public:
				sastField.access = SASTField::Access::Public;
				break;

			case clang::AS_protected:
				sastField.access = SASTField::Access::Protected;
				if(sastNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
					throw std::runtime_error("Protected fields are not allowed in POD types");
				break;

			case clang::AS_private:
				sastField.access = SASTField::Access::Private;
				if (sastNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
					throw std::runtime_error("Private fields are not allowed in POD types");
				break;

			default:
				sastField.access = SASTField::Access::Private;
				break;
			}

			auto fieldType = field->getType();

			// Handle static arrays
			if (fieldType->isArrayType())
			{
				if (isStaticArray)
				{
					if (!fieldType->isConstantArrayType())
						throw std::runtime_error("Field '" + sastField.name + "' is marked static_array but is not statically sized");

					sastField.type = SASTType::Array;
				}
				else
					throw std::runtime_error("Statically sized array marked for serialization but not marked, or is improperly marked as STATIC_ARRAY");

				sastField.originalTypeName = fieldType.getAsString();

				if (auto arrayType = clang::dyn_cast<clang::ArrayType>(fieldType.getTypePtr()))
				{
					auto elemType = arrayType->getElementType();
					sastField.elementType = ProcessFieldType(elemType);
				}
			}
			// Handle dynamic arrays
			else if (fieldType->isPointerType())
			{
				if (isStaticArray)
					throw std::runtime_error("Field '" + sastField.name + "' is marked STATIC_ARRAY but is not statically sized");
				else if (!dynamicLengthVar.has_value())
					throw std::runtime_error("Field '" + sastField.name + "' is a dynamic array marked for serialization, but has no valid length variable attribute.\nEnsure field is properly marked with DYNAMIC_ARRAY(lengthVar), lengthVar being the name of another field which contains the array's element count");

				sastField.type = SASTType::Dynamic_Array;
				sastField.lengthVar = dynamicLengthVar.value();

				dynamicLengthReferences.emplace_back(sastField.name, sastField.lengthVar);

				sastField.originalTypeName = fieldType.getAsString();

				if (auto ptrType = clang::dyn_cast<clang::PointerType>(fieldType.getTypePtr()))
				{
					auto elemType = ptrType->getPointeeType();
					sastField.elementType = ProcessFieldType(elemType);
				}
			}
			// Handle template specializations (i.e. STL containers)
			else if (const auto* specType = fieldType->getAs<clang::TemplateSpecializationType>())
			{
				if (const auto* tmplDecl = specType->getTemplateName().getAsTemplateDecl())
				{
					std::string tmplName = tmplDecl->getNameAsString();
					sastField.originalTypeName = fieldType.getAsString();

					if (tmplName == "vector")
					{
						sastField.type = SASTType::Vector;
						if (!specType->template_arguments().empty())
						{
							auto arg = specType->template_arguments()[0];
							if (arg.getKind() == clang::TemplateArgument::Type)
							{
								auto elemType = arg.getAsType();
								sastField.elementType = ProcessFieldType(elemType);
							}
						}
					}
					else if (tmplName == "set")
					{
						sastField.type = SASTType::Set;
						if (!specType->template_arguments().empty())
						{
							auto arg = specType->template_arguments()[0];
							if (arg.getKind() == clang::TemplateArgument::Type)
							{
								auto elemType = arg.getAsType();
								sastField.elementType = ProcessFieldType(elemType);
							}
						}
					}
					else if (tmplName == "unordered_set")
					{
						sastField.type = SASTType::Unordered_Set;
						if (!specType->template_arguments().empty())
						{
							auto arg = specType->template_arguments()[0];
							if (arg.getKind() == clang::TemplateArgument::Type)
							{
								auto elemType = arg.getAsType();
								sastField.elementType = ProcessFieldType(elemType);
							}
						}
					}
					else if (tmplName == "map")
					{
						sastField.type = SASTType::Map;
						if (specType->template_arguments().size() >= 2)
						{
							auto keyArg = specType->template_arguments()[0];
							auto valueArg = specType->template_arguments()[1];
							if (keyArg.getKind() == clang::TemplateArgument::Type)
							{
								auto keyType = keyArg.getAsType();
								sastField.keyType = ProcessFieldType(keyType);
							}
							if (valueArg.getKind() == clang::TemplateArgument::Type)
							{
								auto valueType = valueArg.getAsType();
								sastField.valueType = ProcessFieldType(valueType);
							}
						}
					}
					else if (tmplName == "unordered_map")
					{
						sastField.type = SASTType::Unordered_Map;
						if (specType->template_arguments().size() >= 2)
						{
							auto keyArg = specType->template_arguments()[0];
							auto valueArg = specType->template_arguments()[1];
							if (keyArg.getKind() == clang::TemplateArgument::Type)
							{
								auto keyType = keyArg.getAsType();
								sastField.keyType = ProcessFieldType(keyType);
							}
							if (valueArg.getKind() == clang::TemplateArgument::Type)
							{
								auto valueType = valueArg.getAsType();
								sastField.valueType = ProcessFieldType(valueType);
							}
						}
					}
					else
					{
						// Fallback: treat as a regular object if the template type is not recognized
						sastField.type = SASTType::Object;
						sastField.originalTypeName = fieldType.getAsString();

						if (sastField.originalTypeName == "std::string")
						{
							sastField.type = SASTType::String;
						}
						else if (auto recordDecl = fieldType->getAsCXXRecordDecl())
						{
							std::string recordName = recordDecl->getQualifiedNameAsString();
							if (m_result.SASTMap.find(recordName) != m_result.SASTMap.end())
							{
								sastField.objectNode = m_result.SASTMap[recordName];
								if (sastField.objectNode)
								{
									if (sastField.objectNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
										sastField.type = SASTType::POD;
								}
							}
						}
					}
				}
			}
			else if (fieldType->isRecordType())
			{
				// For user-defined types, mark as object
				sastField.type = SASTType::Object;
				sastField.originalTypeName = fieldType.getAsString();

				if (sastField.originalTypeName == "std::string")
				{
					sastField.type = SASTType::String;
				}
				else if (auto recordDecl = fieldType->getAsCXXRecordDecl())
				{
					std::string recordName = recordDecl->getQualifiedNameAsString();
					if (m_result.SASTMap.find(recordName) != m_result.SASTMap.end())
					{
						sastField.objectNode = m_result.SASTMap[recordName];
						if (sastField.objectNode)
						{
							if (sastField.objectNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
								sastField.type = SASTType::POD;
						}
					}
				}
			}
			else if (fieldType->isIntegerType())
			{
				sastField.type = SASTType::Int;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else if (fieldType->isFloatingType())
			{
				sastField.type = SASTType::Float;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else if (fieldType->isBooleanType())
			{
				sastField.type = SASTType::Bool;
				sastField.originalTypeName = fieldType.getAsString();
			}
			else
			{
				// Fallback for types not covered above
				sastField.type = SASTType::Object;
				sastField.originalTypeName = fieldType.getAsString();
			}

			sastNode->fields.push_back(sastField);
		}

		for (const auto& [arrayName, lenVar] : dynamicLengthReferences)
		{
			if (!fieldNames.contains(lenVar))
			{
				throw std::runtime_error("Dynamic array field '" + arrayName + "' refers to length variable '" + lenVar + "', but that variable does not exist in the object");
			}
		}
	}

	std::shared_ptr<SASTField> ASTParser::ProcessFieldType(const clang::QualType& fieldType)
	{
		auto sastField = std::make_shared<SASTField>();
		sastField->name = "";

		if(fieldType->isPointerType())
			throw std::runtime_error("Element type found to be array but not statically sized array, this is not supported. Serialized container contains dynamically allocated arrays.");

		// Handle C-style arrays (or similar array types)
		if (fieldType->isArrayType())
		{
			sastField->type = SASTType::Array;
			sastField->originalTypeName = fieldType.getAsString();

			if (auto arrayType = clang::dyn_cast<clang::ArrayType>(fieldType.getTypePtr()))
			{
				auto elemType = arrayType->getElementType();
				sastField->elementType = ProcessFieldType(elemType);
			}
		}
		// Handle template specializations (i.e. STL containers)
		else if (const auto* specType = fieldType->getAs<clang::TemplateSpecializationType>())
		{
			if (const auto* tmplDecl = specType->getTemplateName().getAsTemplateDecl())
			{
				std::string tmplName = tmplDecl->getNameAsString();
				sastField->originalTypeName = fieldType.getAsString();

				if (tmplName == "vector")
				{
					sastField->type = SASTType::Vector;
					if (!specType->template_arguments().empty())
					{
						auto arg = specType->template_arguments()[0];
						if (arg.getKind() == clang::TemplateArgument::Type)
						{
							auto elemType = arg.getAsType();
							sastField->elementType = ProcessFieldType(elemType);
						}
					}
				}
				else if (tmplName == "set")
				{
					sastField->type = SASTType::Set;
					if (!specType->template_arguments().empty())
					{
						auto arg = specType->template_arguments()[0];
						if (arg.getKind() == clang::TemplateArgument::Type)
						{
							auto elemType = arg.getAsType();
							sastField->elementType = ProcessFieldType(elemType);
						}
					}
				}
				else if (tmplName == "unordered_set")
				{
					sastField->type = SASTType::Unordered_Set;
					if (!specType->template_arguments().empty())
					{
						auto arg = specType->template_arguments()[0];
						if (arg.getKind() == clang::TemplateArgument::Type)
						{
							auto elemType = arg.getAsType();
							sastField->elementType = ProcessFieldType(elemType);
						}
					}
				}
				else if (tmplName == "map")
				{
					sastField->type = SASTType::Map;
					if (specType->template_arguments().size() >= 2)
					{
						auto keyArg = specType->template_arguments()[0];
						auto valueArg = specType->template_arguments()[1];
						if (keyArg.getKind() == clang::TemplateArgument::Type)
						{
							auto keyType = keyArg.getAsType();
							sastField->keyType = ProcessFieldType(keyType);
						}
						if (valueArg.getKind() == clang::TemplateArgument::Type)
						{
							auto valueType = valueArg.getAsType();
							sastField->valueType = ProcessFieldType(valueType);
						}
					}
				}
				else if (tmplName == "unordered_map")
				{
					sastField->type = SASTType::Unordered_Map;
					if (specType->template_arguments().size() >= 2)
					{
						auto keyArg = specType->template_arguments()[0];
						auto valueArg = specType->template_arguments()[1];
						if (keyArg.getKind() == clang::TemplateArgument::Type)
						{
							auto keyType = keyArg.getAsType();
							sastField->keyType = ProcessFieldType(keyType);
						}
						if (valueArg.getKind() == clang::TemplateArgument::Type)
						{
							auto valueType = valueArg.getAsType();
							sastField->valueType = ProcessFieldType(valueType);
						}
					}
				}
				else
				{
					// Fallback: treat as a regular object if the template type is not recognized
					sastField->type = SASTType::Object;
					sastField->originalTypeName = fieldType.getAsString();

					if (sastField->originalTypeName == "std::string")
					{
						sastField->type = SASTType::String;
					}
					else if (auto recordDecl = fieldType->getAsCXXRecordDecl())
					{
						std::string recordName = recordDecl->getQualifiedNameAsString();
						if (m_result.SASTMap.find(recordName) != m_result.SASTMap.end())
						{
							sastField->objectNode = m_result.SASTMap[recordName];
							if (sastField->objectNode)
							{
								if (sastField->objectNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
									sastField->type = SASTType::POD;
							}
						}
					}
				}
			}
		}
		else if (fieldType->isRecordType())
		{
			// For user-defined types, mark as object
			sastField->type = SASTType::Object;
			sastField->originalTypeName = fieldType.getAsString();

			if (sastField->originalTypeName == "std::string")
			{
				sastField->type = SASTType::String;
			}
			else if (auto recordDecl = fieldType->getAsCXXRecordDecl())
			{
				std::string recordName = recordDecl->getQualifiedNameAsString();
				if (m_result.SASTMap.find(recordName) != m_result.SASTMap.end())
				{
					sastField->objectNode = m_result.SASTMap[recordName];
					if (sastField->objectNode)
					{
						if (sastField->objectNode->serializationPolicy == SASTNode::SerializationPolicy::POD)
							sastField->type = SASTType::POD;
					}
				}
			}
		}
		else if (fieldType->isIntegerType())
		{
			sastField->type = SASTType::Int;
			sastField->originalTypeName = fieldType.getAsString();
		}
		else if (fieldType->isFloatingType())
		{
			sastField->type = SASTType::Float;
			sastField->originalTypeName = fieldType.getAsString();
		}
		else if (fieldType->isBooleanType())
		{
			sastField->type = SASTType::Bool;
			sastField->originalTypeName = fieldType.getAsString();
		}
		else
		{
			// Fallback for types not covered above
			sastField->type = SASTType::Object;
			sastField->originalTypeName = fieldType.getAsString();
		}

		return sastField;
	}
}
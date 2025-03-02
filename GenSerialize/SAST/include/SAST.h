#ifndef GENTOOLS_GENSERIALIZE_SAST_H
#define GENTOOLS_GENSERIALIZE_SAST_H

#include <string>
#include <vector>
#include <memory>

namespace GenTools::GenSerialize
{
	/// <summary>
	/// An enum representing the basic types supported for serialization
	/// </summary>
	enum class SASTType
	{
		Int,
		Float,
		Double,
		Bool,
		String,
		Object
	};

	struct SASTNode;

	struct SASTField
	{
		std::string name;						// The field name (or custom name if specified)
		SASTType type;							// The type of the field
		std::string originalTypeName;			// The C++ type name of the field, for debugging or further processing
		std::shared_ptr<SASTNode> objectNode;	// For complex types, a link to the SASTNode representing that type
	};

	struct SASTNode
	{
		std::string name; 						// The fully qualified type name of the class or struct
		
		// The serialization or "access" policy for this type
		enum class SerializationPolicy
		{
			POD,								// POD (SERIALIZABLE_POD)
			All,								// All (SERIALIZABLE_ALL),
			Public,								// Public (SERIALIZABLE_PUBLIC),	
			Protected,							// Protected (SERIALIZABLE_PROTECTED),
			Private,							// Private (SERIALIZABLE_PRIVATE),
			Custom								// Custom (SERIALIZABLE)
		} serializationPolicy = SerializationPolicy::Custom;

		std::vector<std::string> formats;		// The formats to use for serialization
		std::vector<SASTField> fields;			// The fields of the class or struct for serialization

		// Pointers to tge SAST nodes for any serializable base classes
		// For multiple inheritance, this can hold more than one parent
		std::vector<std::shared_ptr<SASTNode>> baseNodes;
	};
}

#endif // !GENTOOLS_GENSERIALIZE_SAST_H

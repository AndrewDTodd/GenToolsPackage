#pragma once
#include <string>
#include <SerializationMacros.h>
#include "SimpleType.generated.h"
#include "NestedType1.h"

class SERIALIZABLE(JSON) SimpleType
{
private:
	SERIALIZE_FIELD NestedType1 nested;
	SERIALIZE_FIELD std::string name;

public:
	GENERATED_SERIALIZATION_BODY()
};
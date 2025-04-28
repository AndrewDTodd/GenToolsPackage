#pragma once
#include <vector>
#include <SerializationMacros.h>
#include "NestedType1.generated.h"
#include "PODType1.h"

class SERIALIZABLE(JSON) NestedType1
{
private:
	SERIALIZE_FIELD PODType1 data;

	SERIALIZE_FIELD std::vector<int> values;

public:
	GENERATED_SERIALIZATION_BODY()
};
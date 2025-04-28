#pragma once
#include "SerializationMacros.h"
#include "NestedType2.generated.h"

#include "PODType2.h"

class SERIALIZABLE(JSON) NestedType2
{
private:
	SERIALIZE_FIELD PODType2 info;
	SERIALIZE_FIELD_AS(active) bool enabled;

public:
	GENERATED_SERIALIZATION_BODY()
};
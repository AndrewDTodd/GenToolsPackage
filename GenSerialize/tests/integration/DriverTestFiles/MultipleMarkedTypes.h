#pragma once
#include <vector>

#include "SerializationMacros.h"
#include "MultipleMarkedTypes.generated.h"

#include "NestedType2.h"
#include "PODType1.h"
#include "PODType2.h"

#ifndef FirstMarkedType_SERIALIZATION_BODY
#define FirstMarkedType_SERIALIZATION_BODY()
#endif

#ifndef SecondMarkedType_SERIALIZATION_BODY
#define SecondMarkedType_SERIALIZATION_BODY()
#endif

class SERIALIZABLE(JSON) FirstMarkedType
{
private:
	SERIALIZE_FIELD NestedType2 nestedData;
	SERIALIZE_FIELD PODType1 meta;

public:
	FirstMarkedType_SERIALIZATION_BODY()
};

class SERIALIZABLE(JSON) SecondMarkedType
{
private:
	SERIALIZE_FIELD std::vector<PODType2> collection;
	SERIALIZE_FIELD int count;

public:
	SecondMarkedType_SERIALIZATION_BODY()
};
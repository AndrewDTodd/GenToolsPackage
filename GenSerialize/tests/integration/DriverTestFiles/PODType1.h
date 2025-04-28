#pragma once
#include <string>
#include "SerializationMacros.h"

struct SERIALIZABLE_POD PODType1
{
	int id;
	float value;
	std::string label;
};
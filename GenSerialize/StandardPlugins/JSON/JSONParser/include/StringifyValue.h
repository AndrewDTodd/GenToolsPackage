#ifndef GENTOOLS_GENSERIALIZE_STRINGIFY_VALUE_H
#define GENTOOLS_GENSERIALIZE_STRINGIFY_VALUE_H

#include <string>
#include <memory>

#include <JSONValue.h>

namespace GenTools::GenSerialize::JSON
{
    std::string StringifyValue(const std::unique_ptr<JSONValue>& value, int level, int indent);
}

#endif // !GENTOOLS_GENSERIALIZE_STRINGIFY_VALUE_H

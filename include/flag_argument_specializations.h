#ifndef FLAG_ARGUMENT_SPECIALIZATIONS_H
#define FLAG_ARGUMENT_SPECIALIZATIONS_H

#include <flag_argument.h>
#include <parse_functions.h>

namespace TokenValueParser
{
	//Flag Aggregate Specializations
	//************************************************************************************************
	class Arg_Bool : public flag_argument_t<bool>
	{
	public:
		Arg_Bool() : flag_argument_t<bool>(false, StringToBool) {}
		Arg_Bool(bool&& defaultValue) : flag_argument_t<bool>(std::move(defaultValue), StringToBool) {}
	};

	class Arg_Int32 : public flag_argument_t<int32_t>
	{
	public:
		Arg_Int32() : flag_argument_t<int32_t>(0, ParseInt32) {}
		Arg_Int32(int32_t&& defaultValue) : flag_argument_t<int32_t>(std::move(defaultValue), ParseInt32) {}
	};

	class Arg_UInt32 : public flag_argument_t<uint32_t>
	{
	public:
		Arg_UInt32() : flag_argument_t<uint32_t>(0, ParseUInt32) {}
		Arg_UInt32(uint32_t&& defaultValue) : flag_argument_t<uint32_t>(std::move(defaultValue), ParseUInt32) {}
	};

	class Arg_Int64 : public flag_argument_t<int64_t>
	{
	public:
		Arg_Int64() : flag_argument_t<int64_t>(0, ParseInt64) {}
		Arg_Int64(int64_t&& defaultValue) : flag_argument_t<int64_t>(std::move(defaultValue), ParseInt64) {}
	};

	class Arg_UInt64 : public flag_argument_t<uint64_t>
	{
	public:
		Arg_UInt64() : flag_argument_t<uint64_t>(0, ParseUInt64) {}
		Arg_UInt64(uint64_t&& defaultValue) : flag_argument_t<uint64_t>(std::move(defaultValue), ParseUInt64) {}
	};

	class Arg_Float : public flag_argument_t<float>
	{
	public:
		Arg_Float() : flag_argument_t<float>(0.0f, ParseFloat) {}
		Arg_Float(float&& defaultValue) : flag_argument_t<float>(std::move(defaultValue), ParseFloat) {}
	};

	class Arg_Double : public flag_argument_t <double>
	{
	public:
		Arg_Double() : flag_argument_t<double>(0.0, ParseDouble) {}
		Arg_Double(double&& defaultValue) : flag_argument_t<double>(std::move(defaultValue), ParseDouble) {}
	};

	class Arg_LongDouble : public flag_argument_t<long double>
	{
	public:
		Arg_LongDouble() : flag_argument_t<long double>(0.0L, ParseLongDouble) {}
		Arg_LongDouble(long double&& defaultValue) : flag_argument_t<long double>(std::move(defaultValue), ParseLongDouble) {}
	};

	class Arg_String : public flag_argument_t<std::string>
	{
	public:
		Arg_String() : flag_argument_t<std::string>("", ParseString) {}
		Arg_String(std::string&& defaultValue) : flag_argument_t<std::string>(std::move(defaultValue), ParseString) {}
	};

	class Arg_DelString : public flag_argument_t<std::string>
	{
	public:
		Arg_DelString() : flag_argument_t<std::string>("", ParseStringDelimited) {}
		Arg_DelString(std::string&& defaultValue) : flag_argument_t<std::string>(std::move(defaultValue), ParseStringDelimited) {}
	};

	class Ptr_Bool : public flag_pointer_t<bool>
	{
	public:
		Ptr_Bool() : flag_pointer_t<bool>(StringToBool) {}
		Ptr_Bool(bool* targetBool) : flag_pointer_t<bool>(targetBool, StringToBool) {}
	};

	class Ptr_Int32 : public flag_pointer_t<int32_t>
	{
	public:
		Ptr_Int32() : flag_pointer_t<int32_t>(ParseInt32) {}
		Ptr_Int32(int32_t* targetInteger) : flag_pointer_t<int32_t>(targetInteger, ParseInt32) {}
	};

	class Ptr_UInt32 : public flag_pointer_t<uint32_t>
	{
	public:
		Ptr_UInt32() : flag_pointer_t<uint32_t>(ParseUInt32) {}
		Ptr_UInt32(uint32_t* targetUnsignedInteger) : flag_pointer_t<uint32_t>(targetUnsignedInteger, ParseUInt32) {}
	};

	class Ptr_Int64 : public flag_pointer_t<int64_t>
	{
	public:
		Ptr_Int64() : flag_pointer_t<int64_t>(ParseInt64) {}
		Ptr_Int64(int64_t* targetInteger) : flag_pointer_t<int64_t>(targetInteger, ParseInt64) {}
	};

	class Ptr_UInt64 : public flag_pointer_t<uint64_t>
	{
	public:
		Ptr_UInt64() : flag_pointer_t<uint64_t>(ParseUInt64) {}
		Ptr_UInt64(uint64_t* targetUnsignedInteger) : flag_pointer_t<uint64_t>(targetUnsignedInteger, ParseUInt64) {}
	};

	class Ptr_Float : public flag_pointer_t<float>
	{
	public:
		Ptr_Float() : flag_pointer_t<float>(ParseFloat) {}
		Ptr_Float(float* targetFloat) : flag_pointer_t<float>(targetFloat, ParseFloat) {}
	};

	class Ptr_Double : public flag_pointer_t <double>
	{
	public:
		Ptr_Double() :flag_pointer_t<double>(ParseDouble) {}
		Ptr_Double(double* targetDouble) : flag_pointer_t<double>(targetDouble, ParseDouble) {}
	};

	class Ptr_LongDouble : public flag_pointer_t<long double>
	{
	public:
		Ptr_LongDouble() : flag_pointer_t<long double>(ParseLongDouble) {}
		Ptr_LongDouble(long double* targetLDouble) : flag_pointer_t<long double>(targetLDouble, ParseLongDouble) {}
	};

	class Ptr_String : public flag_pointer_t<std::string>
	{
	public:
		Ptr_String() : flag_pointer_t<std::string>(ParseString) {}
		Ptr_String(std::string* targetString) : flag_pointer_t<std::string>(targetString, ParseString) {}
	};

	class Ptr_DelString : public flag_pointer_t<std::string>
	{
	public:
		Ptr_DelString() : flag_pointer_t<std::string>(ParseStringDelimited) {}
		Ptr_DelString(std::string* targetString) : flag_pointer_t<std::string>(targetString, ParseStringDelimited) {}
	};
	//************************************************************************************************
}
#endif // !FLAG_ARGUMENT_SPECIALIZATIONS_H

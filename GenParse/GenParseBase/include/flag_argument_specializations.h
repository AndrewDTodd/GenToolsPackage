#ifndef GENTOOLS_GENPARSE_FLAG_ARGUMENT_SPECIALIZATIONS_H
#define GENTOOLS_GENPARSE_FLAG_ARGUMENT_SPECIALIZATIONS_H

#include <flag_argument.h>
#include <parse_functions.h>

namespace GenTools::GenParse
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

	//////////////////////////////////////////////////////////////////////////////
	// Arg_List
	//
	// Templated on BaseArg, which is an existing flag_argument_t
	// specialization (e.g. Arg_DelString, Arg_Double, etc.). This class
	// derives from flag_argument_t<std::vector<T>> where T is BaseArg's value type.
	//
	// It stores a pointer to the element parsing function (of type T(*)(const char*))
	// and in its Parse/TryParse methods it uses the stored parsing function to parse
	// the input stream until it fails to parse or reaches the end delimiter if specified.
	//
	// The template booleans Enclosed and Separated allow you to specify:
	//   - Enclosed: whether the whole list must be wrapped (e.g. in '[' and ']')
	//   - Separated: whether the list’s elements are separated by a fixed
	//                delimiter (for example, a comma)
	//////////////////////////////////////////////////////////////////////////////
	template<IsFlagArgument BaseArg, bool Enclosed = false, bool Separated = false>
	class Arg_List : public flag_argument_t<std::vector<typename BaseArg::value_type>>
	{
	public:
		using element_type = typename BaseArg::value_type;
		using container_type = std::vector<element_type>;

	private:
		char m_leftDelimiter;
		char m_rightDelimiter;
		char m_separator;

		// The element parse function pointer
		// (For Arg_DelString this might be ParseStringDelimited, for Arg_Double it might be ParseDouble, etc.)
		element_type(*elementParseFunc)(const char*);

		static std::string Trim(const std::string& str)
		{
			size_t start = str.find_first_not_of(" \t\n");
			size_t end = str.find_last_not_of(" \t\n");
			if (start == std::string::npos || end == std::string::npos)
				return "";
			return str.substr(start, end - start + 1);
		}

	public:
		// Constructor
		// The user must provide the element parse function pointer
		Arg_List(const char leftDelimiter = '[', const char rightDelimiter = ']', const char separator = ',')
			: flag_argument_t<container_type>(container_type()), 
			m_leftDelimiter(leftDelimiter), m_rightDelimiter(rightDelimiter), m_separator(separator)
		{
			BaseArg base;
			elementParseFunc = base.GetParseFunction();
		}

		Arg_List(container_type&& defaultValue, 
			const char leftDelimiter = '[', const char rightDelimiter = ']', const char separator = ',')
			: flag_argument_t<container_type>(std::forward<container_type>(defaultValue)),
			m_leftDelimiter(leftDelimiter), m_rightDelimiter(rightDelimiter), m_separator(separator)
		{
			BaseArg base;
			elementParseFunc = base.GetParseFunction();
		}

		// Override Parse: interpret the provided string (which is assumes to contain
		// the entire list, either as a single token or with the delimiters embedded)
		// and split it into individual elements
		void Parse(const char* str) const override
		{
			auto& container = this->argument;
			container.clear();
			std::string input(str);
			std::string listContent;

			if constexpr (Enclosed)
			{
				// The list must be enclosed in its delimiters (e.g. '[' and ']')
				if (input.empty() || input.front() != m_leftDelimiter || input.back() != m_rightDelimiter)
				{
					throw std::invalid_argument(std::string("List must be enclosed in delimiters, '") + m_leftDelimiter + "' and '" + m_rightDelimiter + "'");
				}

				listContent = input.substr(1, input.size() - 2);
			}
			else
			{
				listContent = input;
			}

			// Split the listContent into individual elements
			std::vector<std::string> elements;
			if constexpr (Separated)
			{
				// For separated items, split the listContent by the separator
				size_t start = 0;
				size_t pos = 0;
				while ((pos = listContent.find(m_separator, start)) != std::string::npos)
				{
					std::string token = Trim(listContent.substr(start, pos - start));
					if (!token.empty())
						elements.push_back(token);

					start = pos + 1;
				}

				std::string token = Trim(listContent.substr(start));
				if (!token.empty())
					elements.push_back(token);
			}
			else
			{
				// Otherwise, split by whitespace
				std::istringstream iss(listContent);
				std::string token;
				while (iss >> token)
				{
					elements.push_back(token);
				}
			}

			// For each element, parse it and add it to the container
			for (const auto& element : elements)
			{
				container.push_back(elementParseFunc(element.c_str()));
			}
		}

		bool TryParse(const char* str, std::string* errorMsg = nullptr) const noexcept override
		{
			try
			{
				Parse(str);
			}
			catch (const std::exception& e)
			{
				if (errorMsg)
					*errorMsg = e.what();
				return false;
			}

			return true;
		}

		std::string GetArgType() const noexcept override
		{
			BaseArg base;
			return "List<" + base.GetArgType() + ">";
		}
	};
	//************************************************************************************************
}
#endif // !GENTOOLS_GENPARSE_FLAG_ARGUMENT_SPECIALIZATIONS_H

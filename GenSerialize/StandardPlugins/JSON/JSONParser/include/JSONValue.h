#ifndef GENTOOLS_GENSERIALIZE_JSON_VALUE_H
#define GENTOOLS_GENSERIALIZE_JSON_VALUE_H

#include <stdexcept>
#include <string>
#include <concepts>

#include <IFormatPlugin.h>

namespace GenTools::GenSerialize
{
	enum class FORMAT_PLUGIN_ABI JSONType
	{
		String,
		Number,
		Bool,
		Array,
		Object,
		Null
	};

	class FORMAT_PLUGIN_ABI JSONValue
	{
	public:
		virtual ~JSONValue() = default;

		template<typename T>
		auto as()
#if !defined(DEBUG) && !defined(_DEBUG)
			noexcept
#endif // !DEBUG
			;
		
		template<typename T>
		const auto as() const
#if !defined(DEBUG) && !defined(_DEBUG)
			noexcept
#endif // !DEBUG
			;

		virtual JSONType Type() const noexcept = 0;
	};

	struct FORMAT_PLUGIN_ABI JSONString : public JSONValue
	{
		std::string value;

		explicit JSONString(const std::string& val) noexcept;

		const std::string& Get() const noexcept;

		JSONType Type() const noexcept final;
	};

	struct FORMAT_PLUGIN_ABI JSONNumber : public JSONValue
	{
		double value;

		explicit JSONNumber(double val) noexcept;

		double Get() const noexcept;

		JSONType Type() const noexcept final;
	};

	struct FORMAT_PLUGIN_ABI JSONBool : public JSONValue
	{
		bool value;

		explicit JSONBool(bool val) noexcept;

		bool Get() const noexcept;

		JSONType Type() const noexcept final;
	};

	struct FORMAT_PLUGIN_ABI JSONNull : public JSONValue
	{
		JSONNull() = default;

		JSONType Type() const noexcept final;
	};

	template<typename T>
	concept IsJSONValue = std::is_base_of_v<JSONValue, T>&& std::movable<T>;
}

#include <JSONValue.inl>

#endif // !GENTOOLS_GENSERIALIZE_JSON_VALUE_H

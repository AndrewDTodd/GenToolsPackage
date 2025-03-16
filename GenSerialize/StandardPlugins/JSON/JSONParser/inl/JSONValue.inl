#ifndef GENTOOLS_GENSERIALIZE_JSON_VALUE_INL
#define GENTOOLS_GENSERIALIZE_JSON_VALUE_INL

#if defined(__GNUC__) or defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif

namespace GenTools::GenSerialize
{
	template<typename T>
	FORCE_INLINE auto JSONValue::as()
#if !defined(DEBUG) && !defined(_DEBUG)
		noexcept
#endif // !DEBUG
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (auto derived = dynamic_cast<T*>(this))
			return derived;
		throw std::bad_cast();
#else
		return static_cast<T*>(this);
#endif // DEBUG
	}

	template<typename T>
	FORCE_INLINE const auto JSONValue::as() const
#if !defined(DEBUG) && !defined(_DEBUG)
		noexcept
#endif // !DEBUG
	{
#if defined(DEBUG) || defined(_DEBUG)
		if (auto derived = dynamic_cast<const T*>(this))
			return derived;
		throw std::bad_cast();
#else
		return static_cast<const T*>(this);
#endif // DEBUG
	}

	FORCE_INLINE JSONString::JSONString(const std::string& val) noexcept
		: value(val)
	{}

	FORCE_INLINE const std::string& JSONString::Get() const noexcept
	{
		return value;
	}

	FORCE_INLINE JSONType JSONString::Type() const noexcept
	{
		return JSONType::String;
	}

	FORCE_INLINE JSONNumber::JSONNumber(double val) noexcept
		: value(val)
	{}

	FORCE_INLINE double JSONNumber::Get() const noexcept
	{
		return value;
	}

	FORCE_INLINE JSONType JSONNumber::Type() const noexcept
	{
		return JSONType::Number;
	}

	FORCE_INLINE JSONBool::JSONBool(bool val) noexcept
		:value(val)
	{}

	FORCE_INLINE bool JSONBool::Get() const noexcept
	{
		return value;
	}

	FORCE_INLINE JSONType JSONBool::Type() const noexcept
	{
		return JSONType::Bool;
	}

	FORCE_INLINE JSONType JSONNull::Type() const noexcept
	{
		return JSONType::Null;
	}
}

#endif // !GENTOOLS_GENSERIALIZE_JSON_VALUE_INL

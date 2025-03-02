#ifndef GENTOOLS_GENSERIALIZE_MACROS_H
#define GENTOOLS_GENSERIALIZE_MACROS_H

#ifdef __clang__
// MACRO to mark an object for serialization. Attribute list specifies formats to create logic for
#define SERIALIZABLE(...) __attribute__((annotate("serializable:" #__VA_ARGS__)))
#else
// MACRO to mark an object for serialization. Attribute list specifies formats to create logic for
#define SERIALIZABLE(...)
#endif

#ifdef __clang__
// MACRO to mark an object for serialization, defaulting to include all fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_ALL(...) __attribute__((annotate("serializable:all")))
#else
// MACRO to mark an object for serialization, defaulting to include all fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_ALL(...)
#endif

#ifdef __clang__
// MACRO to mark an object for serialization, defaulting to include only public fields. Attribute list specifies formats to create logic for
#defie SERIALIZABLE_PUBLIC(...) __attribute__((annotate("serializable:public")))
#else
// MACRO to mark an object for serialization, defaulting to include only public fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_PUBLIC(...)
#endif

#ifdef 
// MACRO to mark an object for serialization, defaulting to include only protected fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_PROTECTED(...) __attribute__((annotate("serializable:protected")))
#else
// MACRO to mark an object for serialization, defaulting to include only protected fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_PROTECTED(...)
#endif

#ifdef __clang__
// MACRO to mark an object for serialization, defaulting to include only private fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_PRIVATE(...) __attribute__((annotate("serializable:private")))
#else
// MACRO to mark an object for serialization, defaulting to include only private fields. Attribute list specifies formats to create logic for
#define SERIALIZABLE_PRIVATE(...)
#endif

#ifdef __clang__
// MACRO to mark a Plane Old Data (POD) object for serialization. 
// Will include all fields of the object for serialization in other types that contain instances of this POD type, but not produce serialization logic in the POD type itself.
#define SERIALIZABLE_POD __attribute__((annotate("serializable:pod")))
#else
// MACRO to mark a Plane Old Data (POD) object for serialization.
// Will include all fields of the object for serialization in other types that contain instances of this POD type, but not produce serialization logic in the POD type itself.
#define SERIALIZABLE_POD
#endif

#ifdef __clang__
// MACRO to mark a field for serialization.
#define SERIALIZE_FIELD __attribute__((annotate("serialize")))
#else
// MACRO to mark a field for serialization.
#define SERIALIZE_FIELD
#endif

#ifdef __clang__
// MACRO to mark a field for serialization, with a custom name.
#define SERIALIZE_FIELD_AS(name) __attribute__((annotate("serialize:" #name)))
#else
// MACRO to mark a field for serialization, with a custom name.
#define SERIALIZE_FIELD_AS(name)
#endif

#ifdef __clang__
// MACRO to mark a field for exclusion from serialization.
#define SERIALIZE_EXCLUDE __attribute__((annotate("serialize:exclude")))
#else
// MACRO to mark a field for exclusion from serialization.
#define SERIALIZE_EXCLUDE
#endif

#endif // !GENTOOLS_GENSERIALIZE_MACROS_H

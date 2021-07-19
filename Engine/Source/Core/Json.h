#pragma once

#include <EASTL/vector.h>
#include <EASTL/map.h>
#include <EASTL/string.h>

namespace An
{
	struct JsonValue
	{
		enum Type
		{
			Object,
			Array,
			Floating,
			Integer,
			Boolean,
			String,
			Null
		};

		JsonValue();

		JsonValue(const JsonValue& copy);
		JsonValue(JsonValue&& copy);
		JsonValue& operator=(const JsonValue& copy);
		JsonValue& operator=(JsonValue&& copy);

		JsonValue(eastl::vector<JsonValue>& array);
		JsonValue(eastl::map<eastl::string, JsonValue>& object);
		JsonValue(eastl::string string);
		JsonValue(const char* string);
		JsonValue(double number);
		JsonValue(long number);
		JsonValue(bool boolean);

		static JsonValue NewObject();
		static JsonValue NewArray();

		bool IsNull() const;
		bool HasKey(eastl::string identifier) const;
		int Count() const;

		eastl::string ToString() const;
		double ToFloat() const;
		long ToInt() const;
		bool ToBool() const;

		JsonValue& operator[](eastl::string identifier);
		JsonValue& operator[](size_t index);

		const JsonValue& Get(eastl::string identifier) const;
		const JsonValue& Get(size_t index) const;

		void Append(JsonValue& value);

		~JsonValue();

		Type m_type;
		union Data
		{
			eastl::vector<JsonValue>* m_pArray;
			eastl::map<eastl::string, JsonValue>* m_pObject;
			eastl::string* m_pString;
			double m_floatingNumber;
			long m_integerNumber;
			bool m_boolean;
		} m_internalData;
	};

	JsonValue ParseJsonFile(eastl::string& file);
	eastl::string SerializeJsonValue(JsonValue json, eastl::string indentation = "");
}
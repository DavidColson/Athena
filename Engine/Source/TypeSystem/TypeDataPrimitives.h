// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/Json.h"
#include "TypeData.h"

namespace An
{
    // *************************************
	// TYPE DATA ENUM
	// *************************************

	struct Enumerator
	{
		eastl::string m_identifier;
		int m_value;
	};
	struct TypeData_Enum : public TypeData
	{
		/**
		 * Turns an instance of this type into a JsonValue structure for serialization to Json
		 **/
		virtual JsonValue ToJson(TypedPtr var);

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/
		virtual OwnedTypedPtr FromJson(const JsonValue& val);

		eastl::vector<Enumerator> categories;
		
		/**
		 * Constructor used for building typedatas, not for use outside of REFLECT macros
		 **/
		TypeData_Enum(uint32_t _id, const char* _name, size_t _size, TypeDataOps* _pTypeOps, TypeData::CastableTo _castableTo, std::initializer_list<Enumerator> cats);
	};

	/**
	 * Used to define a type as reflectable struct during type declaration 
	 **/
	#define REFLECT_ENUM(EnumType)\
		template<> \
		TypeData& getPrimitiveTypeData<EnumType>();\

	/**
	 * Used to specify the structure of a type, use in cpp files
	 **/
	#define REFLECT_ENUM_BEGIN(EnumType)				\
		template <>										\
		TypeData& getPrimitiveTypeData<EnumType>() {	\
		using XX = EnumType;							\
		static TypeData_Enum selfTypeData(				\
			Type::Index<XX>(),							\
			#EnumType,									\
			sizeof(XX),									\
			new TypeDataOps_Internal<XX>,				\
			TypeData::Enum, {							\

	/**
	 * Used to specify an enumerator inside a REFLECT_ENUM_BEGIN/END pair
	 **/
	#define REFLECT_ENUMERATOR(enumerator)\
				{ #enumerator, (int)XX::enumerator },

	/**
	 * Complete a type structure definition
	 **/
	#define REFLECT_ENUM_END()		\
				});				\
			return selfTypeData;\
		}


    template<typename T>
	TypeData& getPrimitiveTypeData() 
	{
		static TypeData unknownType("UnknownType", 0); 
		return unknownType;
	}

    template <>
	TypeData& getPrimitiveTypeData<int>();
	template <>
	TypeData& getPrimitiveTypeData<float>();
	template <>
	TypeData& getPrimitiveTypeData<double>();
	template <>
	TypeData& getPrimitiveTypeData<eastl::string>();
	template <>
	TypeData& getPrimitiveTypeData<bool>();
}
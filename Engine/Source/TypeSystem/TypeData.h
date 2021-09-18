// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/Json.h"
#include "TypedPtr.h"

namespace An
{
	struct TypeDataOps;
	struct TypeData_Struct;
	struct TypeData_Enum;
	struct TypeData
	{
		/**
		 * Creates a new instance of this type, returning it as a variant
		 **/

		OwnedTypedPtr New();
		
		/**
		 * Turns an instance of this type into a JsonValue structure for serialization to Json
		 **/

		virtual JsonValue ToJson(TypedPtr var) { return JsonValue(); }

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/

		virtual OwnedTypedPtr FromJson(const JsonValue& val) { return OwnedTypedPtr(); }

		/**
		 * Checks for equality with another TypeData
		 **/
		bool operator==(const TypeData& other);

		/**
		 * Checks for inequality with another TypeData
		 **/
		bool operator!=(const TypeData& other);

		/**
		 * Returns false if this is an unknown, invalid typedata
		 **/
		bool IsValid();

		/**
		 * 	Convenience function that casts this typedata to a TypeData_Struct
		 **/
		TypeData_Struct& AsStruct();

		/**
		 * 	Convenience function that casts this typedata to a TypeData_Enum
		 **/
		TypeData_Enum& AsEnum();


		enum CastableTo
		{
			Enum,
			Struct,
			None
		};

		/**
		 * Constructors used for building typedatas, not for use outside of REFLECT macros
		 **/
		TypeData(const char* name, size_t size) : m_name(name), m_size(size) {}
		TypeData(uint32_t id, const char* name, size_t size, TypeDataOps* pTypeOps, CastableTo castableTo) : m_id(id), m_name(name), m_size(size), m_pTypeOps(pTypeOps), m_castableTo(castableTo) {}
		~TypeData();

		uint32_t m_id{ 0 };
		const char* m_name;
		size_t m_size;
		TypeDataOps* m_pTypeOps{ nullptr };
		CastableTo m_castableTo{ CastableTo::None };
	};

    struct TypeDataOps
	{
		virtual OwnedTypedPtr New() = 0;
		virtual void Copy(void* destination, void* pObject) = 0;
		virtual void Move(void* destination, void* pObject) = 0;
		virtual void PlacementNew(void* location) = 0;
		virtual void Destruct(void* pObject) = 0;
		virtual void Free(void* pObject) = 0;
	};

	template<typename T>
	struct TypeDataOps_Internal : public TypeDataOps
	{
		virtual OwnedTypedPtr New() override
		{
			OwnedTypedPtr ptr;
			ptr.m_pData = new T();
			ptr.m_pType = &TypeDatabase::Get<T>();
			return eastl::move(ptr);
		}

		virtual void Copy(void* destination, void* pObject) override
		{
			*reinterpret_cast<T*>(destination) = *reinterpret_cast<T*>(pObject);
		}

		virtual void Move(void* destination, void* pObject) override
		{
			*reinterpret_cast<T*>(destination) = eastl::move(*reinterpret_cast<T*>(pObject));
		}

		virtual void PlacementNew(void* location) override
		{
			new (location) T();
		}

		virtual void Destruct(void* pObject) override
		{
			static_cast<T*>(pObject)->~T();
		}

		virtual void Free(void* pObject) override
		{
			T* pData = static_cast<T*>(pObject);
			delete pData;
		}
	};
}
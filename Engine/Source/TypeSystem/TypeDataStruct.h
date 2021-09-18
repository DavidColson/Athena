// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "TypeData.h"
#include "Core/ErrorHandling.h"

namespace An
{
	struct Member;
	struct TypeData_Struct : public TypeData
	{
		/**
		 * Turns an instance of this type into a JsonValue structure for serialization to Json
		 **/
		virtual JsonValue ToJson(TypedPtr var);

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/
		virtual OwnedTypedPtr FromJson(const JsonValue& val);

		/**
		 * Is this type derived somehow from the given type?
		 **/
		template<typename Type>
		bool IsDerivedFrom();

		/**
		 * Checks for existence of a member by name
		 **/
		bool MemberExists(const char* _name);

		/**
		 * Retrieves reference to a member of this typeData
		 **/
		Member& GetMember(const char* _name);

		/**
		 * Defines a forward iterator on the members of this typedata
		 * 
		 * Use like: for(Member& mem : someTypeData) { ... }
		 * 
		 **/
		struct MemberIterator
		{
			MemberIterator(eastl::map<size_t, Member*>::iterator it) : m_it(it) {}

			Member& operator*() const;

			bool operator==(const MemberIterator& other) const;

			bool operator!=(const MemberIterator& other) const;

			MemberIterator& operator++();

			eastl::map<size_t, Member*>::iterator m_it;
		};

		/**
		 * Iterator to first member
		 **/
		const MemberIterator begin();

		/**
		 * Iterator to last member
		 **/
		const MemberIterator end();
		
		/**
		 * Constructor used for building typedatas, not for use outside of REFLECT macros
		 **/
		TypeData_Struct(void(*initFunc)(TypeData_Struct*)) : TypeData( nullptr, 0 ) { initFunc(this); }

		eastl::map<size_t, Member*> m_members;
		eastl::map<eastl::string, size_t> m_memberOffsets;
		TypeData_Struct* m_pParentType{ nullptr };
	};

	struct Member
	{
		/**
		 * String identifier of this member
		 **/
		const char* GetName();

		/**
		 * Check to determine if this member is of type T
		 **/
		template<typename T>
		bool IsType();

		/**
		 * Returns the typeData for the type of this member
		 **/
		TypeData& GetType();

		/**
		 * Get a ptr to the value of a member from an instance of it's owning type
		 **/
		TypedPtr Get(TypedPtr instance);

		/**
		 * Set the member of instance to newValue
		 **/
		void Set(TypedPtr instance, TypedPtr newValue);

		/**
		 * Set the member of instance to newValue
		 **/
		template <typename T>
		void Set(TypedPtr instance, T newValue);
		
		/**
		 *	Constructor, for internal use in REFLEC_* macros, do not use elsewhere
		**/
		Member(const char* name, size_t offset, TypeData& type) : m_name(name), m_offset(offset), m_pType(&type) {}

	protected:
		const char* m_name;
		size_t m_offset;
		TypeData* m_pType;
	};

	/**
	 * Used to define a type as reflectable struct during type declaration 
	 **/
	#define REFLECT()                               \
		static An::TypeData_Struct staticTypeData;                \
		static void initReflection(An::TypeData_Struct* type); \
		An::TypeData_Struct& GetTypeData();

	/**
	 * Used to define a type as reflectable struct during type declaration 
	 **/
	#define REFLECT_DERIVED()                               \
		static An::TypeData_Struct staticTypeData;                \
		static void initReflection(An::TypeData_Struct* type); \
		virtual An::TypeData_Struct& GetTypeData();

	/**
	 * Used to specify the structure of a type, use in cpp files
	 **/
	#define REFLECT_BEGIN(ReflectedStruct)\
		An::TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		An::TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		void ReflectedStruct::initReflection(An::TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			An::TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->m_id = An::Type::Index<XX>();\
			selfTypeData->m_name = #ReflectedStruct;\
			selfTypeData->m_size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new An::TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = An::TypeData::Struct;\
			selfTypeData->m_members = {

	/**
	 * Used to specify the structure of a type for derived types, again used in cpp files
	 **/
	#define REFLECT_BEGIN_DERIVED(ReflectedStruct, ParentStruct)\
		An::TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		An::TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		void ReflectedStruct::initReflection(An::TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			An::TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->m_id = An::Type::Index<XX>();\
			selfTypeData->m_name = #ReflectedStruct;\
			selfTypeData->m_size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new An::TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = An::TypeData::Struct;\
			selfTypeData->m_pParentType = &ParentStruct::staticTypeData;\
			selfTypeData->m_members = {

	/**
	 * Used to specify a member inside a REFLECT_BEGIN/END pair
	 **/
	#define REFLECT_MEMBER(member)\
				{offsetof(XX, member), new An::Member(#member, offsetof(XX, member), An::TypeDatabase::Get<decltype(XX::member)>())},

	/**
	 * Complete a type structure definition
	 **/
	#define REFLECT_END()\
			};\
			for (const eastl::pair<size_t, An::Member*>& mem : selfTypeData->m_members) { selfTypeData->m_memberOffsets[mem.second->GetName()] = mem.first; }\
		}


	/**
	 *  Special version of the begin macro for types that are template specializations, such as Vec<float>
	 **/
	#define REFLECT_TEMPLATED_BEGIN(ReflectedStruct)\
		An::TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		An::TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		template<>\
		void ReflectedStruct::initReflection(An::TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			An::TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->m_id = An::Type::Index<XX>();\
			selfTypeData->m_name = #ReflectedStruct;\
			selfTypeData->m_size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new An::TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = An::TypeData::Struct;\
			selfTypeData->m_members = {


    // -----------------------------------
	// ---------IMPLEMENTATION------------
	// -----------------------------------


    // ***********************************************************************

    template<typename T>
    bool Member::IsType()
    {
        return *m_pType == TypeDatabase::Get<T>();
    }

    // ***********************************************************************

    template <typename T>
    void Member::Set(TypedPtr instance, T newValue)
    {
        ASSERT(TypeDatabase::Get<T>() == *m_pType, "Trying to set incorrect value type on some member");
        void* ptr = (char*)instance.m_pData + m_offset;
        memcpy(ptr, &newValue, sizeof(T));
    }

    // ***********************************************************************

	template<typename Type>
	bool TypeData_Struct::IsDerivedFrom()
	{
		TypeData& testType = TypeDatabase::Get<Type>();
		if (testType.m_id == this->m_id)
			return true;

		TypeData* pParent = m_pParentType;
		while (pParent != nullptr)
		{
			if (*pParent == testType)
				return true;
			pParent = pParent->AsStruct().m_pParentType;
		}
		return false;
	}
}
// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include <EASTL/string.h>
#include <EASTL/map.h>

#include "Variant.h"
#include "Json.h"
#include "Log.h"


namespace An
{
	// *************************************
	// BASE TYPE DATA
	// *************************************

	struct TypeDataOps;
	struct TypeData_Struct;
	struct TypeData_Enum;
	struct TypeData
	{
		/**
		 * Creates a new instance of this type, returning it as a variant
		 **/

		// return OwnedTypePtr
		Variant New();
		
		/**
		 * Turns an instance of this type into a JsonValue structure for serialization to Json
		 **/

		// param is TypePtr?
		virtual JsonValue ToJson(Variant var) { return JsonValue(); }

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/

		// return OwnedTypePtr
		virtual Variant FromJson(const JsonValue& val) { return Variant(); }

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

		// temp until we have type attributes
		bool m_isComponent{ false };
	};




	// *************************************
	// TYPE DATA STRUCT
	// *************************************

	struct Member;
	struct TypeData_Struct : public TypeData
	{
		/**
		 * Turns an instance of this type into a JsonValue structure for serialization to Json
		 **/
		virtual JsonValue ToJson(Variant var);

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/
		virtual Variant FromJson(const JsonValue& val);

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
		const char* m_name;

		/**
		 * Check to determine if this member is of type T
		 **/
		template<typename T>
		bool IsType();

		/**
		 * Returns the typeData for the type of this member
		 **/
		virtual TypeData& GetType() = 0;

		/**
		 * Get an instance of a member from an instance of it's owning type
		 * Note that the return value will have copied the value into the variant
		 **/
		// return typedPtr, param typedPtr so the return value directly points to the real data in the instance.
		virtual Variant Get(Variant& instance) = 0;

		/**
		 * Set the member of instance to newValue
		 **/
		// Takes TypedPtr instance and typedPtr value (it will be copied into it's new location I think, could be const param then?)
		virtual void Set(Variant& instance, Variant newValue) = 0;

		// TODO: Don't love what's happening here, seems like it's prone to error. Think of something else
		// These should not be necessary when TypedPtrs are working

		/**
		 * Get an instance of a member from a pointer to a base class of it's owning type
		 * Note that the return value will have copied the value into the variant
		 **/
		virtual Variant GetFromBase(void* pInstance) = 0;

		/**
		 * Set the member of base class instance to newValue
		 **/
		virtual void SetOnBase(void* pInstance, Variant value) = 0;
		
		/**
		 *	Constructor, for internal use in REFLEC_* macros, do not use elsewhere
		**/
		Member(const char* name) : m_name(name) {}
	};

	/**
	 * Used to define a type as reflectable struct during type declaration 
	 **/
	#define REFLECT()                               \
		static TypeData_Struct staticTypeData;                \
		static void initReflection(TypeData_Struct* type); \
		TypeData_Struct& GetTypeData();

	/**
	 * Used to define a type as reflectable struct during type declaration 
	 **/
	#define REFLECT_DERIVED()                               \
		static TypeData_Struct staticTypeData;                \
		static void initReflection(TypeData_Struct* type); \
		virtual TypeData_Struct& GetTypeData();

	/**
	 * Used to specify the structure of a type, use in cpp files
	 **/
	#define REFLECT_BEGIN(ReflectedStruct)\
		TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		void ReflectedStruct::initReflection(TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->id = Type::Index<XX>();\
			selfTypeData->name = #ReflectedStruct;\
			selfTypeData->size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = TypeData::Struct;\
			selfTypeData->m_members = {

	/**
	 * Used to specify the structure of a type for derived types, again used in cpp files
	 **/
	#define REFLECT_BEGIN_DERIVED(ReflectedStruct, ParentStruct)\
		TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		void ReflectedStruct::initReflection(TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->id = Type::Index<XX>();\
			selfTypeData->name = #ReflectedStruct;\
			selfTypeData->size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = TypeData::Struct;\
			selfTypeData->m_pParentType = &ParentStruct::staticTypeData;\
			selfTypeData->m_members = {

	/**
	 * Used to specify a member inside a REFLECT_BEGIN/END pair
	 **/
	#define REFLECT_MEMBER(member)\
				{offsetof(XX, member), new Member_Internal<decltype(XX::member), XX>(#member, &XX::member)},

	/**
	 * Complete a type structure definition
	 **/
	#define REFLECT_END()\
			};\
			for (const eastl::pair<size_t, Member*>& mem : selfTypeData->m_members) { selfTypeData->m_memberOffsets[mem.second->name] = mem.first; }\
		}


	/**
	 *  Special version of the begin macro for types that are template specializations, such as Vec<float>
	 **/
	#define REFLECT_TEMPLATED_BEGIN(ReflectedStruct)\
		TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		template<>\
		void ReflectedStruct::initReflection(TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->id = Type::Index<XX>();\
			selfTypeData->name = #ReflectedStruct;\
			selfTypeData->size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = TypeData::Struct;\
			selfTypeData->m_members = {
	
	// TODO: This should not be needed anymore because we can now ask IsDerivedFrom<IComponent>
	/**
	 * Special version of reflection function used for components. Just stores a member that tells you this type is a component
	 * Intend to replace with proper type attributes at some point
	 **/
	#define REFLECT_COMPONENT_BEGIN(ReflectedStruct)\
		TypeData_Struct ReflectedStruct::staticTypeData{ReflectedStruct::initReflection};\
		TypeData_Struct& ReflectedStruct::GetTypeData() { return ReflectedStruct::staticTypeData; }\
		void ReflectedStruct::initReflection(TypeData_Struct* selfTypeData) {\
			using XX = ReflectedStruct;\
			TypeDatabase::Data::Get().typeNames.emplace(#ReflectedStruct, selfTypeData);\
			selfTypeData->id = Type::Index<XX>();\
			selfTypeData->m_isComponent = true;\
			selfTypeData->name = #ReflectedStruct;\
			selfTypeData->size = sizeof(XX);\
			selfTypeData->m_pTypeOps = new TypeDataOps_Internal<XX>;\
			selfTypeData->m_castableTo = TypeData::Struct;\
			selfTypeData->m_members = {





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
		virtual JsonValue ToJson(Variant var);

		/**
		 * Converts a Json structure into an instance of this type, returned as a variant
		 **/
		virtual Variant FromJson(const JsonValue& val);

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






	namespace TypeDatabase
	{
		/**
		 * Check for the existence of a type in the database by string name
		 **/
		bool TypeExists(const char* name);

		/**
		 * Retrieve a reference to a typeData from a string
		 **/
		TypeData& GetFromString(const char* name);

		/**
		 * Similar to above, checks for existing of Type T in the database
		 **/
		template<typename T>
		bool TypeExists();

		/**
		 *  Similiar to GetFromString, retrieves a reference to a TypeData of type T
		 **/
		template<typename T>
		TypeData& Get();
	};

	/**
	 * Type Index retrieval
	 * Note that these indexes are not stable between runs
	 * Use like Type::Index<T>()
	 **/
	struct Type {
		template<typename Type>
		inline static uint32_t Index()
		{
			static uint32_t typeIndex = TypeDatabase::Data::Get().typeCounter++;
			return typeIndex;
		}
	};



	// -----------------------------------
	// ------------INTERNAL---------------
	// -----------------------------------

	// For storing lookup tables and other static type information
	namespace TypeDatabase
	{
		struct Data
		{
			static Data& Get() 
			{
				if (m_pInstance == nullptr)
					m_pInstance = new Data();
				return *m_pInstance;
			}
			eastl::map<eastl::string, TypeData*> typeNames;
			uint32_t typeCounter{ 0 };

		private:
			static Data* m_pInstance;
		};

		template<typename T>
		bool TypeExists()
		{
			if (DefaultTypeResolver::Get<T>() == TypeData("UnknownType", 0))
				return false;
			return true;
		}

		// Generic typedata return
		template<typename T>
		TypeData& Get()
		{
			return DefaultTypeResolver::Get<T>();
		}
	}

	// Type resolving mechanism
	template<typename T>
	TypeData& getPrimitiveTypeData() 
	{
		static TypeData unknownType("UnknownType", 0); 
		return unknownType;
	}

	// @TODO: Move to BaseTypes.h/cpp that will include parsing for base types
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

	struct EntityID;
	template <>
	TypeData& getPrimitiveTypeData<EntityID>();
	struct AssetHandle;
	template <>
	TypeData& getPrimitiveTypeData<AssetHandle>();

	struct DefaultTypeResolver 
	{
		// the decltype term here may result in invalid c++ if T doesn't contain typeData. 
		// As such if it doesn't, this template instantiation will be completely ignored by the compiler through SFINAE. 
		// If it does contain typeData, this will be instantiated and we're all good.
		// See here for deep explanation: https://stackoverflow.com/questions/1005476/how-to-detect-whether-there-is-a-specific-member-variable-in-class
		template<typename T, typename = int>
		struct IsReflected : eastl::false_type {};
		template<typename T>
		struct IsReflected<T, decltype((void) T::staticTypeData, 0)> : eastl::true_type {};

		// We're switching template versions depending on whether T has been internally reflected (i.e. has an typeData member)
		template<typename T, eastl::enable_if_t<IsReflected<T>::value, int> = 0>
		static TypeData& Get()
		{
			return T::staticTypeData;
		}
		template<typename T, eastl::enable_if_t<!IsReflected<T>::value, int> = 0>
		static TypeData& Get()
		{
			return getPrimitiveTypeData<T>();
		}
	};

	template<typename T>
	bool Member::IsType()
	{
		// This returns true if you ask if the item is an unknown type, which is real bad
		return GetType() == TypeDatabase::Get<T>(); 
	}

	template<typename MemberType, typename ClassType>
	struct Member_Internal : public Member
	{
		Member_Internal(const char* _name, MemberType ClassType::*pointer) : Member(_name), memberPointer(pointer) {}

		virtual TypeData& GetType() override
		{
			return TypeDatabase::Get<MemberType>();
		}

		virtual Variant Get(Variant& instance) override
		{
			return Variant(instance.GetValue<ClassType>().*memberPointer);
		}

		virtual Variant GetFromBase(void* pInstance) override
		{
			return Variant(static_cast<ClassType*>(pInstance)->*memberPointer);
		}

		// Copy happening on value, consider replacing with argument wrapper
		virtual void Set(Variant& instance, Variant value) override
		{
			instance.GetValue<ClassType>().*memberPointer = value.GetValue<MemberType>();
		}

		// Copy happening on value, consider replacing with argument wrapper
		virtual void SetOnBase(void* pInstance, Variant value) override
		{
			static_cast<ClassType*>(pInstance)->*memberPointer = value.GetValue<MemberType>();
		}

		MemberType ClassType::* memberPointer;
	};

	struct TypeDataOps
	{
		virtual Variant New() = 0;
		virtual Variant CopyToVariant(void* pObject) = 0;
		virtual void Copy(void* destination, void* pObject) = 0;
		virtual void PlacementNew(void* location) = 0;
		virtual void Destruct(void* pObject) = 0;
		virtual void Free(void* pObject) = 0;
	};

	template<typename T>
	struct TypeDataOps_Internal : public TypeDataOps
	{
		virtual Variant New() override
		{
			// Return OwnedTypePtr
			return Variant(T());
		}

		virtual Variant CopyToVariant(void* pObject) override
		{
			// Return OwnedTypePtr
			return Variant(*reinterpret_cast<T*>(pObject));
		}

		virtual void Copy(void* destination, void* pObject) override
		{
			*reinterpret_cast<T*>(destination) = *reinterpret_cast<T*>(pObject);
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
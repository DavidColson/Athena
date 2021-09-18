// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include <EASTL/string.h>
#include <EASTL/map.h>

#include "Core/ErrorHandling.h"
#include "TypeData.h"
#include "TypeDataStruct.h"
#include "TypeDataPrimitives.h"

namespace An
{
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
}
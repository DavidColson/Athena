// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once

#include "Core/ErrorHandling.h"

namespace An
{
    struct TypeData;
    
    // Just a reference to some data stored by an OwnedTypePtr
    struct TypedPtr
    {
        friend struct Member;

        TypedPtr(void* ptr, TypeData* pTypeData);
        ~TypedPtr();

        template <typename T>
        T* Cast();

        template <typename T>
        T* SafeCast();

        template <typename T>
        T& CastRef();

        TypeData& GetType();

    private:
        void* m_pData{ nullptr };
        TypeData* m_pType;
    };

    // Owns data for real, will delete it when the ptr gets deleted
    struct OwnedTypedPtr
    {
        template <typename T>
        friend struct TypeDataOps_Internal;

        OwnedTypedPtr();
        ~OwnedTypedPtr();

        OwnedTypedPtr(const OwnedTypedPtr& copy);
		OwnedTypedPtr(OwnedTypedPtr&& move);

        TypedPtr Ref();

        template <typename T>
        T* Cast();

        template <typename T>
        T* SafeCast();

        template <typename T>
        T& CastRef();

        TypeData& GetType();

        template <typename T, typename... Args>
        static OwnedTypedPtr New(Args... args);

    private:
        void* m_pData{ nullptr };
        TypeData* m_pType{ nullptr };
    };


    // -----------------------------------
	// ---------IMPLEMENTATION------------
	// -----------------------------------

    
	// ***********************************************************************

	template <typename T>
	T* TypedPtr::Cast()
	{
		ASSERT(*m_pType == TypeDatabase::Get<T>(), "Attempting to cast to wrong type");
		return static_cast<T*>(m_pData);
	}

	// ***********************************************************************

	template <typename T>
	T* TypedPtr::SafeCast()
	{
		return (m_pType == TypeDatabase::Get<T>()) ? static_cast<T*>(m_pData) : nullptr;
	}

	// ***********************************************************************

	template <typename T>
	T& TypedPtr::CastRef()
	{
		ASSERT(*m_pType == TypeDatabase::Get<T>(), "Attempting to cast to wrong type");
		return *static_cast<T*>(m_pData);
	}

	// ***********************************************************************

	template <typename T>
	T* OwnedTypedPtr::Cast()
	{
		ASSERT(*m_pType == TypeDatabase::Get<T>(), "Attempting to cast to wrong type");
		return static_cast<T*>(m_pData);
	}

	// ***********************************************************************

	template <typename T>
	T* OwnedTypedPtr::SafeCast()
	{
		return (m_pType == TypeDatabase::Get<T>()) ? static_cast<T*>(m_pData) : nullptr;
	}

	// ***********************************************************************

	template <typename T>
	T& OwnedTypedPtr::CastRef()
	{
		ASSERT(*m_pType == TypeDatabase::Get<T>(), "Attempting to cast to wrong type");
		return *static_cast<T*>(m_pData);
	}

    template <typename T, typename... Args>
    static OwnedTypedPtr OwnedTypedPtr::New(Args... args)
    {
        OwnedTypedPtr res;
        res.m_pData = new T{eastl::forward<Args>(args)...};
        res.m_pType = &TypeDatabase::Get<T>();
        return eastl::move(res);
    }
}
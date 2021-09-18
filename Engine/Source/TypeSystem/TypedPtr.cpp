// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypedPtr.h"

#include "TypeSystem.h"

namespace An
{

	// ***********************************************************************

    TypedPtr::TypedPtr(void* ptr, TypeData* pTypeData)
    {
        m_pData = ptr;
        m_pType = pTypeData;
    }
    
	// ***********************************************************************

    TypedPtr::~TypedPtr()
    {
        m_pData = nullptr;
        m_pType = nullptr;
    }

    TypeData& TypedPtr::GetType()
    {
        return *m_pType;
    }


	// ***********************************************************************

    OwnedTypedPtr::OwnedTypedPtr()
    {
        m_pData = nullptr;
        m_pType = nullptr; 
    }

	// ***********************************************************************

    OwnedTypedPtr::~OwnedTypedPtr()
    {
        if (m_pData)
        {
            m_pType->m_pTypeOps->Free(m_pData);
            m_pType = nullptr;
        }
    }

	// ***********************************************************************

    OwnedTypedPtr::OwnedTypedPtr(const OwnedTypedPtr& copy)
    {
        // Copy
        copy.m_pType->m_pTypeOps->Copy(m_pData, copy.m_pData);
        m_pType = copy.m_pType;
    }

	// ***********************************************************************

    OwnedTypedPtr::OwnedTypedPtr(OwnedTypedPtr&& move)
    {
        // Move
        m_pType = move.m_pType;
        m_pData = move.m_pData;

        move.m_pData = nullptr;
        move.m_pType = nullptr;
    }

	// ***********************************************************************

    TypedPtr OwnedTypedPtr::Ref()
    {
        return TypedPtr{m_pData, m_pType};
    }

    TypeData& OwnedTypedPtr::GetType()
    {
        return *m_pType;
    }

}
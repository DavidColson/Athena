// Copyright 2020-2021 David Colson. All rights reserved.

#pragma once


namespace An
{
    struct TypeData;
    class OwnedTypedPtr;
    
    // Just a reference to some data stored by an OwnedTypePtr
    class TypedPtr
    {
        friend struct Member;
    public:
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
    class OwnedTypedPtr
    {
        template <typename T>
        friend struct TypeDataOps_Internal;
    public:
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
        static OwnedTypedPtr New(Args... args)
        {
            OwnedTypedPtr res;
            res.m_pData = new T{eastl::forward<Args>(args)...};
            res.m_pType = &TypeDatabase::Get<T>();
            return eastl::move(res);
        }

    private:
        void* m_pData{ nullptr };
        TypeData* m_pType{ nullptr };
    };

}
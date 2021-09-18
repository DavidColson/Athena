// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypeDataStruct.h"

namespace An
{
    // ***********************************************************************

	JsonValue TypeData_Struct::ToJson(TypedPtr value)
	{
		JsonValue result = JsonValue::NewObject();

		// TODO: Order of members is lost using this method. We have our member offsets, see if we can use it somehow
		for (Member& member : value.GetType().AsStruct())
		{
			result[member.GetName()] = member.GetType().ToJson(member.Get(value));
		}

		return result;
	}

	// ***********************************************************************

	OwnedTypedPtr TypeData_Struct::FromJson(const JsonValue& json)
	{
		OwnedTypedPtr var = New();

		for (const eastl::pair<eastl::string, JsonValue>& val : *json.m_internalData.m_pObject)
		{
			if (MemberExists(val.first.c_str()))
			{
				Member& mem = GetMember(val.first.c_str());

				OwnedTypedPtr parsed = mem.GetType().FromJson(val.second);
				mem.Set(var.Ref(), parsed.Ref());   
			}
		}

		return var;
	}

	// ***********************************************************************

	bool TypeData_Struct::MemberExists(const char* _name)
	{
		return m_memberOffsets.count(_name) == 1;
	}

	// ***********************************************************************

	Member& TypeData_Struct::GetMember(const char* _name)
	{
		ASSERT(m_memberOffsets.count(_name) == 1, "The member you're trying to access doesn't exist");
		return *m_members[m_memberOffsets[_name]];
	}

	// ***********************************************************************

	Member& TypeData_Struct::MemberIterator::operator*() const 
	{ 
		return *m_it->second;
	}

	// ***********************************************************************

	bool TypeData_Struct::MemberIterator::operator==(const MemberIterator& other) const 
	{
		return m_it == other.m_it;
	}

	// ***********************************************************************

	bool TypeData_Struct::MemberIterator::operator!=(const MemberIterator& other) const 
	{
		return m_it != other.m_it;
	}

	// ***********************************************************************

	TypeData_Struct::MemberIterator& TypeData_Struct::MemberIterator::operator++()
	{
		++m_it;
		return *this;
	}

	// ***********************************************************************

	const TypeData_Struct::MemberIterator TypeData_Struct::begin() 
	{
		return MemberIterator(m_members.begin());
	}

	// ***********************************************************************

	const TypeData_Struct::MemberIterator TypeData_Struct::end()
	{
		return MemberIterator(m_members.end());
	}

	// ***********************************************************************

    const char* Member::GetName()
    {
        return m_name;
    }

	// ***********************************************************************

    TypeData& Member::GetType()
    {
        return *m_pType;
    }

	// ***********************************************************************

    TypedPtr Member::Get(TypedPtr instance)
    {
        return TypedPtr((char*)instance.m_pData + m_offset, &GetType());
    }

	// ***********************************************************************

    void Member::Set(TypedPtr instance, TypedPtr newValue)
    {
        ASSERT(*newValue.m_pType == *m_pType, "Trying to set incorrect value type on some member");
        void* ptr = (char*)instance.m_pData + m_offset;
        memcpy(ptr, newValue.m_pData, m_pType->m_size);
    }
}
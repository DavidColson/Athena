// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypeData.h"

#include "TypeDataStruct.h"
#include "TypeDataPrimitives.h"

namespace An
{
	// ***********************************************************************

	TypeData::~TypeData()
	{
		delete m_pTypeOps;

		// TODO: For loop over members and delete them
	}

	// ***********************************************************************

	OwnedTypedPtr TypeData::New()
	{
		return m_pTypeOps->New();
	}

	// ***********************************************************************

	bool TypeData::operator==(const TypeData& other)
	{
		return other.m_id == this->m_id;
	}

	// ***********************************************************************

	bool TypeData::operator!=(const TypeData& other)
	{
		return other.m_id != this->m_id;
	}

	// ***********************************************************************

	bool TypeData::IsValid()
	{
		return m_id != 0;
	}

	// ***********************************************************************

	TypeData_Struct& TypeData::AsStruct()
	{
		return *static_cast<TypeData_Struct*>(this);
	}

	// ***********************************************************************

	TypeData_Enum& TypeData::AsEnum()
	{
		return *static_cast<TypeData_Enum*>(this);
	}

}
// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypeDatabase.h"

namespace An
{
	namespace TypeDatabase
	{
		Data* TypeDatabase::Data::m_pInstance{ nullptr };

		bool TypeExists(const char* name)
		{
			if (Data::Get().typeNames.count(name) == 1)
				return true;
			else
				return false;
		}

		TypeData& GetFromString(const char* name)
		{
			return *Data::Get().typeNames[name];
		}
	}
}
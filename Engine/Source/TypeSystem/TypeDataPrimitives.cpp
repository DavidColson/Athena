// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypeDataPrimitives.h"

#include "Core/Log.h"
#include <EASTL/string.h>
#include "TypeDatabase.h"

namespace An
{
    // ***********************************************************************

	TypeData_Enum::TypeData_Enum(uint32_t _id, const char* _name, size_t _size, TypeDataOps* _pTypeOps, TypeData::CastableTo _castableTo, std::initializer_list<Enumerator> cats) : TypeData(_id, _name, _size, _pTypeOps, _castableTo), categories(cats)
	{
		TypeDatabase::Data::Get().typeNames.emplace(m_name, static_cast<TypeData*>(this));
	}

	// ***********************************************************************

	JsonValue TypeData_Enum::ToJson(TypedPtr var)
	{
		// This will perform a reinterpret cast to int, probably enforce enums to be ints somehow
		return JsonValue(categories[var.CastRef<int>()].m_identifier);
	}

	// ***********************************************************************

	OwnedTypedPtr TypeData_Enum::FromJson(const JsonValue& val)
	{

		eastl::vector<Enumerator>::iterator it = eastl::find_if(categories.begin(), categories.end(), [&val](Enumerator& enumerator) 
		{
			return enumerator.m_identifier == val.ToString();
		});

		if (it == categories.end())
			Log::Crit("Attempting to create an enum from an invalid jsonValue: %s", val.ToString().c_str());

		return OwnedTypedPtr::New<int>((int)eastl::distance(categories.begin(), it));
	}





	// ***********************************************************************

	struct TypeData_Int : TypeData
	{
		TypeData_Int() : TypeData{"int", sizeof(int)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("int", this);
			m_id = Type::Index<int>();
			m_pTypeOps = new TypeDataOps_Internal<int>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{
			return JsonValue((long)var.CastRef<int>());
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<int>((int)val.ToInt());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<int>()
	{
		static TypeData_Int typeData;
		return typeData;
	}





	// ***********************************************************************

	struct TypeData_Float : TypeData
	{
		TypeData_Float() : TypeData{"float", sizeof(float)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("float", this);
			m_id = Type::Index<float>();
			m_pTypeOps = new TypeDataOps_Internal<float>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{
			return JsonValue((double)var.CastRef<float>());
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<float>((float)val.ToFloat());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<float>()
	{
		static TypeData_Float typeData;
		return typeData;
	}





	// ***********************************************************************

	struct TypeData_Double : TypeData
	{
		TypeData_Double() : TypeData{"double", sizeof(double)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("double", this);
			m_id = Type::Index<double>();
			m_pTypeOps = new TypeDataOps_Internal<double>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{
			return JsonValue(var.CastRef<double>());
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<double>(val.ToFloat());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<double>()
	{
		static TypeData_Double typeData;
		return typeData;
	}





	// ***********************************************************************

	struct TypeData_String : TypeData
	{
		TypeData_String() : TypeData{"eastl::string", sizeof(eastl::string)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("eastl::string", this);
			m_id = Type::Index<eastl::string>();
			m_pTypeOps = new TypeDataOps_Internal<eastl::string>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{	
			return JsonValue(var.CastRef<eastl::string>());
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<eastl::string>(val.ToString());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<eastl::string>()
	{
		static TypeData_String typeData;
		return typeData;
	}





	// ***********************************************************************

	struct TypeData_Bool : TypeData
	{
		TypeData_Bool() : TypeData{"bool", sizeof(bool)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("bool", this);
			m_id = Type::Index<bool>();
			m_pTypeOps = new TypeDataOps_Internal<bool>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{
			return JsonValue(var.CastRef<bool>());
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<bool>(val.ToBool());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<bool>()
	{
		static TypeData_Bool typeData;
		return typeData;
	}

}
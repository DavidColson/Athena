// Copyright 2020-2021 David Colson. All rights reserved.

#include "TypeSystem.h"
#include "Core/ErrorHandling.h"

// @ I'd like these custom types to be taken outside of the core of the type system
#include "AssetDatabase/AssetDatabase.h"
#include "Core/Scanning.h"

#include <EASTL/string.h>


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

	// Primitive Types
	//////////////////


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







	// ***********************************************************************

	struct TypeData_AssetHandle: TypeData
	{
		TypeData_AssetHandle() : TypeData{"AssetHandle", sizeof(AssetHandle)} 
		{
			TypeDatabase::Data::Get().typeNames.emplace("AssetHandle", this);
			m_id = Type::Index<AssetHandle>();
			m_pTypeOps = new TypeDataOps_Internal<AssetHandle>();
		}

		virtual JsonValue ToJson(TypedPtr var) override
		{
			JsonValue val = JsonValue::NewObject();
			AssetHandle handle = var.CastRef<AssetHandle>();
			val["Asset"] = AssetDB::GetAssetIdentifier(handle);
			return val;
		}

		virtual OwnedTypedPtr FromJson(const JsonValue& val) override
		{
			return OwnedTypedPtr::New<AssetHandle>(val.Get("Asset").ToString());
		}
	};
	template <>
	TypeData& getPrimitiveTypeData<AssetHandle>()
	{
		static TypeData_AssetHandle typeData;
		return typeData;
	}
}
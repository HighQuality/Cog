#include "CogPch.h"
#include "TypeList.h"

// TODO: Expose through static getter instead of global variable
const TypeList gTypeList;

// The definition of this should be generated by the build tool
void RegisterExecutableTypes_Generated(TypeList* aTypeList);

TypeList::TypeList()
{
	BuildList();
}

TypeList::~TypeList()
{
}

void TypeList::BuildList()
{
	RegisterExecutableTypes_Generated(this);

	for (TypeData* type : myIDToData)
	{
		if (!type)
			continue;

		if (type->GetBaseName() == L"CogTypeBase")
			continue;

		TypeData& baseType = GetTypeData(type->GetBaseName(), false);
		type->myBaseType = &baseType;

		if (type->IsSpecializingBaseType())
		{
			if (baseType.mySpecializedBy)
				FATAL(L"Both % and % specializes %", baseType.mySpecializedBy->GetName(), type->GetName());

			baseType.mySpecializedBy = type;
		}
	}

	for (TypeData* type : myIDToData)
	{
		if (!type)
			continue;

		if (TypeData* base = type->myBaseType)
			base->mySubtypes.Add(type);
	}

	for (TypeData* type : myIDToData)
	{
		if (!type)
			continue;

		if (type->IsSpecializingBaseType() && !type->IsSpecialized())
		{
			TypeData* current = type->myBaseType;
			
			do
			{
				current->mySpecializedBy = type;
				current = current->myBaseType;
			} while (current && current->IsSpecializingBaseType());
		}
	}

	for (TypeData* type : myIDToData)
	{
		if (!type)
			continue;

		if (type->IsSingleton())
		{
			myAllSingletons.Add(type);

			if (!type->IsSpecialized())
				myLeafSingletons.Add(type);
		}
		else
		{
			myAllObjectTypes.Add(type);

			if (!type->IsSpecialized())
				myLeafObjectTypes.Add(type);
		}
	}
}

const TypeData& TypeList::GetTypeDataByIndex(const u16 aTypeIndex, const bool aOutermost) const
{
	CHECK_MSG(myIDToData.IsValidIndex(aTypeIndex), L"Type index out of range");

	const TypeData* data = myIDToData[aTypeIndex];

	if (!data)
		FATAL(L"This type has not been registered");

	return aOutermost && data->IsSpecialized() ? *data->mySpecializedBy : *data;
}

TypeData& TypeList::GetTypeDataByIndex(const u16 aTypeIndex, const bool aOutermost)
{
	CHECK_MSG(myIDToData.IsValidIndex(aTypeIndex), L"Type index out of range");

	TypeData* data = myIDToData[aTypeIndex];

	if (!data)
		FATAL(L"This type has not been registered");

	return aOutermost && data->IsSpecialized() ? *data->mySpecializedBy : *data;
}

TypeData& TypeList::Internal_AddGenericType(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, const StringView& aBaseTypeName, const bool aIsSpecialization)
{
	CHECK(aTypeName.GetLength() > 0);

	const u16 typeIndex = aTypeID.GetUnderlyingInteger();
	myTypeNameToID.Add(aTypeName, typeIndex);

	if (typeIndex >= myIDToData.GetLength())
		myIDToData.Resize(typeIndex + 1);

	myIDToData[typeIndex] = MakeUnique<TypeData>();
	TypeData& data = *myIDToData[typeIndex];

	if (data.GetName().GetLength() > 0)
		FATAL("Double registration on type ", aTypeName);

	data.myTypeID = &aTypeID;
	data.myName = String(aTypeName);
	data.myBaseName = String(aBaseTypeName);
	data.myIsSpecialization = aIsSpecialization;

	return data;
}

TypeData& TypeList::Internal_AddType(const TypeID<CogTypeBase>& aTypeID, StringView aTypeName, StringView aBaseTypeName, const bool aIsSpecialization, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t)
{
	TypeData& data = Internal_AddGenericType(aTypeID, aTypeName, aBaseTypeName, aIsSpecialization);
	data.myFactoryAllocator = aFactoryAllocator;
	return data;
}

TypeData& TypeList::Internal_AddSingleton(const TypeID<CogTypeBase>& aTypeID, StringView aTypeName, StringView aBaseTypeName, const bool aIsSpecialization, UniquePtr<Singleton>(*aSingletonAllocator)(), nullptr_t)
{
	TypeData& data = Internal_AddGenericType(aTypeID, aTypeName, aBaseTypeName, aIsSpecialization);
	data.mySingletonAllocator = aSingletonAllocator;
	return data;
}

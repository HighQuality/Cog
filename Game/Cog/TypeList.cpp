#include "pch.h"
#include "TypeList.h"

void TypeList::RegisterTypes()
{
}

void TypeList::BuildList()
{
	RegisterTypes();

	// TODO: Rewrite after fixing map/array problem
	Array<u16> components;

	for (const auto& pair : myIDToData)
		components.Add(pair.key);

	for (const u16 id : components)
	{
		TypeData& component = myIDToData[id];

		const StringView& specializationOf = component.GetSpecializationOf();

		if (specializationOf.GetLength() > 0)
		{
			if (const u16* specializationOfID = myTypeNameToID.Find(specializationOf))
			{
				TypeData* base = myIDToData.Find(*specializationOfID);
				CHECK(base);
				CHECK(!base->GetSpecialization());
				base->SetSpecialization(component);
			}
			else
			{
				FATAL(L"Component % specializes an unknown component: %", component.GetName(), specializationOf);
			}
		}
	}
}

TypeData& TypeList::Internal_AddType(const u16 aTypeID, const StringView& aTypeName, BaseFactory*(*aFactoryAllocator)(), nullptr_t)
{
	CHECK(aTypeName.GetLength() > 0);

	u16& id = myTypeNameToID[aTypeName];
	CHECK(id == 0);
	id = aTypeID;

	TypeData& data = myIDToData[aTypeID];

	if (data.GetName().GetLength() > 0)
		FATAL("Double registration on type ", aTypeName);

	data.SetName(aTypeName);
	data.SetFactoryAllocator(aFactoryAllocator);

	return data;
}

void TypeList::Internal_AddSpecialization(const StringView& aBaseName, const u16 aTypeID, const StringView& aSpecializationName, BaseFactory*(*aFactoryAllocator)(), nullptr_t)
{
	TypeData& componentData = Internal_AddType(aTypeID, aSpecializationName, aFactoryAllocator, nullptr);
	componentData.SetSpecializationOf(aBaseName);
}

#include "GamePch.h"
#include "TypeList.h"

void TypeList::RegisterTypes()
{
	Internal_AddType(TypeID<Object>::Resolve<Object>().GetUnderlyingInteger(), L"Object", &CreateObjectFactory<Object>, nullptr);
}

void TypeList::BuildList()
{
	RegisterTypes();

	for (auto& pair : myIDToData)
	{
		TypeData& type = pair.value;

		const StringView& specializationOf = type.GetSpecializationOf();

		if (specializationOf.GetLength() > 0)
		{
			if (const u16* specializationOfID = myTypeNameToID.Find(specializationOf))
			{
				TypeData* base = myIDToData.Find(*specializationOfID);
				CHECK(base);
				CHECK(!base->GetSpecialization());
				base->SetSpecialization(type);
			}
			else
			{
				FATAL(L"Component % specializes an unknown component: %", type.GetName(), specializationOf);
			}
		}
	}

	for (auto& pair : myIDToData)
	{
		if (!pair.value.HasOutermostSpecialization())
			pair.value.AssignOutermostSpecialization();
	}
}

TypeData& TypeList::Internal_AddType(const u16 aTypeID, const StringView& aTypeName, UniquePtr<BaseFactory>(*aFactoryAllocator)(), nullptr_t)
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

void TypeList::Internal_AddSpecialization(const StringView& aBaseName, const u16 aTypeID, const StringView& aSpecializationName, UniquePtr<BaseFactory>(*aFactoryAllocator)(), nullptr_t)
{
	TypeData& componentData = Internal_AddType(aTypeID, aSpecializationName, aFactoryAllocator, nullptr);
	componentData.SetSpecializationOf(aBaseName);
}

#include "CogPch.h"
#include "TypeList.h"

void RegisterExecutableTypes_Generated(TypeList* aTypeList);

void TypeList::BuildList()
{
	Internal_AddType(TypeID<Object>::Resolve<Object>().GetUnderlyingInteger(), L"Object", &CreateObjectChunk<Object, ObjectCogTypeChunk>, nullptr);
	
	RegisterExecutableTypes_Generated(this);

	for (auto& pair : myIDToData)
	{
		TypeData& type = *pair.value;

		const StringView& specializationOf = type.GetSpecializationOf();

		if (specializationOf.GetLength() > 0)
		{
			if (const u16* specializationOfID = myTypeNameToID.Find(specializationOf))
			{
				UniquePtr<TypeData>* base = myIDToData.Find(*specializationOfID);
				CHECK(base);
				CHECK(base->IsValid());
				CHECK(!base->Get()->GetSpecialization());
				base->Get()->SetSpecialization(type);
			}
			else
			{
				FATAL(L"Component % specializes an unknown component: %", type.GetName(), specializationOf);
			}
		}
	}

	for (auto& pair : myIDToData)
	{
		if (!pair.value->HasOutermostSpecialization())
			pair.value->AssignOutermostSpecialization();
	}
}

TypeData& TypeList::Internal_AddType(const u16 aTypeID, const StringView& aTypeName, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t)
{
	CHECK(aTypeName.GetLength() > 0);

	myTypeNameToID.Add(aTypeName, aTypeID);

	TypeData& data = *myIDToData.Add(aTypeID, MakeUnique<TypeData>());

	if (data.GetName().GetLength() > 0)
		FATAL("Double registration on type ", aTypeName);

	data.SetName(aTypeName);
	data.SetFactoryAllocator(aFactoryAllocator);

	return data;
}

void TypeList::Internal_AddSpecialization(const StringView& aBaseName, const u16 aTypeID, const StringView& aSpecializationName, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t)
{
	TypeData& componentData = Internal_AddType(aTypeID, aSpecializationName, aFactoryAllocator, nullptr);
	componentData.SetSpecializationOf(aBaseName);
}

#include "pch.h"
#include "ComponentList.h"
#include "ComponentFactory.h"

void ComponentList::RegisterComponents()
{
}

void ComponentList::BuildList()
{
	RegisterComponents();

	// TODO: Rewrite after fixing map/array problem
	Array<u16> components;

	for (const auto& pair : myIDToData)
		components.Add(pair.key);

	for (const u16 id : components)
	{
		ComponentData& component = myIDToData[id];

		StringView specializationOf = component.GetSpecializationOf();

		if (specializationOf.GetLength() > 0)
		{
			if (const u16* id = myTypeNameToID.Find(specializationOf))
			{
				ComponentData* base = myIDToData.Find(*id);
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

ComponentData& ComponentList::Internal_AddComponent(const u16 aComponentID, const StringView& aComponentName, BaseComponentFactory*(*aFactoryAllocator)(), nullptr_t)
{
	CHECK(aComponentName.GetLength() > 0);

	u16& id = myTypeNameToID[aComponentName];
	CHECK(id == 0);
	id = aComponentID;

	ComponentData& data = myIDToData[aComponentID];

	if (data.GetName().GetLength() > 0)
		FATAL("Double registration on type ", aComponentName);

	data.SetName(aComponentName);
	data.SetFactoryAllocator(aFactoryAllocator);

	return data;
}

void ComponentList::Internal_AddSpecialization(const StringView& aBaseName, const u16 aComponentID, const StringView& aSpecializationName, BaseComponentFactory*(*aFactoryAllocator)(), nullptr_t)
{
	ComponentData& componentData = Internal_AddComponent(aComponentID, aSpecializationName, aFactoryAllocator, nullptr);
	componentData.SetSpecializationOf(aBaseName);
}

#pragma once
#include "ComponentData.h"

class Component;
template <typename T>
class ComponentFactory;

template <typename TComponentType>
BaseComponentFactory* CreateComponentFactory()
{
	return new ComponentFactory<TComponentType>(TypeID<Component>::Resolve<TComponentType>());
}

#define DECLARE_COMPONENT(TComponent) do { Internal_AddComponent(TypeID<Component>::Resolve<TComponent>().GetUnderlyingInteger(), L"" #TComponent, &CreateComponentFactory<TComponent>, nullptr); } while (false)
#define DECLARE_COMPONENT_SPECIALIZATION(BaseComponent, Specialization) do { Internal_AddSpecialization(L"" #BaseComponent, TypeID<Component>::Resolve<Specialization>().GetUnderlyingInteger(), L"" #Specialization, &CreateComponentFactory<Specialization>, nullptr); } while (false)

class ComponentList
{
public:
	virtual ~ComponentList() = default;

	void BuildList();

	template <typename T>
	FORCEINLINE const ComponentData& GetComponentData() const
	{
		return GetComponentData(TypeID<Component>::Resolve<T>());
	}

	FORCEINLINE const ComponentData& GetComponentData(const TypeID<Component>& aComponentID) const
	{
		const auto* data = myIDToData.Find(aComponentID.GetUnderlyingInteger());
		if (!data)
			FATAL(L"This component has not been registered");
		return *data;
	}

	FORCEINLINE const ComponentData& GetComponentData(const StringView& aComponentName) const
	{
		const auto* id = myTypeNameToID.Find(aComponentName);
		if (!id)
			FATAL(L"This component has not been registered");
		const auto* data = myIDToData.Find(*id);
		if (!data)
			FATAL(L"This component has not been registered");
		return *data;
	}

protected:
	// Use macros "DECLARE_COMPONENT" and "DECLARE_COMPONENT_SPECIALIZATION" instead
	ComponentData& Internal_AddComponent(u16 aComponentID, const StringView& aComponentName, BaseComponentFactory*(*aFactoryAllocator)(), nullptr_t);
	void Internal_AddSpecialization(const StringView& aBaseName, u16 aComponentID, const StringView& aSpecializationName, BaseComponentFactory*(*aFactoryAllocator)(), nullptr_t);

	virtual void RegisterComponents();

private:
	// Name -> ID
	Map<StringView, u16> myTypeNameToID;

	// ID -> Component Data
	Map<u16, ComponentData> myIDToData;
};


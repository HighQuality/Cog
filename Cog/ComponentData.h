#pragma once

class BaseComponentFactory;

class ComponentData
{
public:
	const StringView& GetName() const { return myName; }
	void SetName(const StringView& aName) { myName = aName; }

	const StringView& GetSpecializationOf() const { return mySpecializationOf; }
	void SetSpecializationOf(const StringView& aSpecializationOf) { mySpecializationOf = aSpecializationOf; }

	const ComponentData* GetSpecialization() const { return mySpecialization; }
	void SetSpecialization(const ComponentData& aSpecialization) { mySpecialization = &aSpecialization; }

	void SetFactoryAllocator(BaseComponentFactory*(*aFactoryAllocator)()) { myFactoryAllocator = aFactoryAllocator; }

	BaseComponentFactory* AllocateFactory() const { return myFactoryAllocator(); }

	bool operator==(const ComponentData& aRight) const
	{
		return myName == aRight.myName &&
			mySpecializationOf == aRight.mySpecializationOf;
	}

	bool operator!=(const ComponentData& aRight) const
	{
		return !(*this == aRight);
	}

private:
	const ComponentData* mySpecialization = nullptr;
	StringView myName; 
	StringView mySpecializationOf;
	BaseComponentFactory*(*myFactoryAllocator)() = nullptr;
};

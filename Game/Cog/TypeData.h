#pragma once

class BaseFactory;

class TypeData
{
public:
	const StringView& GetName() const { return myName; }
	void SetName(const StringView& aName) { myName = aName; }

	const StringView& GetSpecializationOf() const { return mySpecializationOf; }
	void SetSpecializationOf(const StringView& aSpecializationOf) { mySpecializationOf = aSpecializationOf; }

	const TypeData* GetSpecialization() const { return mySpecialization; }
	void SetSpecialization(const TypeData& aSpecialization) { mySpecialization = &aSpecialization; }

	void SetFactoryAllocator(BaseFactory*(*aFactoryAllocator)()) { myFactoryAllocator = aFactoryAllocator; }

	BaseFactory* AllocateFactory() const { return myFactoryAllocator(); }

	bool operator==(const TypeData& aRight) const
	{
		return myName == aRight.myName &&
			mySpecializationOf == aRight.mySpecializationOf;
	}

	bool operator!=(const TypeData& aRight) const
	{
		return !(*this == aRight);
	}

private:
	const TypeData* mySpecialization = nullptr;
	StringView myName; 
	StringView mySpecializationOf;
	BaseFactory*(*myFactoryAllocator)() = nullptr;
};

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
	void SetSpecialization(TypeData& aSpecialization) { mySpecialization = &aSpecialization; }

	void SetFactoryAllocator(UniquePtr<BaseFactory>(*aFactoryAllocator)()) { myFactoryAllocator = aFactoryAllocator; }

	UniquePtr<BaseFactory> AllocateFactory() const { return myFactoryAllocator(); }

	bool operator==(const TypeData& aRight) const
	{
		return myName == aRight.myName &&
			mySpecializationOf == aRight.mySpecializationOf;
	}

	bool operator!=(const TypeData& aRight) const
	{
		return !(*this == aRight);
	}

	const TypeData& GetOutermostSpecialization() const
	{
		CHECK(myOutermostSpecialization);
		return *myOutermostSpecialization;
	}

private:
	bool HasOutermostSpecialization() const { return myOutermostSpecialization != nullptr; }

	FORCEINLINE const TypeData& AssignOutermostSpecialization()
	{
		if (!mySpecialization)
		{
			myOutermostSpecialization = this;
			return *this;
		}

		myOutermostSpecialization = &mySpecialization->AssignOutermostSpecialization();
		CHECK(myOutermostSpecialization);
		return *myOutermostSpecialization;
	}

	friend class TypeList;

	const TypeData* myOutermostSpecialization = nullptr;
	
	TypeData* mySpecialization = nullptr;
	StringView myName; 
	StringView mySpecializationOf;
	UniquePtr<BaseFactory>(*myFactoryAllocator)() = nullptr;
};

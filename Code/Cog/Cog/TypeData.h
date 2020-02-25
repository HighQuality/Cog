#pragma once

class CogTypeBase;
class CogTypeChunk;
class Singleton;

class TypeData
{
public:
	TypeData();
	~TypeData();

	TypeData(const TypeData&) = delete;
	TypeData& operator=(const TypeData&) = delete;

	TypeData(TypeData&&) = delete;
	TypeData& operator=(TypeData&&) = delete;

	FORCEINLINE StringView GetName() const { return myName; }
	FORCEINLINE StringView GetBaseName() const { return myBaseName; }

	FORCEINLINE const TypeData* GetBaseType() const { return myBaseType; }

	FORCEINLINE bool IsSpecializingBaseType() const { return myIsSpecialization; }
	FORCEINLINE bool IsSpecialized() const { return mySpecializedBy != nullptr; }

	UniquePtr<CogTypeChunk> AllocateFactory() const { CHECK(!IsSingleton()); return myFactoryAllocator(); }
	UniquePtr<Singleton> AllocateSingleton() const { CHECK(IsSingleton()); return mySingletonAllocator(); }

	FORCEINLINE bool IsSingleton() const { return mySingletonAllocator != nullptr; }

	FORCEINLINE const TypeID<CogTypeBase>& GetTypeID() const { return *myTypeID; }

private:
	friend class TypeList;

	TypeData* mySpecializedBy = nullptr;

	TypeData* myBaseType = nullptr;
	Array<TypeData*> mySubtypes;

	String myName;
	String myBaseName;
	UniquePtr<CogTypeChunk>(*myFactoryAllocator)() = nullptr;
	UniquePtr<Singleton>(*mySingletonAllocator)() = nullptr;
	const TypeID<CogTypeBase>* myTypeID = nullptr;
	bool myIsSpecialization = false;
};

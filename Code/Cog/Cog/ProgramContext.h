#pragma once

class Singleton;
class CogTypeBase;

class ProgramContext
{
public:
	ProgramContext();
	~ProgramContext();
	void Run();

	template <typename T>
	FORCEINLINE T& GetSingleton() const
	{
		return reinterpret_cast<T&>(GetSingleton(TypeID<CogTypeBase>::Resolve<T>()));
	}

	FORCEINLINE Singleton& GetSingleton(const TypeID<CogTypeBase>& aTypeId) const
	{
		const u16 typeIndex = aTypeId.GetUnderlyingInteger();
		Singleton* singleton = myTypeIdToSingleton.TryGet(typeIndex, nullptr);
		CHECK(singleton);
		return *singleton;
	}

	// This should only be used when initializing singletons as it creates singletons that haven't been created yet.
	// Use GetSingleton<T>() during runtime instead.
	template <typename T>
	T& ResolveSingleton()
	{
		return reinterpret_cast<T&>(ResolveSingleton(TypeID<CogTypeBase>::Resolve<T>()));
	}

private:
	void InitializeSingletons();
	Singleton& ResolveSingleton(const TypeID<CogTypeBase>& aTypeId);

	Array<UniquePtr<Singleton>> mySingletonInstances;
	Array<Singleton*> myTypeIdToSingleton;
	bool myIsInitializingSingletons = false;
};

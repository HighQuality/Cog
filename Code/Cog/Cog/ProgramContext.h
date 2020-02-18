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

private:
	void InitializeSingletons();
	void DestroySingletons();

	Array<UniquePtr<Singleton>> mySingletonInstances;
	Array<Singleton*> myTypeIdToSingleton;
};

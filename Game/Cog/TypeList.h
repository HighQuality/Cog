#pragma once
#include "TypeData.h"
#include <Memory/Factory.h>

class Object;

template <typename TComponentType>
BaseFactory* CreateObjectFactory()
{
	return new Factory<TComponentType>();
}

#define CHECK_BASE_DECLARED(TType) \
	do { \
	static_assert(!IsSame<TType::Base, TType::Base::Base>, "Component " #TType " does not declare it's base"); \
	static_assert(IsDerivedFrom<TType, TType::Base>, #TType "::Base is not in inheritance chain."); \
	} while (false)

#define REGISTER_TYPE(TType) \
	do { \
	CHECK_BASE_DECLARED(TType); \
	Internal_AddType(TypeID<Object>::Resolve<TType>().GetUnderlyingInteger(), L"" #TType, &CreateObjectFactory<TType>, nullptr); \
	} while (false)

#define REGISTER_TYPE_SPECIALIZATION(BaseType, Specialization) \
	do { \
	CHECK_BASE_DECLARED(Specialization); \
	static_assert(IsDerivedFrom<Specialization, BaseType>, #Specialization " does not derive from " #BaseType); \
	Internal_AddSpecialization(L"" #BaseType, TypeID<Object>::Resolve<Specialization>().GetUnderlyingInteger(), L"" #Specialization, &CreateObjectFactory<Specialization>, nullptr); \
	} while (false)

class TypeList
{
public:
	virtual ~TypeList() = default;

	void BuildList();

	template <typename T>
	FORCEINLINE const TypeData& GetTypeData() const
	{
		return GetTypeData(TypeID<Object>::Resolve<T>());
	}

	FORCEINLINE const TypeData& GetTypeData(const TypeID<Object>& aTypeID) const
	{
		const auto* data = myIDToData.Find(aTypeID.GetUnderlyingInteger());
		if (!data)
			FATAL(L"This type has not been registered");
		return *data;
	}

	FORCEINLINE const TypeData& GetTypeData(const StringView& aTypeName) const
	{
		const auto* id = myTypeNameToID.Find(aTypeName);
		if (!id)
			FATAL(L"This type has not been registered");
		const auto* data = myIDToData.Find(*id);
		if (!data)
			FATAL(L"This type has not been registered");
		return *data;
	}

protected:
	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddType(u16 aTypeID, const StringView& aTypeName, BaseFactory*(*aFactoryAllocator)(), nullptr_t);
	void Internal_AddSpecialization(const StringView& aBaseName, u16 aTypeID, const StringView& aSpecializationName, BaseFactory*(*aFactoryAllocator)(), nullptr_t);

	virtual void RegisterTypes();

private:
	// Name -> ID
	Map<StringView, u16> myTypeNameToID;

	// ID -> Component Data
	Map<u16, TypeData> myIDToData;
};


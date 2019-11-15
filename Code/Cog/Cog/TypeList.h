#pragma once
#include "TypeData.h"
#include <Memory/Factory.h>

class Object;

template <typename TComponentType, EnableIf<!std::is_abstract_v<TComponentType>, int> = 0>
UniquePtr<BaseFactory> CreateObjectFactory()
{
	return MakeUnique<Factory<TComponentType>>();
}

template <typename TComponentType, EnableIf<std::is_abstract_v<TComponentType>, int> = 0>
UniquePtr<BaseFactory> CreateObjectFactory()
{
	FATAL(L"Can't instantiate object of abstract type");
}

#define CHECK_BASE_DECLARED(TType) \
	do { \
	static_assert(!IsSame<TType::Base, TType::Base::Base>, "Component " #TType " does not declare it's base"); \
	static_assert(IsDerivedFrom<TType, TType::Base>, #TType "::Base is not in inheritance chain."); \
	} while (false)

#define REGISTER_TYPE(TypeListObject, TType) \
	do { \
	CHECK_BASE_DECLARED(TType); \
	TypeListObject->Internal_AddType(TypeID<Object>::Resolve<TType>().GetUnderlyingInteger(), L"" #TType, &CreateObjectFactory<TType>, nullptr); \
	} while (false)

#define REGISTER_TYPE_SPECIALIZATION(TypeListObject, BaseType, Specialization) \
	do { \
	CHECK_BASE_DECLARED(Specialization); \
	static_assert(IsDerivedFrom<Specialization, BaseType>, #Specialization " does not derive from " #BaseType); \
	TypeListObject->Internal_AddSpecialization(L"" #BaseType, TypeID<Object>::Resolve<Specialization>().GetUnderlyingInteger(), L"" #Specialization, &CreateObjectFactory<Specialization>, nullptr); \
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

	FORCEINLINE const TypeData& GetTypeData(const TypeID<Object>& aTypeID, const bool aOutermost = true) const
	{
		const auto* data = myIDToData.Find(aTypeID.GetUnderlyingInteger());
		if (!data)
			FATAL(L"This type has not been registered");
		return aOutermost ? data->GetOutermostSpecialization() : *data;
	}

	FORCEINLINE const TypeData& GetTypeData(const StringView& aTypeName, const bool aOutermost = true) const
	{
		const auto* id = myTypeNameToID.Find(aTypeName);
		if (!id)
			FATAL(L"This type has not been registered");
		const auto* data = myIDToData.Find(*id);
		if (!data)
			FATAL(L"This type has not been registered");
		return aOutermost ? data->GetOutermostSpecialization() : *data;
	}

	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddType(u16 aTypeID, const StringView& aTypeName, UniquePtr<BaseFactory>(*aFactoryAllocator)(), nullptr_t);
	void Internal_AddSpecialization(const StringView& aBaseName, u16 aTypeID, const StringView& aSpecializationName, UniquePtr<BaseFactory>(*aFactoryAllocator)(), nullptr_t);

private:
	// Name -> ID
	Map<StringView, u16> myTypeNameToID;

	// ID -> Component Data
	Map<u16, TypeData> myIDToData;
};


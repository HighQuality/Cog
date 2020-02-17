#pragma once
#include "TypeData.h"
#include "CogTypeChunk.h"

class CogTypeBase;
class Object;
class Singleton;

template <typename TObjectType, typename TChunkType, EnableIf<!std::is_abstract_v<TObjectType>, i32> = 0>
UniquePtr<CogTypeChunk> CreateObjectChunk()
{
	return MakeUnique<TChunkType>();
}

template <typename TObjectType, typename TChunkType, EnableIf<std::is_abstract_v<TObjectType>, i32> = 0>
UniquePtr<CogTypeChunk> CreateObjectChunk()
{
	FATAL(L"Can't instantiate object of abstract type");
}

template <typename TSingleton, EnableIf<!std::is_abstract_v<TSingleton>, i32> = 0>
UniquePtr<TSingleton> CreateSingleton()
{
	return MakeUnique<TSingleton>();
}

template <typename TSingleton, EnableIf<std::is_abstract_v<TSingleton>, i32> = 0>
UniquePtr<TSingleton> CreateSingleton()
{
	FATAL(L"Can't instantiate singleton of abstract type");
}

#define IS_SINGLETON(TType) \
	IsDerivedFrom<TType, Singleton>

#define CHECK_BASE_DECLARED(TType) \
	do { \
	static_assert(!IsSame<TType::Base, TType::Base::Base>, "Type " #TType " does not declare it's base"); \
	static_assert(IsDerivedFrom<TType, TType::Base>, #TType "::Base is not in inheritance chain."); \
	} while (false)

// Singletons contain their COGPROPERTIES as member variables, therefore the size check is not applicable
#define CHECK_SIZE(TType) \
	do { \
	static_assert(IS_SINGLETON(TType) || sizeof(TType) == sizeof(Object) || sizeof(TType) == sizeof(TType::Base), "Type " #TType " may not contain member variables "); \
	} while (false)

#define REGISTER_TYPE(TypeListObject, TType) \
	do { \
	CHECK_BASE_DECLARED(TType); \
	CHECK_SIZE(TType); \
	if constexpr (!IS_SINGLETON(TType)) \
		TypeListObject->Internal_AddType(TypeID<CogTypeBase>::Resolve<TType>(), L"" #TType, &CreateObjectChunk<TType, JOIN(TType, CogTypeChunk)>, nullptr); \
	else \
		TypeListObject->Internal_AddSingleton(TypeID<CogTypeBase>::Resolve<TType>(), L"" #TType, &CreateSingleton<TType>, nullptr) \
	} while (false)

#define REGISTER_TYPE_SPECIALIZATION(TypeListObject, BaseType, Specialization) \
	do { \
	CHECK_BASE_DECLARED(Specialization); \
	CHECK_SIZE(Specialization); \
	static_assert(IsDerivedFrom<Specialization, BaseType>, #Specialization " does not derive from " #BaseType); \
	if constexpr (!IS_SINGLETON(Specialization)) \
		TypeListObject->Internal_AddSpecialization(L"" #BaseType, TypeID<CogTypeBase>::Resolve<Specialization>(), L"" #Specialization, &CreateObjectChunk<Specialization, JOIN(Specialization, CogTypeChunk)>, nullptr); \
	else \
		TypeListObject->Internal_AddSingletonSpecialization(L"" #BaseType, TypeID<CogTypeBase>::Resolve<Specialization>(), L"" #Specialization, &CreateSingleton<Specialization>, nullptr) \
	} while (false)

class TypeList
{
public:
	TypeList() = default;
	virtual ~TypeList() = default;

	void BuildList();

	template <typename T>
	const TypeData& GetTypeData(const bool aOutermost = true) const
	{
		return GetTypeData(TypeID<CogTypeBase>::Resolve<T>(), aOutermost);
	}

	const TypeData& GetTypeData(const TypeID<CogTypeBase>& aTypeID, const bool aOutermost = true) const
	{
		return GetTypeDataByIndex(aTypeID.GetUnderlyingInteger(), aOutermost);
	}

	const TypeData& GetTypeData(const StringView& aTypeName, const bool aOutermost = true) const
	{
		const u16* id = myTypeNameToID.Find(aTypeName);
		if (!id)
			FATAL(L"This type has not been registered");
		return GetTypeDataByIndex(*id, aOutermost);
	}

	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddType(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t);
	void Internal_AddSpecialization(const StringView& aBaseName, const TypeID<CogTypeBase>& aTypeID, const StringView& aSpecializationName, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t);

	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddSingleton(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, UniquePtr<Singleton>(*aSingletonAllocator)(), nullptr_t);
	void Internal_AddSingletonSpecialization(const StringView& aBaseName, const TypeID<CogTypeBase>& aTypeID, const StringView& aSpecializationName, UniquePtr<Singleton>(*aSingletonAllocator)(), nullptr_t);

private:
	const TypeData& GetTypeDataByIndex(u16 aTypeIndex, bool aOutermost) const;
	
	TypeData& Internal_AddGenericType(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName);
	
	// Name -> ID
	Map<StringView, u16> myTypeNameToID;

	// ID -> Component Data
	Array<UniquePtr<TypeData>> myIDToData;
};

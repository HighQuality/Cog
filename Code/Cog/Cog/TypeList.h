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
UniquePtr<Singleton> CreateSingleton()
{
	return MakeUnique<TSingleton>();
}

template <typename TSingleton, EnableIf<std::is_abstract_v<TSingleton>, i32> = 0>
UniquePtr<Singleton> CreateSingleton()
{
	FATAL(L"Can't instantiate singleton of abstract type");
}

#define IS_SINGLETON(TType) \
	IsDerivedFrom<TType, Singleton>

// Singletons contain their COGPROPERTIES as member variables, therefore the size check is not applicable
#define CHECK_SIZE(TType) \
	do { \
	static_assert(IS_SINGLETON(TType) || sizeof(TType) == sizeof(Object) || sizeof(TType) == sizeof(TType::Base), "Type " #TType " may not contain member variables "); \
	} while (false)

#define REGISTER_TYPE(TypeListObject, TType) \
	do { \
		if constexpr (!IS_SINGLETON(TType)) { \
			CHECK_SIZE(TType); \
			TypeListObject->Internal_AddType(TypeID<CogTypeBase>::Resolve<TType>(), TType::StaticTypeName, TType::Base::StaticTypeName, TType::StaticIsSpecialization, &CreateObjectChunk<TType, class JOIN(TType, CogTypeChunk)>, nullptr); \
		} else { \
			TypeListObject->Internal_AddSingleton(TypeID<CogTypeBase>::Resolve<TType>(), TType::StaticTypeName, TType::Base::StaticTypeName, TType::StaticIsSpecialization, &CreateSingleton<TType>, nullptr); \
		} \
	} while (false)

class TypeList
{
public:
	TypeList() = default;
	virtual ~TypeList() = default;

	TypeList(const TypeList&) = delete;
	TypeList& operator=(const TypeList&) = delete;

	TypeList(TypeList&&) = default;
	TypeList& operator=(TypeList&&) = default;

	void BuildList();

	FORCEINLINE ArrayView<const TypeData*> GetSingletons(const bool aOnlyLeafTypes = true) const { return  aOnlyLeafTypes ? myLeafSingletons : myAllSingletons; }
	FORCEINLINE ArrayView<const TypeData*> GetObjectTypes(const bool aOnlyLeafTypes = true) const { return aOnlyLeafTypes ? myLeafObjectTypes : myAllObjectTypes; }

public:
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

private:
	template <typename T>
	TypeData& GetTypeData(const bool aOutermost = true)
	{
		return GetTypeData(TypeID<CogTypeBase>::Resolve<T>(), aOutermost);
	}

	TypeData& GetTypeData(const TypeID<CogTypeBase>& aTypeID, const bool aOutermost = true)
	{
		return GetTypeDataByIndex(aTypeID.GetUnderlyingInteger(), aOutermost);
	}

	TypeData& GetTypeData(const StringView& aTypeName, const bool aOutermost = true)
	{
		const u16* id = myTypeNameToID.Find(aTypeName);
		if (!id)
			FATAL(L"This type has not been registered");
		return GetTypeDataByIndex(*id, aOutermost);
	}

public:
	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddType(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, const StringView& aBaseTypeName, bool aIsSpecialization, UniquePtr<CogTypeChunk>(*aFactoryAllocator)(), nullptr_t);

	// Use macros "REGISTER_TYPE" and "REGISTER_TYPE_SPECIALIZATION" instead
	TypeData& Internal_AddSingleton(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, const StringView& aBaseTypeName, bool aIsSpecialization, UniquePtr<Singleton>(*aSingletonAllocator)(), nullptr_t);

private:
	const TypeData& GetTypeDataByIndex(u16 aTypeIndex, bool aOutermost) const;
	TypeData& GetTypeDataByIndex(u16 aTypeIndex, bool aOutermost);
	
	TypeData& Internal_AddGenericType(const TypeID<CogTypeBase>& aTypeID, const StringView& aTypeName, const StringView& aBaseTypeName, bool aIsSpecialization);
	
	// Name -> ID
	Map<StringView, u16> myTypeNameToID;

	// ID -> Component Data
	Array<UniquePtr<TypeData>> myIDToData;

	Array<const TypeData*> myLeafSingletons;
	Array<const TypeData*> myAllSingletons;
	
	Array<const TypeData*> myLeafObjectTypes;
	Array<const TypeData*> myAllObjectTypes;
};

extern const TypeList gTypeList;

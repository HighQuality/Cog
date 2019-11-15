#include "GamePch.h"
#include "ObjectPool.h"
#include "Object.h"
#include "TypeList.h"

static ObjectPool gObjectPool;
static UniquePtr<const TypeList> gTypeList;

void RegisterTypeList()
{
	UniquePtr<TypeList> typeList = MakeUnique<TypeList>();
	typeList->BuildList();
	gTypeList = Move(typeList);
}

Object& NewObjectByType(const TypeID<Object>& aTypeID)
{
	return gObjectPool.CreateObjectByType(aTypeID);
}

static UniquePtr<BaseFactory> CreateFactoryOfType(const TypeID<Object>& aType)
{
	return gTypeList->GetTypeData(aType).AllocateFactory();
}

Object& ObjectPool::CreateObjectByType(const TypeID<Object>& aType)
{
	BaseFactory& factory = FindOrCreateObjectFactory(aType, &CreateFactoryOfType);

	return *static_cast<Object*>(factory.AllocateRawObject());
}

BaseFactory& ObjectPool::FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, UniquePtr<BaseFactory> (*aFactoryCreator)(const TypeID<Object>&))
{
	const u16 index = aObjectType.GetUnderlyingInteger();
	myObjectFactories.Resize(TypeID<Object>::MaxUnderlyingInteger());
	auto& factory = myObjectFactories[index];
	if (!factory)
		factory = aFactoryCreator(aObjectType);
	return *factory;
}

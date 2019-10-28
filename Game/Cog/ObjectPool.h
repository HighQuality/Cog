#pragma once
#include <TypeFundamentals/TypeID.h>

class BaseFactory;
class Object;

class ObjectPool
{
public:
	Object& CreateObjectByType(const TypeID<Object>& aType);

private:
	BaseFactory& FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, UniquePtr<BaseFactory>(*aFactoryCreator)(const TypeID<Object>&));

	Array<UniquePtr<BaseFactory>> myObjectFactories;
};

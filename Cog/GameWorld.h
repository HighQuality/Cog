#pragma once
#include "TypeID.h"
#include "ComponentFactory.h"

class RenderTarget;
class Object;
class Component;
class BaseComponentFactory;
class ObjectFactory;

template <typename T, typename TChunkType = FactoryChunk<T>>
class Factory;

class GameWorld
{
public:
	GameWorld();
	~GameWorld();

	void DispatchTick(Time aDeltaTime);
	void DispatchDraw(RenderTarget& aRenderTarget);

	Object& CreateObject();

	// TODO: Remove
	void RemoveObject(const Object& object);

	template <typename T>
	T& CreateComponent(Object& aObject)
	{
		BaseComponentFactory& factory = FindOrCreateComponentFactory<T>();		
		return static_cast<T&>(CreateComponentOnObjectFromFactory(factory, aObject));
	}

private:
	static Component& CreateComponentOnObjectFromFactory(BaseComponentFactory& aComponentFactory, Object& aObject);

	ObjectFactory* myObjectFactory;

	template <typename T>
	BaseComponentFactory& FindOrCreateComponentFactory()
	{
		const auto factoryID = TypeID<Component>::Resolve<T>();
		myComponentFactories.Resize(TypeID<Component>::MaxUnderlyingInteger());

		BaseComponentFactory*& factory = myComponentFactories[factoryID.GetUnderlyingInteger()];
		if (factory == nullptr)
			factory = new ComponentFactory<T>(factoryID);
		return *factory;
	}

	Array<BaseComponentFactory*> myComponentFactories;
};


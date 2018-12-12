#pragma once
#include "TypeID.h"
#include "ComponentFactory.h"

class GameWorld;

class RenderTarget;
class Object;
class Component;
class BaseComponentFactory;
class ObjectFactory;
class ObjectInitializer;

template <typename T, typename TChunkType = FactoryChunk<T>>
class Factory;

class CogGameWorld
{
public:
	CogGameWorld();
	virtual ~CogGameWorld();

	void DispatchTick(Time aDeltaTime);
	void DispatchDraw(RenderTarget& aRenderTarget);

	[[nodiscard]]
	ObjectInitializer CreateObject();

	// TODO: Remove
	void RemoveObject(const Object& object);

	BaseComponentFactory& FindOrCreateComponentFactory(TypeID<Component> aComponentType, BaseComponentFactory*(*aFactoryCreator)());

protected:
	virtual GameWorld* GetSubPointer() = 0;
	virtual const GameWorld* GetSubPointer() const = 0;

private:
	ObjectFactory* myObjectFactory;

	Array<BaseComponentFactory*> myComponentFactories;
};


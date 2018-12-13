#pragma once
#include "TypeID.h"
#include "ComponentFactory.h"

class Scene;

class RenderTarget;
class Object;
class Component;
class BaseComponentFactory;
class ObjectFactory;
class ObjectInitializer;
class CogGame;

template <typename T, typename TChunkType = FactoryChunk<T>>
class Factory;

class CogScene
{
public:
	CogScene(CogGame& aGame);
	virtual ~CogScene();

	void DispatchTick(Time aDeltaTime);
	void DispatchDraw(RenderTarget& aRenderTarget);

	FORCEINLINE CogGame& GetCogGame() const { return myGame; }
	
	[[nodiscard]]
	ObjectInitializer CreateObject();

	// TODO: Remove
	void RemoveObject(const Object& object);

	BaseComponentFactory& FindOrCreateComponentFactory(TypeID<Component> aComponentType, BaseComponentFactory*(*aFactoryCreator)());

protected:
	virtual Scene* GetSubPointer() = 0;
	virtual const Scene* GetSubPointer() const = 0;

private:
	ObjectFactory& myObjectFactory;
	CogGame& myGame;

	Array<BaseComponentFactory*> myComponentFactories;
};


#pragma once
#include "CogGame.h"
#include "InheritComponent.h"
#include <FunctionView.h>
#include "Object.h"

class RenderTarget;
class Entity;
class EntityInitializer;
class CogGame;

class ComponentChunkedData : public ChunkedData
{
public:
	using Base = ChunkedData;

	ComponentChunkedData(i32 aElements)
	{
		myEntity.Resize(aElements);
	}

	void DefaultInitializeIndex(const u16 aIndex) override
	{
		myEntity[aIndex] = nullptr;	
	}

private:
	friend class Component;
	Array<Entity*> myEntity;
};

class Component : public Object
{
	DECLARE_CHUNKED_DATA(ComponentChunkedData);

	DECLARE_CHUNKED_PROPERTY_ACCESSORS(Entity, private, public);

public:
	using Base = Object;

	virtual void Tick(const FrameData& aTickData) {  }

	FORCEINLINE Entity* GetParent() const { return GetEntity()->GetParent(); }

	template <typename T>
	FORCEINLINE T& CreateWidget() { return GetEntity()->CreateWidget<T>(); }

	FORCEINLINE virtual void GetBaseClasses(const FunctionView<void(const TypeID<Component>&)>& aFunction) const { }

	void Destroy() override;

protected:
	Component() = default;


	virtual void Initialize() {  }

	friend Entity;
	virtual void ResolveDependencies(EntityInitializer& aInitializer) {  }

	template <typename T>
	friend class ComponentFactory;
};

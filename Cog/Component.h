#pragma once
#include "BaseComponentFactoryChunk.h"
#include "CogGame.h"
#include "InheritComponent.h"
#include <FunctionView.h>

class RenderTarget;
class BaseComponentFactoryChunk;
class Object;
class ObjectInitializer;
class CogGame;

class Component
{
public:
	virtual ~Component() = default;

	Component(const Component&) = delete;
	Component(Component&&) = delete;

	Component& operator=(const Component&) = delete;
	Component& operator=(Component&&) = delete;
	
	FORCEINLINE bool IsTickEnabled() const { return myChunk->IsTickEnabled(myChunkIndex); }
	FORCEINLINE bool IsVisible() const { return myChunk->IsVisible(myChunkIndex); }

	// TODO: Should these return const references if we are const?
	FORCEINLINE Object& GetObject() const { return myChunk->FindObject(myChunkIndex); }
	
	FORCEINLINE Object& GetParent() const { return GetObject().GetParent(); }
	FORCEINLINE Object* TryGetParent() const { return GetObject().TryGetParent(); }
	FORCEINLINE bool HasParent() const { return GetObject().HasParent(); }
	
	template <typename TType, typename ...TArgs>
	void Synchronize(TType& aObject, void(TType::*aFunction)(TArgs...))
	{
		GetGame().Synchronize(aObject, aFunction);
	}

	FORCEINLINE virtual void GetBaseClasses(const FunctionView<void(const TypeID<Component>&)>& aFunction) const { }

	// TODO: These are only public since ComponentFactoryChunk needs to access them in order to compare them to this base function
	virtual void Tick(Time aDeltaTime) {  }
	virtual void Draw2D(RenderTarget& aRenderTarget) const {  }
	virtual void Draw3D(RenderTarget& aRenderTarget) const {  }

protected:
	Component() = default;
	
	void SetTickEnabled(const bool aShouldTick);
	void SetIsVisible(const bool aIsVisible);

	virtual void Initialize() {  }

	friend Object;
	virtual void ResolveDependencies(ObjectInitializer& aInitializer) {  }

private:
	template <typename T>
	friend class ComponentFactoryChunk;

	template <typename T>
	friend class Ptr;

	friend Object;
	
	BaseComponentFactoryChunk* myChunk;
	u16 myChunkIndex;
};

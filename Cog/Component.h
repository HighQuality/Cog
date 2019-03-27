#pragma once
#include "BaseComponentFactoryChunk.h"
#include "CogGame.h"
#include "InheritComponent.h"
#include <FunctionView.h>

class RenderTarget;
class BaseComponentFactoryChunk;
class Entity;
class EntityInitializer;
class CogGame;

class Component
{
public:
	using Base = void;

	virtual ~Component();

	Component(const Component&) = delete;
	Component(Component&&) = delete;

	Component& operator=(const Component&) = delete;
	Component& operator=(Component&&) = delete;
	
	FORCEINLINE bool IsTickEnabled() const { return myChunk->IsTickEnabled(myChunkIndex); }

	// TODO: Should these return const references if we are const?
	FORCEINLINE Entity& GetEntity() const { return myChunk->FindEntity(myChunkIndex); }
	
	FORCEINLINE Entity& GetParent() const { return GetEntity().GetParent(); }
	FORCEINLINE Entity* TryGetParent() const { return GetEntity().TryGetParent(); }
	FORCEINLINE bool HasParent() const { return GetEntity().HasParent(); }

	template <typename T>
	FORCEINLINE T& CreateWidget() { return GetEntity().CreateWidget<T>(); }

	FORCEINLINE virtual void GetBaseClasses(const FunctionView<void(const TypeID<Component>&)>& aFunction) const { }

protected:
	Component() = default;

	virtual void Tick(const FrameData& aTickData) {  }

	void SetTickEnabled(const bool aShouldTick);

	virtual void Initialize() {  }

	friend Entity;
	virtual void ResolveDependencies(EntityInitializer& aInitializer) {  }

private:
	template <typename T>
	friend class ComponentFactoryChunk;
	
	template <typename T>
	friend class Ptr;

	friend Entity;
	
	BaseComponentFactoryChunk* myChunk;
	u16 myChunkIndex;
};

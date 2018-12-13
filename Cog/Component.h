#pragma once
#include "BaseComponentFactoryChunk.h"

class CogScene;
class RenderTarget;
class BaseComponentFactoryChunk;
class Object;
class ObjectInitializer;

class Component
{
public:
	virtual ~Component();

	Component(const Component&) = delete;
	Component(Component&&) = delete;

	Component& operator=(const Component&) = delete;
	Component& operator=(Component&&) = delete;

	FORCEINLINE bool IsTickEnabled() const { return myChunk->IsTickEnabled(myChunkIndex); }
	FORCEINLINE bool IsVisible() const { return myChunk->IsVisible(myChunkIndex); }

	// TODO: Should these return const references if we are const?
	FORCEINLINE Object& GetObject() const { return myChunk->FindObject(myChunkIndex); }
	FORCEINLINE Scene& GetScene() const { return GetObject().GetScene(); }

protected:
	Component();
	
	void SetTickEnabled(const bool aShouldTick) { myChunk->SetTickEnabled(myChunkIndex, aShouldTick); }
	void SetIsVisible(const bool aIsVisible) { myChunk->SetIsVisible(myChunkIndex, aIsVisible); }

	virtual void Initialize() {  }

	friend Object;
	virtual void ResolveDependencies(ObjectInitializer& aInitializer) {  }

	virtual void Tick(Time aDeltaTime) {  }
	virtual void Draw2D(RenderTarget& aRenderTarget) const {  }
	virtual void Draw3D(RenderTarget& aRenderTarget) const {  }
	
private:
	template <typename T>
	friend class ComponentFactoryChunk;

	template <typename T>
	friend class Ptr;

	friend Object;
	friend CogScene;

	BaseComponentFactoryChunk* myChunk;
	u16 myChunkIndex;
};

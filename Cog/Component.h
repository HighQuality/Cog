#pragma once
#include "BaseComponentFactoryChunk.h"
#include "CogGame.h"

class CogScene;
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
	FORCEINLINE Scene& GetScene() const { return GetObject().GetScene(); }
	FORCEINLINE CogScene& GetCogScene() const { return GetObject().GetCogScene(); }
	CogGame& GetCogGame() const;

	template <typename TType, typename ...TArgs>
	void Synchronize(TType& aObject, void(TType::*aFunction)(TArgs...))
	{
		GetCogGame().Synchronize(aObject, aFunction);
	}

	// TODO: These are only public since ComponentFactoryChunk needs to access them in order to compare them to this base function
	virtual void Tick(Time aDeltaTime) {  }
	virtual void Draw2D(RenderTarget& aRenderTarget) const {  }
	virtual void Draw3D(RenderTarget& aRenderTarget) const {  }

protected:
	Component() = default;
	
	void SetTickEnabled(const bool aShouldTick) { myChunk->SetTickEnabled(myChunkIndex, aShouldTick); }
	void SetIsVisible(const bool aIsVisible) { myChunk->SetIsVisible(myChunkIndex, aIsVisible); }

	virtual void Initialize() {  }

	friend Object;
	virtual void ResolveDependencies(ObjectInitializer& aInitializer) {  }

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

#pragma once

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

	bool ShouldTick() const;
	bool IsVisible() const;

	// TODO: Should these return const references if we are const?
	FORCEINLINE Object& GetObject() const { return *myObject; }
	FORCEINLINE Scene& GetScene() const { return GetObject().GetScene(); }

protected:
	Component();
	
	void SetShouldTick(bool aShouldTick);
	void SetIsVisible(bool aIsVisible);

	virtual void Initialize() {  }

	friend Object;
	virtual void ResolveDependencies(ObjectInitializer& aInitializer);

	virtual void Tick(Time aDeltaTime) {  }
	virtual void Draw2D(RenderTarget& aRenderTarget) const {  }
	virtual void Draw3D(RenderTarget& aRenderTarget) const {  }
	
private:
	template <typename T>
	friend class ComponentFactoryChunk;

	template <typename T>
	friend class Ptr;

	friend CogScene;

	BaseComponentFactoryChunk* myChunk;
	Object* myObject;
};

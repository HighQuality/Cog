#pragma once

class GameWorld;
class RenderTarget;
class BaseComponentFactoryChunk;
class Object;

class Component
{
public:
	virtual ~Component();

	bool ShouldTick() const;
	bool IsVisible() const;

	FORCEINLINE Object& GetObject() { return *myObject; }
	FORCEINLINE const Object& GetObject() const { return *myObject; }

protected:
	Component();
	
	void SetShouldTick(bool aShouldTick);
	void SetIsVisible(bool aIsVisible);

	virtual void Initialize() {  }

	virtual void Tick(Time aDeltaTime) {  }
	virtual void Draw2D(RenderTarget& aRenderTarget) const {  }
	virtual void Draw3D(RenderTarget& aRenderTarget) const {  }

private:
	template <typename T>
	friend class ComponentFactoryChunk;

	template <typename T>
	friend class Ptr;

	friend GameWorld;

	BaseComponentFactoryChunk* myChunk;
	Object* myObject;
};


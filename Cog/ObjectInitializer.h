#pragma once

class Object;

class ObjectInitializer
{
public:
	explicit ObjectInitializer(Object& aObject)
	{
		CHECK(IsInGameThread());
		myObject = &aObject;
	}

	ObjectInitializer(const ObjectInitializer&) = delete;
	ObjectInitializer& operator=(const ObjectInitializer&) = delete;

	ObjectInitializer(ObjectInitializer&& aOther)
	{
		*this = Move(aOther);
	}

	ObjectInitializer& operator=(ObjectInitializer&& aOther)
	{
		CHECK(!aOther.wasMoved);
		wasMoved = false;
		myObject = aOther.myObject;
		aOther.wasMoved = true;
		return *this;
	}

	~ObjectInitializer()
	{
		if (!wasMoved)
		{
			InitializeObject();
		}
	}

	template <typename TComponentType>
	TComponentType& AddComponent()
	{
		return myObject->AddComponent<TComponentType>();
	}

private:
	void InitializeObject();

	Object* myObject;
	bool wasMoved = false;
};

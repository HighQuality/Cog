#pragma once
#include <Utilities/Program.h>
#include <Utilities/FactoryChunk.h>

struct FrameData;
class Component;

class BaseComponentFactory
{
public:
	BaseComponentFactory(const TypeID<Component>& aTypeID)
	{
		myTypeID = aTypeID;
	}

	virtual ~BaseComponentFactory() = default;

	virtual Component& AllocateGeneric() = 0;
	virtual void ReturnGeneric(const Component& aComponent) = 0;

	virtual void DispatchTick() = 0;

	const TypeID<Component>& GetTypeID() const { return myTypeID; }

private:
	TypeID<Component> myTypeID;
};

template <typename T>
class ComponentFactory : public BaseComponentFactory
{
public:
	using Base = BaseComponentFactory;

	ComponentFactory()
		: Base(TypeID<Component>::Resolve<T>())
	{
	}

	~ComponentFactory() = default;

	DELETE_COPYCONSTRUCTORS_AND_MOVES(ComponentFactory);

	void DispatchTick() override
	{
		myFactory.IterateChunks([](FactoryChunk<T> & aChunk)
			{
				gProgram->QueueWork<FactoryChunk<T>>([](FactoryChunk<T> * aChunk)
				{
					const FrameData& frameData = GetGame().GetFrameData();

					aChunk->ForEach([&frameData](T & aComponent)
						{
							aComponent.T::Tick(frameData);
						});
				}, &aChunk);
			});
	}


	template <typename TCallback>
	void IterateChunks(TCallback aCallback)
	{
		myFactory.IterateChunks(Move(aCallback));
	}


	Component& AllocateGeneric() override
	{
		return myFactory.Allocate();
	}

	void ReturnGeneric(const Component& aComponent) override
	{
		myFactory.Return(static_cast<const T&>(aComponent));
	}

private:
	Factory<T> myFactory;
};

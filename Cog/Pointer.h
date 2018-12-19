#pragma once
#include "Entity.h"
#include "Component.h"
#include "Object.h"
#include "EntityFactory.h"
#include "BaseComponentFactoryChunk.h"
#include "BaseObjectFactoryChunk.h"
#include "EntityFactory.h"

template <typename T>
class Ptr final
{
public:
	Ptr()
	{
		myPointer = nullptr;
		myGeneration = 0;
	}

	Ptr(T* aPointer)
		: Ptr()
	{
		if (aPointer)
		{
			myPointer = aPointer;
			myGeneration = ResolveGeneration();
		}
	}

	Ptr(T& aPointer)
		: Ptr(&aPointer)
	{
	}

	FORCEINLINE bool operator==(const Ptr& aOther) const
	{
		return myPointer == aOther.myPointer;
	}

	FORCEINLINE bool operator!=(const Ptr& aOther) const
	{
		return myPointer != aOther.myPointer;
	}

	explicit operator bool() const { return IsValid(); }

	bool IsValid() const
	{
		if (myPointer)
		{
			if (myGeneration == ResolveGeneration())
				return true;
			*const_cast<Ptr*>(this) = Ptr();
		}

		return false;
	}

	operator T*() const
	{
		if (IsValid())
			return myPointer;
		return nullptr;
	}

	T* operator->() const
	{
		return *this;
	}

private:
	u16 ResolveGeneration() const
	{
		if (!myPointer)
			return 0;

		if constexpr (IsDerivedFrom<T, Component>)
		{
			const Component& component = *reinterpret_cast<const Component*>(myPointer);
			if (const auto* chunk = component.myChunk)
				return chunk->FindGeneration(component.myChunkIndex);
			return 0;
		}
		else if constexpr (IsDerivedFrom<T, Object>)
		{
			const Object& object = *reinterpret_cast<const Object*>(myPointer);
			if (const auto* chunk = object.myChunk)
				return chunk->FindGeneration(object.myChunkIndex);
			return 0;
		}
		else if constexpr (IsDerivedFrom<T, Entity>)
		{
			const Entity& entity = *reinterpret_cast<const Entity*>(myPointer);
			if (const auto* chunk = entity.myChunk)
				return chunk->FindGeneration(entity);
			return 0;
		}
		else
		{
			static_assert(false, "Ptr can only be used to point on entities, components and objects");
			abort();
		}
	}

	mutable T* myPointer;
	mutable u16 myGeneration;
};

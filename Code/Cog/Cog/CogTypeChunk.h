#pragma once
#include <Memory/InlineObject.h>
#include <Threading/Spinlock.h>
#include <Containers/TypeMap.h>
#include "Event.h"
#include "Impulse.h"

template <class T>
class Ptr;

class Event;
class Impulse;
class CogTypeChunk;
class Object;
class ProgramContext;

struct ScheduledEvent
{
	ScheduledEvent(InlineObject<Event> aEvent, const TypeID<Event>& aEventType)
		: event(Move(aEvent)),
		eventType(aEventType)
	{
	}

	InlineObject<Event> event;
	const TypeID<Event>& eventType;
};

struct ScheduledImpulse
{
	ScheduledImpulse(InlineObject<Impulse> aImpulse, const TypeID<Impulse>& aImpulseType, u8 aReceiverIndex, u8 aReceiverGeneration)
		: impulse(Move(aImpulse)),
		impulseType(aImpulseType),
		receiverIndex(aReceiverIndex),
		receiverGeneration(aReceiverGeneration)
	{
	}

	InlineObject<Impulse> impulse;
	const TypeID<Impulse>& impulseType;
	u8 receiverIndex;
	u8 receiverGeneration;
};

class CogTypeChunk
{
public:
	CogTypeChunk();
	virtual ~CogTypeChunk();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeChunk);

	FORCEINLINE ProgramContext& GetProgramContext() const { CHECK_PEDANTIC(myProgramContext); return *myProgramContext; }

	virtual void Initialize();

	FORCEINLINE u8 GetGeneration(const u8 aIndex) const { return myGeneration[aIndex]; }
	
	using EventBroadcastFunctionPtr = void(*)(const Event&, CogTypeChunk&, ArrayView<u8>);
	using ImpulseInvokerFunctionPtr = void(*)(const Event&, CogTypeChunk&, u8);
	virtual void GatherListeners(TypeMap<Event, EventBroadcastFunctionPtr>& aEventListeners, TypeMap<Impulse, ImpulseInvokerFunctionPtr>& aImpulseListeners) const = 0;

	template <typename T>
	void BroadcastEvent(T aEvent)
	{
		BroadcastEventById(InlineObject<Event>::New<T>(Move(aEvent)), TypeID<Event>::Resolve<T>());
	}

	void BroadcastEventById(InlineObject<Event> aEvent, const TypeID<Event>& aEventType);

	template <typename T>
	void SendImpulse(T aImpulse, const u8 aReceiver, const u8 aReceiverGeneration)
	{
		SendImpulseById(InlineObject<Impulse>::New<T>(Move(aImpulse)), TypeID<Impulse>::Resolve<T>(), aReceiver, aReceiverGeneration);
	}

	void SendImpulseById(InlineObject<Impulse> aImpulse, const TypeID<Impulse>& aImpulseType, u8 aReceiver, u8 aReceiverGeneration);

	Ptr<Object> Allocate();

	void SetOwner(u8 aIndex, const Ptr<Object>& aNewOwner);
	FORCEINLINE const Ptr<Object>& GetOwner(const u8 aIndex) const { reinterpret_cast<const Ptr<Object>&>(myOwners[aIndex].Get()); }

protected:
	template <typename T>
	friend class Ptr;

	friend class ChunkPool;
	void SetProgramContext(ProgramContext& aProgramContext);

	virtual UniquePtr<Object> CreateDefaultObject() const = 0;

	friend class ObjectPool;
	virtual void InitializeObjectAtIndex(u8 aIndex) = 0;
	virtual void DestructObjectAtIndex(u8 aIndex) = 0;

	template <typename T>
	static void DestructObjectHelper(T* aObject)
	{
		aObject->~T();
	}

private:
	TypeMap<Event, EventBroadcastFunctionPtr> myEventBroadcasters;
	TypeMap<Impulse, ImpulseInvokerFunctionPtr> myImpulseInvokers;

	void ScheduleTick();
	void Tick();

	Array<ScheduledImpulse> myScheduledImpulses;
	Array<ScheduledEvent> myScheduledEvents;

	Spinlock myMessageLock;

	bool OccupyFirstFreeSlot(u8& aFreeIndex);
	/** Returns number of elements filled in aOccupiedSlots output */
	u16 GatherOccupiedSlots(u8* aOccupiedSlots) const;

	friend class ObjectReturner;
	
	void ReturnByIndex(u8 aIndex);

#ifdef ENV64
	static constexpr i32 PtrSize = 24;
#elif defined(ENV32)
	constexpr i32 PtrSize = 20;
#else
#error "Unknown environment""
#endif

	UniquePtr<Object> myDefaultObject;
	ProgramContext* myProgramContext = nullptr;
	
	volatile u64 myFreeSlots[4];

	ManualInitializationObject<char[PtrSize]> myOwners[256];
	u8 myGeneration[256];
};

class ObjectReturner
{
public:
	FORCEINLINE ObjectReturner(CogTypeChunk& aChunk, const u8 aIndex)
	{
		aChunk.ReturnByIndex(aIndex);
	}
};

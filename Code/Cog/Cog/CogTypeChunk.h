#pragma once

template <class T>
class Ptr;

class Message;
class CogTypeChunk;
class Program;

struct EventDispatcherInfo
{
	void(*messageBroadcaster)(const Message&, CogTypeChunk*, ArrayView<u8>) = nullptr;
	void(*messageSender)(const Message&, CogTypeChunk*, u8) = nullptr;
	TypeID<Message>::CounterType typeId;
};

class CogTypeChunk
{
public:
	CogTypeChunk();
	virtual ~CogTypeChunk();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogTypeChunk);

	FORCEINLINE Program& GetProgram() const { CHECK_PEDANTIC(myProgram); return *myProgram; }

	virtual void Initialize();

	FORCEINLINE bool IsPendingDestroy(const u8 aIndex) const { return myIsPendingDestroy[aIndex]; }

	void MarkPendingDestroy(u8 aIndex);
	
	FORCEINLINE u8 GetGeneration(const u8 aIndex) const { return myGeneration[aIndex]; }
	
	virtual Array<EventDispatcherInfo> GetInterestingEvents() const { return Array<EventDispatcherInfo>(); }

	template <typename T>
	void BroadcastMessage(const T& aMessage)
	{
		BroadcastMessageById(aMessage, TypeID<Message>::Resolve<T>().GetUnderlyingInteger());
	}

	void BroadcastMessageById(const Message& aMessage, TypeID<Message>::CounterType aIndex);

	template <typename T>
	void SendMessage(const T& aMessage, const u8 aReceiver)
	{
		SendMessageById(aMessage, TypeID<Message>::Resolve<T>().GetUnderlyingInteger(), aReceiver);
	}

	void SendMessageById(const Message& aMessage, TypeID<Message>::CounterType aIndex, u8 aReceiver);

	Ptr<Object> Allocate(const Ptr<Object>& aOwner, bool aIsRootInstance);

	void SetOwner(u8 aIndex, const Ptr<Object>& aNewOwner);
	FORCEINLINE const Ptr<Object>& GetOwner(const u8 aIndex) const { reinterpret_cast<const Ptr<Object>&>(myOwners[aIndex].Get()); }

protected:
	template <typename T>
	friend class Ptr;

	friend class ObjectPool;
	void SetProgram(Program& aProgram);

	virtual UniquePtr<Object> CreateDefaultObject() const;
	virtual void InitializeObjectAtIndex(u8 aIndex);
	virtual void DestructObjectAtIndex(u8 aIndex);

	template <typename T>
	static void DestructObjectHelper(T* aObject)
	{
		aObject->~T();
	}

private:
	Array<void(*)(const Message&, CogTypeChunk*, ArrayView<u8>)> myMessageBroadcasters;
	Array<void(*)(const Message&, CogTypeChunk*, u8)> myMessageSenders;

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
	Program* myProgram = nullptr;
	
	volatile u64 myFreeSlots[4];

	ManualInitializationObject<char[PtrSize]> myOwners[256];
	bool myIsPendingDestroy[256];
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

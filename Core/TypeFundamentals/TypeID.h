#pragma once

template <typename TBase>
class TypeID
{
	static u16 ourIDCounter;

	u16 myID = 0;

	explicit TypeID(u16 aID)
	{
		myID = aID;
	}

public:
	using CounterType = u16;
	
	TypeID()
	{
	}

	static u16 MaxUnderlyingInteger()
	{
		return ourIDCounter;
	}

	template <typename TType>
	static TypeID Resolve()
	{
		static u16 resolvedID = ourIDCounter++;
		return TypeID(resolvedID);
	}

	u16 GetUnderlyingInteger() const { return myID; }

	bool operator==(TypeID aOther)
	{
		return myID == aOther.myID;
	}

	bool operator!=(TypeID aOther)
	{
		return myID != aOther.myID;
	}
};

template <typename TBase>
u16 TypeID<TBase>::ourIDCounter = 1;

#pragma once

template <typename TBase>
class TypeID
{
	static u16 ourIDCounter;

	u16 myID = 0;
	const std::type_info& myTypeInfo;

	explicit TypeID(u16 aID, const std::type_info& aTypeInfo)
		: myTypeInfo(aTypeInfo)
	{
		myID = aID;
	}
	
	~TypeID() = default;

public:
	using CounterType = u16;
	
	TypeID() = delete;
	TypeID(const TypeID&) = delete;
	TypeID& operator=(const TypeID&) = delete;
	TypeID(TypeID&&) = delete;
	TypeID& operator=(TypeID&&) = delete;

	FORCEINLINE static u16 MaxUnderlyingInteger() { return ourIDCounter; }

	template <typename TType>
	static const TypeID& Resolve()
	{
		static_assert(IsDerivedFrom<TType, TBase>, "Invalid inheritance chain");
		static const TypeID type(ourIDCounter++, typeid(TType));
		return type;
	}

	FORCEINLINE u16 GetUnderlyingInteger() const { return myID; }
	FORCEINLINE const char* GetTypeInfoName() const { return myTypeInfo.name(); }

	bool operator==(const TypeID& aOther)
	{
		return myID == aOther.myID;
	}

	bool operator!=(const TypeID& aOther)
	{
		return myID != aOther.myID;
	}
};

template <typename TBase>
u16 TypeID<TBase>::ourIDCounter = 1;

#pragma once
#include <TypeFundamentals/TypeID.h>
#include <TypeFundamentals/TypeTraits.h>

class CogTypeBase;

template <typename T>
class Class
{
public:
	Class()
	{
		myType = &TypeID<CogTypeBase>::Resolve<T>();
	}

	explicit Class(const TypeID<CogTypeBase>& aType)
	{
		myType = &aType;
	}
	
	Class(Class&&) = delete;
	Class operator=(Class&&) = delete;

	Class(const Class&) = delete;
	Class operator=(const Class&) = delete;

	template <typename TOtherType, EnableIf<IsDerivedFrom<TOtherType, T>, int> = 0>
	Class(const Class<TOtherType>& aCopy);

	template <typename TOtherType, EnableIf<IsDerivedFrom<TOtherType, T>, int> = 0>
	Class& operator=(const Class<TOtherType>& aCopy);

	const TypeID<CogTypeBase>& GetTypeID() const
	{
		return *myType;
	}

private:
	template <typename TOtherType>
	friend class Class;

	const TypeID<CogTypeBase>* myType;
};

template <typename T>
template <typename TOtherType, EnableIf<IsDerivedFrom<TOtherType, T>, int>>
Class<T>::Class(const Class<TOtherType>& aCopy)
{
	myType = aCopy.myType;
}

template <typename T>
template <typename TOtherType, EnableIf<IsDerivedFrom<TOtherType, T>, int>>
Class<T>& Class<T>::operator=(const Class<TOtherType>& aCopy)
{
	myType = aCopy.myType;
	return *this;
}

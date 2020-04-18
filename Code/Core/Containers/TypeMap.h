#pragma once

template <typename TBase, typename TValue>
class TypeMap
{
public:
	TypeMap()
	{
		myValues.Resize(32);
	}

	TValue& operator[](const TypeID<TBase>& aType)
	{
		const auto index = aType.GetUnderlyingInteger();

		if (index >= myValues)
			myValues.Resize(CeilToPowerTwo(index));

		return myValues[index];
	}

	template <typename T>
	TValue& FindOrAdd()
	{
		return (*this)[TypeID<TBase>::template Resolve<T>()];
	}

private:
	Array<TValue> myValues;
};

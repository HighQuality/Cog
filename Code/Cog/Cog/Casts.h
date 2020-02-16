#pragma once

template <typename TTo, typename TFrom>
FORCEINLINE TTo* Cast(TFrom* aFrom)
{
	return dynamic_cast<TTo*>(aFrom);
}

template <typename TTo, typename TFrom>
FORCEINLINE const TTo* Cast(const TFrom* aFrom)
{
	return Cast<const TTo>(const_cast<TFrom*>(aFrom));
}

template <typename TTo, typename TFrom>
FORCEINLINE TTo* Cast(TFrom& aFrom)
{
	return Cast<TTo>(&aFrom);
}

template <typename TTo, typename TFrom>
FORCEINLINE const TTo* Cast(const TFrom& aFrom)
{
	return Cast<const TTo>(const_cast<TFrom*>(&aFrom));
}

template <typename TTo, typename TFrom>
FORCEINLINE TTo& CheckedCast(TFrom& aFrom)
{
	TTo* castedObject = Cast<TTo>(&aFrom);
	CHECK(castedObject);
	return *castedObject;
}

template <typename TTo, typename TFrom>
FORCEINLINE const TTo& CheckedCast(const TFrom& aFrom)
{
	return CheckedCast<const TTo>(const_cast<TFrom&>(aFrom));
}

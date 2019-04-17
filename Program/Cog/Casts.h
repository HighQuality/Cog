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
FORCEINLINE TTo& CastChecked(TFrom& aFrom)
{
	TTo* castedObject = Cast<TTo>(&aFrom);
	CHECK(castedObject);
	return *castedObject;
}

template <typename TTo, typename TFrom>
FORCEINLINE const TTo& CastChecked(const TFrom& aFrom)
{
	return CastChecked<const TTo>(const_cast<TFrom&>(aFrom));
}

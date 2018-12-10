#pragma once

template <u64 NumElements>
class Bitfield
{
public:
	Bitfield()
	{
		for (u64 i = 0; i < ArrayLength; ++i)
			myData[i] = 0;
	}

	u64 SetFirstZero()
	{
		unsigned long index;
		for (u64 i = 0; i < ArrayLength; ++i)
		{
			if (_BitScanReverse64(&index, ~myData[i]))
			{
				if (i + 1 == ArrayLength && i * 64 + index >= NumElements)
					FATAL();
				_bittestandset64(myData[i], index);
				return index;
			}
		}
		FATAL();
	}

	bool IsBitSet(const u64 aIndex) const
	{
		const u64 integer = aIndex / 64;
		const u64 element = aIndex % 64;
		if (integer > ArrayLength)
			FATAL();
		return _bittest64(myData[integer], element);
	}

private:
	static constexpr u64 ArrayLength = NumElements / 64 + (NumElements % 64 != 0 ? 1 : 0);
	u64 myData[ArrayLength];
};

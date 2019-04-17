#pragma once
#include "snappy/snappy.h"

template <typename T>
void CopyToArray(const T& aValue, Array<u8>& aTarget)
{
	static_assert(IsNumber<T>, "Only numbers can be copied to arrays with the default CopyToArray implementation, please specialize");
	aTarget.Append(ArrayView<u8>(reinterpret_cast<const u8*>(&aValue), sizeof T));
}

template <typename T>
void ReadFromArray(const Array<u8>& aSource, i32& aReadHead, T& aValue)
{
	static_assert(IsNumber<T>, "Only numbers can be read from arrays with the default ReadFromArray implementation, please specialize");

	aValue = *reinterpret_cast<const T*>(aSource.Slice(aReadHead, sizeof T).GetData());
	aReadHead += sizeof T;
}

template <typename T>
void CopyToArray(const ArrayView<T>& aValue, Array<u8>& aTarget)
{
	CopyToArray<i32>(aValue.GetLength(), aTarget);
	for (i32 i = 0; i < aValue.GetLength(); ++i)
		CopyToArray<T>(aValue[i], aTarget);
}

template <typename T>
void CopyToArray(const ArraySlice<T>& aValue, Array<u8>& aTarget) { CopyToArray(static_cast<const ArrayView<T>&>(aValue), aTarget); }

template <typename T>
void CopyToArray(const Array<T>& aValue, Array<u8>& aTarget) { CopyToArray(static_cast<const ArrayView<T>&>(aValue), aTarget); }

template <>
inline void CopyToArray<StringView>(const StringView& aValue, Array<u8>& aTarget) { CopyToArray(aValue.View(), aTarget); }

template <>
inline void CopyToArray<StringSlice>(const StringSlice& aValue, Array<u8>& aTarget) { CopyToArray(aValue.View(), aTarget); }

template <>
inline void CopyToArray<String>(const String& aValue, Array<u8>& aTarget) { CopyToArray(aValue.View(), aTarget); }

template <typename T>
void ReadFromArray(const Array<u8>& aSource, i32& aReadHead, Array<T>& aValue)
{
	i32 length;
	ReadFromArray<i32>(aSource, aReadHead, length);
	aValue.Clear();
	T val;
	for (i32 i = 0; i < length; ++i)
	{
		ReadFromArray<T>(aSource, aReadHead, val);
		aValue.Add(Move(val));
	}
}

template <>
inline void ReadFromArray<String>(const Array<u8>& aSource, i32& aReadHead, String& aValue)
{
	i32 length;
	ReadFromArray<i32>(aSource, aReadHead, length);
	aValue.Clear();
	Char val;
	for (i32 i = 0; i < length; ++i)
	{
		ReadFromArray<Char>(aSource, aReadHead, val);
		aValue.Add(val);
	}
}

class BinaryData
{
public:
	void WriteRaw(const void* aData, const i32 aSize)
	{
		myData.Append(ArrayView<u8>(static_cast<const u8*>(aData), aSize));
	}

	template <typename T>
	void Write(const T& aValue)
	{
		CopyToArray(aValue, myData);
	}

	template <typename T>
	T Read()
	{
		T val;
		ReadFromArray(myData, myReadHead, val);
		return val;
	}

	ArrayView<u8> ReadRaw(const i32 aSize)
	{
		auto slice = myData.Slice(myReadHead, aSize);
		myReadHead += aSize;
		return slice;
	}

	void Compress()
	{
		Array<u8> newData;
		newData.Resize(OverflowCheckedCast<i32>(snappy::MaxCompressedLength(myData.GetLength())));
		size_t newLength;
		snappy::RawCompress(reinterpret_cast<const char*>(myData.GetData()), myData.GetLength(), reinterpret_cast<char*>(newData.GetData()), &newLength);
		newData.Resize(OverflowCheckedCast<i32>(newLength));
		myData = Move(newData);
	}

	bool Decompress()
	{
		size_t uncompressedLength;
		if (!snappy::GetUncompressedLength(reinterpret_cast<const char*>(myData.GetData()), myData.GetLength(), &uncompressedLength))
			return false;
		Array<u8> newData;
		newData.Resize(OverflowCheckedCast<i32>(uncompressedLength));
		if (!snappy::RawUncompress(reinterpret_cast<const char*>(myData.GetData()), myData.GetLength(), reinterpret_cast<char*>(newData.GetData())))
			return false;
		myData = Move(newData);
		return true;
	}

	i32 GetLength() const
	{
		return myData.GetLength();
	}

private:
	Array<u8> myData;
	i32 myReadHead = 0;
};

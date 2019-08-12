#pragma once

template <typename T>
class Optional
{
public:
	static Optional Empty;

	static Optional MakeWithValue(const T & aValue)
	{
		return Optional(aValue);
	}

	operator const T&() const
	{
		return GetValue();
	}

	explicit operator bool() const
	{
		return HasValue();
	}

	const T& operator*() const
	{
		return GetValue();
	}

	bool HasValue() const
	{
		return myHasValue;
	}

	const T & GetValue() const
	{
		if (!HasValue())
			abort();
		return myValue;
	}

private:
	Optional()
	{
	}

	Optional(const T & aValue)
	{
		myValue = aValue;
		myHasValue = true;
	}

	T myValue = std::remove_reference_t<T>();
	bool myHasValue = false;
};

template <typename T>
Optional<T> Optional<T>::Empty;

#pragma once

template <typename T>
class Stack
{
public:
	void Push(T aElement)
	{
		myData.Add(Move(aElement));
	}

	T Pop()
	{
		T element = Move(myData.Last());
		myData.RemoveAt(myData.GetLength() - 1);
		return element;
	}

	bool TryPop(T& aElement)
	{
		if (myData.GetLength() == 0)
			return false;
		aElement = Move(myData.Last());
		myData.RemoveAt(myData.GetLength() - 1);
		return true;
	}

	i32 GetLength() const
	{
		return myData.GetLength();
	}

private:
	Array<T> myData;
};

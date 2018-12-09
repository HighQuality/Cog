#pragma once

template <typename TSignature>
class Function;

template <typename TReturn, typename... TArgs>
class Function<TReturn(TArgs...)>
{
	const void* myObject;
	TReturn(*myCaller)(const void*, TArgs...);
	void(*myDeleter)(const void*);

public:
	Function()
	{
		myObject = nullptr;
		myCaller = nullptr;
		myDeleter = nullptr;
	}

	Function(Function&& aMove) noexcept
	{
		myObject = aMove.myObject;
		aMove.myObject = nullptr;

		myCaller = aMove.myCaller;
		aMove.myCaller = nullptr;

		myDeleter = aMove.myDeleter;
		aMove.myDeleter = nullptr;
	}

	Function& operator=(Function&& aMove)
	{
		if (myDeleter)
			myDeleter(myObject);

		myObject = aMove.myObject;
		aMove.myObject = nullptr;

		myCaller = aMove.myCaller;
		aMove.myCaller = nullptr;

		myDeleter = aMove.myDeleter;
		aMove.myDeleter = nullptr;
	}

	Function(const Function& aCopy) = delete;
	Function& operator=(const Function& aCopy) = delete;

	template <typename T>
	Function(T aFunctor) noexcept
	{
		myObject = new T(Move(aFunctor));

		myCaller = [](const void* aObject, TArgs... aArgs) -> TReturn {
			return (*static_cast<const T*>(aObject))(
				std::forward<TArgs>(aArgs)...
			);
		};

		myDeleter = [](const void* aObject) {
			delete static_cast<const T*>(aObject);
		};
	}

	~Function()
	{
		if (myDeleter)
			myDeleter(myObject);
	}

	TReturn operator()(TArgs... aArgs) const
	{
		CHECK(myObject && myCaller);
		return myCaller(myObject, std::forward<TArgs>(aArgs)...);
	}
};

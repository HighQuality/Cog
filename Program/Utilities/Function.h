#pragma once

template <typename TSignature>
class Function;

template <typename TReturn, typename... TArgs>
class Function<TReturn(TArgs...)>
{
	class BaseHelper
	{
	public:
		virtual ~BaseHelper() = default;
		virtual TReturn Call(TArgs...) const = 0;
		virtual BaseHelper* Copy() const = 0;
	};

	template <typename T>
	class Helper final : public BaseHelper
	{
	public:
		Helper(T&& aFunction)
			: myFunction(Move(aFunction))
		{
		}

		TReturn Call(TArgs... aArgs) const final
		{
			return myFunction(std::forward<TArgs>(aArgs)...);
		}
		
		BaseHelper* Copy() const final
		{
			return new Helper(T(myFunction));
		}

	private:
		T myFunction;
	};

	BaseHelper* myFunction = nullptr;

public:
	Function()
	{
	}

	Function(Function&& aMove) noexcept
	{
		myFunction = aMove.myFunction;
		aMove.myFunction = nullptr;
	}

	Function& operator=(Function&& aMove)
	{
		if (myFunction)
			delete myFunction;

		myFunction = aMove.myFunction;
		aMove.myObject = nullptr;
	}

	Function(const Function& aCopy)
	{
		if (aCopy.myFunction)
			myFunction = aCopy.myFunction->Copy();
		else
			myFunction = nullptr;
	}
	
	Function& operator=(const Function& aCopy)
	{
		if (aCopy.myFunction)
			myFunction = aCopy.myFunction->Copy();
		else
			myFunction = nullptr;
		return *this;
	}

	template <typename T>
	Function(T aFunctor) noexcept
		: myFunction(new Helper<T>(Move(aFunctor)))
	{
	}

	~Function()
	{
		delete myFunction;
		myFunction = nullptr;
	}

	FORCEINLINE bool IsValid() const
	{
		return myFunction != nullptr;
	}

	FORCEINLINE TReturn operator()(TArgs... aArgs) const
	{
		CHECK(IsValid());
		return myFunction->Call(std::forward<TArgs>(aArgs)...);
	}
};

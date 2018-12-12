#pragma once

template <typename TType>
class BaseObjectFunctionView;

template <typename TReturn, typename ...TArgs>
class BaseObjectFunctionView<TReturn(TArgs...)>
{
public:
	virtual ~BaseObjectFunctionView() = default;
	
	virtual TReturn Call(TArgs ...aArgs) const = 0;
};

template <typename TType, typename TFunc>
class ObjectFunctionView;

template <typename TObject, typename TReturn, typename ...TArgs>
class ObjectFunctionView<TObject, TReturn(TArgs...)> final : public BaseObjectFunctionView<TReturn(TArgs...)>
{
public:
	ObjectFunctionView() = default;

	ObjectFunctionView(TObject& aObject, TReturn (TObject::*aFunction)(TArgs ...))
	{
		myObject = aObject;
		myFunction = aFunction;
	}

	explicit operator bool() const { return IsValid(); }

	bool IsValid() const
	{
		return myObject.IsValid() && myFunction;
	}

	TReturn Call(TArgs ...aArgs) const final
	{
		CHECK(IsValid());
		return ((*myObject).*myFunction)(aArgs...);
	}

private:
	Ptr<TObject> myObject;
	TReturn (TObject::*myFunction)(TArgs ...) = nullptr;
};

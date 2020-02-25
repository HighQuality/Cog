#pragma once

template <typename TSignature>
class FunctionView;

template <typename TReturn, typename... TArgs>
class FunctionView<TReturn(TArgs...)>
{
    const void* myObject;
    TReturn (*myCaller)(const void*, TArgs&&...);

public:
    FunctionView()
    {
		myObject = nullptr;
		myCaller = nullptr;
    }
	
	template <typename T>
    FunctionView(T&& aFunctor) noexcept
    {
		myObject = std::addressof(aFunctor);

        myCaller = [](const void* aObject, TArgs&&... aArgs) -> TReturn {
            return (*static_cast<const T*>(aObject))(
                Forward<TArgs>(aArgs)...
			);
        };
    }

    TReturn operator()(TArgs&&... aArgs) const
    {
		CHECK(myObject && myCaller);
        return myCaller(myObject, Forward<TArgs>(aArgs)...);
    }
};

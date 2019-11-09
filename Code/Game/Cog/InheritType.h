#pragma once

class Object;

template <typename TBase>
class InheritType : public TBase
{
public:
	using Base = TBase;

	void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const override
	{
		aFunction(TypeID<Object>::Resolve<TBase>());

		Base::GetBaseClasses(aFunction);
	}
};

#pragma once

template <typename TComponent>
class InheritComponent : public TComponent
{
public:
	using Base = TComponent;

	void GetBaseClasses(const FunctionView<void(const TypeID<Component>&)>& aFunction) const override
	{
		aFunction(TypeID<Component>::Resolve<TComponent>());

		Base::GetBaseClasses(aFunction);
	}
};

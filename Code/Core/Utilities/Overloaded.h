#pragma once

template<class... TArgs>
struct Overloaded : TArgs...
{
	using TArgs::operator()...;
};

template<class... TArgs>
Overloaded(TArgs...) -> Overloaded<TArgs...>;

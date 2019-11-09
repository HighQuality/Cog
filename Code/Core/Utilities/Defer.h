#pragma once

template <typename T>
struct Defer
{
	Defer(T f) : f(f) {}
	~Defer() { f(); }
	T f;
};

template <typename T>
Defer<T> MakeDefer(T f)
{
	return Defer<T>(f);
};

#define __defer( line ) defer_ ## line
#define _defer( line ) __defer( line )

struct DeferDummy { };
template<typename T>
Defer<T> operator+(DeferDummy, T&& f)
{
	return MakeDefer<T>(std::forward<T>(f));
}

#define defer auto _defer( __LINE__ ) = DeferDummy( ) + [ & ]( )

#pragma once

template <typename T>
struct Defer
{
	FORCEINLINE Defer(T f) : f(f) {}
	FORCEINLINE ~Defer() { f(); }
	
	FORCEINLINE explicit operator bool() const { return true; }
	
	T f;
};

template <typename T>
FORCEINLINE Defer<T> MakeDefer(T f)
{
	return Defer<T>(f);
};

struct DeferDummy { };
template<typename T>
FORCEINLINE Defer<T> operator+(DeferDummy, T&& f)
{
	return MakeDefer<T>(std::forward<T>(f));
}

// TODO: Change to uppercase
#define defer auto JOIN(__defer_, __LINE__ ) = DeferDummy( ) + [ & ]( )

// TODO: Change to uppercase
#define scoped_lock(mutex) \
	mutex.lock(); \
	if (defer { mutex.unlock(); })

#pragma once

template <typename T>
struct Defer
{
	Defer(T f) : f(f) {}
	~Defer() { f(); }
	
	explicit operator bool() const { return true; }
	
	T f;
};

template <typename T>
Defer<T> MakeDefer(T f)
{
	return Defer<T>(f);
};

struct DeferDummy { };
template<typename T>
Defer<T> operator+(DeferDummy, T&& f)
{
	return MakeDefer<T>(std::forward<T>(f));
}

// TODO: Change to uppercase
#define defer auto JOIN(__defer_, __LINE__ ) = DeferDummy( ) + [ & ]( )

// TODO: Change to uppercase
#define scoped_lock(mutex) \
	mutex.lock(); \
	if (defer { mutex.unlock(); })

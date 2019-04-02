#include "pch.h"
#include "ClientGame.h"
#include <Program.h>

class EventType;

template <typename ...TEventTypes>
Array<u16> GetEventTypes()
{
	return Array<u16> { TypeID<EventType>::Resolve<TEventTypes>().GetUnderlyingInteger()... };
}

#define DECLARE_GET_EVENT_LISTENERS(...) Array<u16> GetEventListeners() { return GetEventTypes<__VA_ARGS__>(); }
#define DECLARE_EVENT_LISTENERS_2(First, ...) DECLARE_EVENT_LISTENERS_2(__VA_ARGS__); 
#define DECLARE_EVENT_LISTENERS(...) DECLARE_EVENT_LISTENERS_2(__VA_ARGS__); DECLARE_GET_EVENT_LISTENERS(First, __VA_ARGS__);
 
class Frame;
class ResolutionChanged;

struct Test
{
	DECLARE_EVENT_LISTENERS(Frame);
};

int main()
{
	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();

	return 0;
}

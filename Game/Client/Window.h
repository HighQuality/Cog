#pragma once
#include "WindowEvent.h"

class Window
{
public:
	Window();
	~Window();

	void SetTitle(StringView aTitle);

	void ProcessMessages();

	bool PollMessage(WindowEvent& event);

	void Open();

	bool IsOpen() const { return myIsOpen; }

	void Close();

	void SetVisible(bool aIsVisible);

	void RequestFocus();
	bool HasFocus() const { return myHasFocus; }

	bool ReceiveMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam);

	HWND GetHandle() const { return myHandle; }

	i32 GetWidth() const { return myWidth; }
	i32 GetHeight() const { return myHeight; }

private:
	void LateParseMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam);

	Array<WindowEvent> queuedEvents;

	i32 myWidth;
	i32 myHeight;

	String myTitle;

	HWND myHandle{};
	bool myIsOpen;
	bool myHasFocus;
	WNDCLASSEXW myWindowClass{};

	i32 myScheduledNewWidth = 0;
	i32 myScheduledNewHeight = 0;
	bool myIsInResizeMode = false;
};

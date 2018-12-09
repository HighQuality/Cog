#pragma once
#include <Windows.h>

class Window
{
public:
	Window();
	~Window();

	void SetTitle(StringView aTitle);

	void ProcessMessages();

	void Open();

	bool IsOpen() const { return myIsOpen; }

	void Close();

	void SetVisible(bool aIsVisible);

	void* GetHandle() const;

	void RequestFocus();
	bool HasFocus() const { return myHasFocus; }

	bool ReceiveMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam);

private:
	void LateParseMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam);
	
	i32 myWidth;
	i32 myHeight;

	String myTitle;

	HWND myHandle{};
	bool myIsOpen;
	bool myHasFocus;
	WNDCLASSEXW myWindowClass{};
};

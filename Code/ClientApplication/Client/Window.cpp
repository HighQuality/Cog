#include "ClientApplicationPch.h"
#include "Window.h"
#include <Windowsx.h>
#include <shellapi.h>

LRESULT CALLBACK WindowProc(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam)
{
	Window* wnd = reinterpret_cast<Window*>(GetWindowLongPtr(aHwnd, GWLP_USERDATA));

	if (wnd)
	{
		if (wnd->ReceiveMessage(aHwnd, aMessage, aWParam, aLParam))
			return 0;
	}

	switch (aMessage)
	{
	case WM_DESTROY:
	case WM_QUIT:
		PostQuitMessage(0);
		return 0;

	case WM_SYSCOMMAND:
		if ((aWParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	}

	return DefWindowProcW(aHwnd, aMessage, aWParam, aLParam);
}

Window::Window()
{
	myIsOpen = false;
	myHasFocus = true;
	myWidth = 1280;
	myHeight = 720;
	myTitle = String(L"Cog");
}

Window::~Window()
{
	if (IsOpen())
		Close();
}

void Window::SetTitle(StringView aTitle)
{
	myTitle = String(aTitle);

	if (IsOpen())
	{
		SetWindowTextW(myHandle, myTitle.GetData());
	}
}

void Window::ProcessMessages()
{
	if (!ENSURE(IsOpen()))
		return;

	MSG msg = { 0 };

	while (PeekMessageW(&msg, myHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		LateParseMessage(myHandle, msg.message, msg.wParam, msg.lParam);
	}
}

bool Window::PollMessage(WindowEvent& event)
{
	if (queuedEvents.GetLength() == 0)
		return false;
	event = queuedEvents.Pop();
	return true;
}

void Window::Open()
{
	if (!ENSURE(!IsOpen()))
		return;

	HINSTANCE mHandle = GetModuleHandle(nullptr);

	DWORD windowStyle = WS_OVERLAPPEDWINDOW;

	// From Sandbucket, disables resizing outside of development, although it needs to disable maximizing too
	// #ifdef PUBLISH
	// 	windowStyle ^= WS_THICKFRAME;
	// #endif

	RECT rect;
	rect.left = 0;
	rect.right = myWidth;
	rect.top = 0;
	rect.bottom = myHeight;

	AdjustWindowRect(&rect, windowStyle, false);

	memset(&myWindowClass, 0, sizeof(WNDCLASSEX));
	myWindowClass.cbSize = sizeof(WNDCLASSEX);
	myWindowClass.style = CS_HREDRAW | CS_VREDRAW;
	myWindowClass.lpfnWndProc = WindowProc;
	myWindowClass.hInstance = mHandle;
	myWindowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	myWindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	myWindowClass.lpszClassName = L"WindowClass";
	myWindowClass.hIcon = ::LoadIcon(mHandle, MAKEINTRESOURCE(101));
	myWindowClass.hIconSm = LoadIcon(mHandle, MAKEINTRESOURCE(101));
	RegisterClassExW(&myWindowClass);

	
	myHandle = CreateWindowExW(WS_EX_ACCEPTFILES, L"WindowClass",
		myTitle.GetData(),
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr, nullptr, mHandle, nullptr);

	myIsOpen = true;

	SetWindowLongPtrW(myHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

void Window::Close()
{
	if (!ENSURE(IsOpen()))
		return;

	CloseWindow(myHandle);
	myHandle = nullptr;
	myIsOpen = false;
}

void Window::SetVisible(bool aIsVisible)
{
	if (!ENSURE(IsOpen()))
		return;
	ShowWindow(myHandle, aIsVisible ? SW_SHOW : SW_HIDE);
}

void Window::RequestFocus()
{
	SetFocus(myHandle);
	myHasFocus = true;
}

bool Window::ReceiveMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam)
{
	WindowEvent event;
	event.type = WindowEventType::None;

	switch (aMessage)
	{
	case WM_SETFOCUS:
		myHasFocus = true;
		break;

	case WM_KILLFOCUS:
		myHasFocus = false;
		break;

	case WM_CLOSE:
		event.type = WindowEventType::Close;
		break;

	case WM_ENTERSIZEMOVE:
		myIsInResizeMode = true;
		break;

	case WM_EXITSIZEMOVE:
		if (myIsInResizeMode)
		{
			if (myScheduledNewWidth > 0 && myScheduledNewHeight > 0)
			{
				event.type = WindowEventType::Resize;
				event.data.resize.newWidth = myScheduledNewWidth;
				event.data.resize.newHeight = myScheduledNewHeight;
			}

			myScheduledNewWidth = 0;
			myScheduledNewHeight = 0;

			myIsInResizeMode = false;
		}
		break;

	case WM_SIZE:
		if (myIsInResizeMode)
		{
			myScheduledNewWidth = LOWORD(aLParam);
			myScheduledNewHeight = HIWORD(aLParam);
		}
		else
		{
			event.type = WindowEventType::Resize;
			event.data.resize.newWidth = LOWORD(aLParam);
			event.data.resize.newHeight = HIWORD(aLParam);
		}
		break;

	}

	if (event.type != WindowEventType::None)
	{
		queuedEvents.Add(event);
	}

	return false;
}

void Window::LateParseMessage(HWND aHwnd, UINT aMessage, WPARAM aWParam, LPARAM aLParam)
{
	switch (aMessage)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		SetCapture(myHandle);
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		SetCapture(nullptr);
		break;
	}
}

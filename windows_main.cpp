#include <Windows.h>
#include <cstdint>
#include "game.cpp"
static bool gameRunning;

struct Vector2 {
    int x;
    int y;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT returnVal = 0;
    switch (uMsg)
    {
		case WM_SIZE:
		{

		} break;
        case WM_CLOSE: {
            gameRunning = false;
            DestroyWindow(hwnd);
        } break;
        default: {
            returnVal = DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return returnVal;
}

// hInstance: handle to the .exe
// hPrevInstance: not used since 16bit windows
// WINAPI: calling convention, tells compiler order of parameters
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    gameRunning = true;

    Vector2 resolution = { 640, 704 };
    Vector2 origin = { (1920 - resolution.x)/2, (1080 - resolution.y) / 2 }; // TODO: query screen resolution instead of hardcoding 1920x1080.
    BITMAPINFO bitmapInfo;
    BITMAPINFOHEADER bmInfoHeader = {};
    bmInfoHeader.biSize = sizeof(bmInfoHeader);
    bmInfoHeader.biCompression = BI_RGB;
    bmInfoHeader.biWidth = resolution.x;
    bmInfoHeader.biHeight = -resolution.y; // Negative means it'll be filled top-down
    bmInfoHeader.biPlanes = 1;       // MSDN sais it must be set to 1, legacy reasons
    bmInfoHeader.biBitCount = 32;    // R+G+B+padding each 8bits
    bitmapInfo.bmiHeader = bmInfoHeader;

    void* bitMapMemory;
    bitMapMemory = VirtualAlloc(0, resolution.x * resolution.y * 4 /*(32bits for color)*/, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Elevator";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = (LPCSTR)CLASS_NAME;

	RegisterClass(&wc);
    RECT desiredClientSize;
    desiredClientSize.left = 0;
    desiredClientSize.right = resolution.x;
    desiredClientSize.top = 0;
    desiredClientSize.bottom = resolution.y;

    DWORD windowStyles = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    AdjustWindowRectEx(&desiredClientSize, windowStyles, false, 0);
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        (LPCSTR)CLASS_NAME,                     // Window class
        "Elevator",    // Window text
        windowStyles,            // Window style

        // Size and position
        origin.x, origin.y, desiredClientSize.right - desiredClientSize.left, desiredClientSize.bottom - desiredClientSize.top,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    if (hwnd) {
        HDC windowDeviceContext = GetDC(hwnd);

        ShowWindow(hwnd, nShowCmd);

        while (gameRunning) {

            // Process Messages
            MSG msg = { };
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                switch (msg.message) {
                case WM_QUIT: {
                    gameRunning = false;
                } break;
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                } break;
                }

            }

            // Render
            updateAndRender(bitMapMemory, resolution.x, resolution.y);
            StretchDIBits(windowDeviceContext, 0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, bitMapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        }
    }

	return 0;
}
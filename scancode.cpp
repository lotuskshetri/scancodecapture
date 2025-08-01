#include <windows.h>
#include <stdio.h>
#include <iostream>

HHOOK g_hHook = NULL;

// Low-level keyboard hook procedure
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        
        // Extract scancode - note: in low-level hooks, scanCode is directly available
        unsigned int scancode = pKbdStruct->scanCode;
        unsigned int extended = (pKbdStruct->flags & LLKHF_EXTENDED) ? 1 : 0;
        
        // Apply the same logic as the original code
        if (extended) {
            if (scancode != 0x45) {
                scancode |= 0xE000;
            }
        } else {
            if (scancode == 0x45) {
                scancode = 0xE11D45;
            } else if (scancode == 0x54) {
                scancode = 0xE037;
            }
        }
        
        // Display the scancode
        const char* msgType = "";
        switch (wParam) {
            case WM_KEYDOWN: msgType = "KEYDOWN"; break;
            case WM_KEYUP: msgType = "KEYUP"; break;
            case WM_SYSKEYDOWN: msgType = "SYSKEYDOWN"; break;
            case WM_SYSKEYUP: msgType = "SYSKEYUP"; break;
        }
        
        printf("%s: VK=0x%02X, Scancode=0x%X, Extended=%d, Time=%lu\n",
               msgType, pKbdStruct->vkCode, scancode, extended, pKbdStruct->time);
        
        // Check for exit condition (Ctrl+Shift+Q)
        if (pKbdStruct->vkCode == 'Q' && 
            (GetAsyncKeyState(VK_CONTROL) & 0x8000) && 
            (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
            printf("Exit key combination pressed. Unhooking...\n");
            PostQuitMessage(0);
        }
    }
    
    // Call next hook
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

int main() {
    // Allocate console
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    
    printf("Global Scancode Hook Started!\n");
    printf("This will capture ALL keyboard input system-wide.\n");
    printf("Press Ctrl+Shift+Q to exit.\n\n");
    
    // Install low-level keyboard hook
    g_hHook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        LowLevelKeyboardProc,
        GetModuleHandle(NULL),
        0
    );
    
    if (g_hHook == NULL) {
        printf("Failed to install hook! Error: %lu\n", GetLastError());
        return 1;
    }
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    if (g_hHook) {
        UnhookWindowsHookEx(g_hHook);
    }
    
    printf("Hook removed. Exiting...\n");
    return 0;
}

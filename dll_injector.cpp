#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

#include <windows.h>

#include "MainUnit.h"
#include "injector.h"

#include <System.SysUtils.hpp>
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>

using dll_injector::Injector;

static Injector create_injector();
static uint64_t get_load_library_x64_addr();
static uint32_t get_load_library_x32_addr();

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
    try {
        Application->Initialize();
        Injector injector = create_injector();
        Application->MainFormOnTaskBar = true;
        TStyleManager::TrySetStyle("Tablet Light");
        Application->CreateForm(__classid(TMainForm), &MainForm);
        MainForm->set_injector(&injector);
        Application->Run();
    } catch (Exception &exception) {
        Application->ShowException(&exception);
    } catch (...) {
        try {
            throw Exception("");
        } catch (Exception &exception) {
            Application->ShowException(&exception);
        }
    }
    return 0;
}

static Injector create_injector() {
    const uint64_t x64_addr = get_load_library_x64_addr();
    const uint32_t x32_addr = get_load_library_x32_addr();
    if (!x64_addr) {
        Application->MessageBox(
            L"64-bit version of LoadLibraryW has not been found.\n"
            L"Injecting into 64-bit processes will not be possible.",
            L"Warning", MB_OK | MB_ICONWARNING);
    }
    if (!x32_addr) {
        Application->MessageBox(
            L"32-bit version of LoadLibraryW has not been found.\n"
            L"Injecting into 32-bit processes will not be possible.",
            L"Warning", MB_OK | MB_ICONWARNING);
    }
    return Injector(x64_addr, x32_addr);
}

static uint64_t get_load_library_x64_addr() {
    // Should always succeed
    return (uint64_t) GetProcAddress(GetModuleHandleA("kernel32"),
                                     "LoadLibraryW");
}

static uint32_t get_load_library_x32_addr() {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    const UnicodeString app_dir = ExtractFilePath(Application->ExeName);
    const UnicodeString helper_path = app_dir + "/injector-helper.exe";
    if (!CreateProcessW(helper_path.c_str(), nullptr, nullptr, nullptr, FALSE,
                        0, nullptr, nullptr, &si, &pi)) {
        return 0;
    }
    WaitForSingleObject(pi.hProcess, 1000);
    uint32_t addr = 0;
    if (!GetExitCodeProcess(pi.hProcess, (DWORD *) &addr)) {
        return 0;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return addr;
}

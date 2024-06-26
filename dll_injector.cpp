﻿#include <vcl.h>
#pragma hdrstop
#include <tchar.h>

#include <windows.h>

#include <System.SysUtils.hpp>
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>

USEFORM("MainUnit.cpp", MainForm);

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int) {
    try {
        Application->Initialize();
        Application->MainFormOnTaskBar = true;
        TStyleManager::TrySetStyle("Tablet Light");
        Application->CreateForm(__classid(TMainForm), &MainForm);
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

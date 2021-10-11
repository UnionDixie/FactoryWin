#pragma once
#include <Windows.h>

struct myMSG
{
    HWND hwnd = nullptr;//
    UINT msg = WM_NULL;//type of msg 1..8000
    WPARAM wp = NULL;//save key
    LPARAM lp = NULL;//state key or mouse pos 
    //or size of window
    bool operator()() {
        if (msg == WM_NULL)
            return true;
        return false;
    }
    bool operator <<(HWND value) {
        hwnd = value;
        return true;
    }
    bool operator <<(UINT value) {
        msg = value;
        return true;
    }
};

struct cmdParam {
    HINSTANCE hInstance = nullptr;//def win32.exe
    HINSTANCE hPrevInstance = nullptr;//not often used
    LPSTR lpCmdLine = nullptr;//arg
    int cmdShows = 10;//10=SW_SHOWDEFAULT
};

enum class StyleWin {
    SimpleWin = 0,//Default and drag true;
    DevWin = 1//dont used....
};
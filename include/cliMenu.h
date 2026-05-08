#ifndef __CLIMENU_H__
#define __CLIMENU_H__
#include "position.h"
#ifdef _WIN32
#include <windows.h>
inline void clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = {0, 0};
    DWORD charsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD consoleSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

    // 填充空格
    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, coordScreen, &charsWritten);

    // 重置颜色属性
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, coordScreen, &charsWritten);

    // 光标回到左上角
    SetConsoleCursorPosition(hConsole, coordScreen);
}
#else   //Linux/macOS
#include <locale.h>
inline void clearScreen() {
    std::cout << "\033[2J\033[H";
}
#endif

inline void setConsoleToUTF8() {
#ifdef _WIN32
    // Windows系统
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    // 还需要设置标准输出模式以支持UTF-8
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#else
    // Linux/macOS系统
    setlocale(LC_ALL, "en_US.UTF-8");
    std::locale::global(std::locale("en_US.UTF-8"));
    std::wcout.imbue(std::locale("en_US.UTF-8"));
    std::wcin.imbue(std::locale("en_US.UTF-8"));
#endif
}


inline void showPositionMenu(const Position& pos)
{
	std::cout << "=== Select Position ======" << std::endl;
	for (int i = 0; i < pos.getPlayerNum(); i++) {
		std::cout << "=== " << pos[i] << " \t  [" << i << "]  ===" << std::endl;
	}
	std::cout << "==========================" << std::endl;
}

inline void ifContinueMenu() {
    std::cout << "======= Next round? ======" << std::endl;
    std::cout << "=== Yeees          [1] ===" << std::endl;
    std::cout << "=== Noooo    [Esc]/[2] ===" << std::endl;
    std::cout << "==========================" << std::endl;
}

#endif

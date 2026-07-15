#include "Output.hpp"
#include <iostream>
#include <print>
#include <filesystem>
#include <windows.h>
namespace Output
{
    bool Initialize_Console()
    {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        if (h == INVALID_HANDLE_VALUE || h == nullptr) return false;
        DWORD mode{};
        if (GetConsoleMode(h, &mode))
        {
            SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
        CONSOLE_SCREEN_BUFFER_INFOEX csbi{ .cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX) };
        if (GetConsoleScreenBufferInfoEx(h, &csbi))
        {
            csbi.srWindow.Bottom = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            csbi.srWindow.Right = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            SetConsoleScreenBufferInfoEx(h, &csbi);
        }
        HWND hwnd = GetConsoleWindow();
        if (hwnd != nullptr)
        {
            auto style = GetWindowLongW(hwnd, GWL_STYLE);
            style &= ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
            SetWindowLongW(hwnd, GWL_STYLE, style);
            SetWindowLongW(hwnd, GWL_EXSTYLE, GetWindowLongW(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hwnd, 0, 242, LWA_ALPHA);
            short target_width = 101;
            short target_height = 25;
            SMALL_RECT min_rect{ 0, 0, 1, 1 };
            SetConsoleWindowInfo(h, TRUE, &min_rect);
            COORD buffer_size{ target_width, target_height };
            SetConsoleScreenBufferSize(h, buffer_size);
            SMALL_RECT window_size{ 0, 0, static_cast<short>(target_width - 1), static_cast<short>(target_height - 1) };
            SetConsoleWindowInfo(h, TRUE, &window_size);
            ShowScrollBar(hwnd, SB_BOTH, FALSE);
        }
        CONSOLE_FONT_INFOEX cfi{ .cbSize = sizeof(CONSOLE_FONT_INFOEX) };
        cfi.nFont = 0;
        cfi.dwFontSize.X = 0;
        cfi.dwFontSize.Y = 16;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy_s(cfi.FaceName, L"Consolas");
        SetCurrentConsoleFontEx(h, FALSE, &cfi);
        CONSOLE_CURSOR_INFO cursor{ .dwSize = 1, .bVisible = FALSE };
        SetConsoleCursorInfo(h, &cursor);
        DWORD input_mode{};
        if (GetConsoleMode(hInput, &input_mode))
        {
            SetConsoleMode(hInput, input_mode | ENABLE_QUICK_EDIT_MODE | ENABLE_MOUSE_INPUT);
        }
        FlushConsoleInputBuffer(hInput);
        char buffer[MAX_PATH]{};
        if (GetModuleFileNameA(nullptr, buffer, MAX_PATH) > 0)
        {
            std::string exe_name = std::filesystem::path(buffer).filename().string();
            SetConsoleTitleA((exe_name + " - scare.lol").c_str());
        }
        std::printf("\n");
        return true;
    }

    constexpr std::string_view Get_Gradient_Logo()
    {
        return "\033[38;2;30;100;255m["
            "\033[38;2;50;130;255mn"
            "\033[38;2;70;160;255mo"
            "\033[38;2;85;180;253mm"
            "\033[38;2;95;200;253mn"
            "\033[38;2;100;210;255mo"
            "\033[38;2;110;220;255mm"
            "\033[38;2;120;225;255mc"
            "\033[38;2;130;230;255ma"
            "\033[38;2;140;235;255mt"
            "\033[38;2;150;240;255m."
            "\033[38;2;160;245;255mx"
            "\033[38;2;170;250;255my"
            "\033[38;2;95;200;253mz"
            "\033[38;2;95;200;253m]";
    }

    void Print_Gradient_Text(std::string_view text, int r1, int g1, int b1, int r2, int g2, int b2)
    {
        if (text.empty()) return;
        if (text.length() == 1)
        {
            std::print(" \033[38;2;{};{};{}m{}\033[0m\n", r1, g1, b1, text);
            return;
        }
        std::print(" ");
        for (size_t i = 0; i < text.length(); ++i)
        {
            float t = static_cast<float>(i) / (text.length() - 1);
            int r = static_cast<int>(r1 + t * (r2 - r1));
            int g = static_cast<int>(g1 + t * (g2 - g1));
            int b = static_cast<int>(b1 + t * (b2 - b1));
            std::print("\033[38;2;{};{};{}m{}", r, g, b, text[i]);
        }
        std::print("\033[0m\n");
    }

    void Write_Prefix()
    {
        static bool initialized = Initialize_Console();
        if (!initialized) return;
        std::print(" {}", Get_Gradient_Logo());
    }
}
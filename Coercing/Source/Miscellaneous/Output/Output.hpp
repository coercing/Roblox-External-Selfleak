#pragma once
#include <string_view>
#include <string>
#include <format>
namespace Output
{
    bool Initialize_Console();
    void Print_Gradient_Text(std::string_view text, int r1, int g1, int b1, int r2, int g2, int b2);
    void Write_Prefix();

    template<typename... Args>
    inline void Info(std::format_string<Args...> fmt, Args&&... args)
    {
        Write_Prefix();
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        Print_Gradient_Text(message, 95, 200, 253, 40, 140, 255);   // Bright cyan-blue → deeper blue
    }

    template<typename... Args>
    inline void Warning(std::format_string<Args...> fmt, Args&&... args)
    {
        Write_Prefix();
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        Print_Gradient_Text(message, 255, 200, 80, 255, 140, 30);   // Kept orange-yellow for visibility but softer
    }

    template<typename... Args>
    inline void Error(std::format_string<Args...> fmt, Args&&... args)
    {
        Write_Prefix();
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        Print_Gradient_Text(message, 255, 80, 80, 180, 30, 50);     // Red kept for error urgency
    }

    template<typename... Args>
    inline void Success(std::format_string<Args...> fmt, Args&&... args)
    {
        Write_Prefix();
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        Print_Gradient_Text(message, 80, 255, 180, 40, 200, 120);   // Teal / cyan-green success
    }

    template<typename... Args>
    inline void Coercing(std::format_string<Args...> fmt, Args&&... args)
    {
        Write_Prefix();
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        Print_Gradient_Text(message, 95, 200, 253, 140, 160, 255);  // Main blue theme
    }
}
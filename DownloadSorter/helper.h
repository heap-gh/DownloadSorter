#pragma once

#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

inline std::string to_utf8(std::wstring& wide_string)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}
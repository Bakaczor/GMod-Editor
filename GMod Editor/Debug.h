#pragma once

#include <concepts>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>

namespace app {
    template<std::convertible_to<double>... Args>
    void DebugPrint(const std::string& label, const Args&... values) {
        std::ostringstream oss;
        oss << label << " ";
        ((oss << values << " "), ...);

        oss << std::endl;
        OutputDebugStringA(oss.str().c_str());
    }
}

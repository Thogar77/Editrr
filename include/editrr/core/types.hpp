#pragma once
#include <string>
#include <ctime>

namespace editrr {

    struct Cursor {
        int x{ 0 };
        int y{ 0 };
        int rx{ 0 };
    };

    struct StatusMessage {
        std::string text;
        std::time_t time{ 0 };
    };

} // namespace editrr
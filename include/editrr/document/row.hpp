#pragma once
#include <string>
#include <vector>

namespace editrr {

    constexpr int TABSTOP = 8;

    enum class Highlight : uint8_t {
        Normal = 0,
        Number,
        String,
        Comment,
        Keyword1,
        Keyword2
    };

    struct Row {
        std::string chars;   // raw text
        std::string render;  // expanded tabs
        std::vector<Highlight> hl;
        bool hl_open_comment{ false };

        int size() const { return (int)chars.size(); }
    };

} // namespace editrr
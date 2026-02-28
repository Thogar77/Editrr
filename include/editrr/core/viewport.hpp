#pragma once
#include "editrr/core/types.hpp"
#include "editrr/document/document.hpp"
#include <algorithm>

namespace editrr {

    class Viewport {
    public:
        int width{ 0 };
        int height{ 0 };   // content area (bez status/msg)
        int rowoff{ 0 };
        int coloff{ 0 };

        void clamp_cursor(Document& doc, Cursor& cur) const;
        void scroll(Document& doc, Cursor& cur);

        int cx_to_rx(const Row& row, int cx) const;
        int rx_to_cx(const Row& row, int rx) const;
    };

} // namespace editrr
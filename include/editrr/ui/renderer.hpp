#pragma once
#include "editrr/core/types.hpp"
#include "editrr/core/viewport.hpp"
#include "editrr/document/document.hpp"
#include "editrr/ui/ansi_theme.hpp"
#include <string>

namespace editrr {

    class Renderer {
    public:
        void clear_screen();
        void refresh(Document& doc, Cursor& cur, Viewport& vp,
            const StatusMessage& status, const std::string& filename,
            bool dirty);

    private:
        
        void draw_rows(std::string& out, Document& doc, Viewport& vp);
        void draw_status_bar(std::string& out, Document& doc, Cursor& cur,
            Viewport& vp, const std::string& filename, bool dirty);
        void draw_message_bar(std::string& out, Viewport& vp, const StatusMessage& status);
    };

} // namespace editrr
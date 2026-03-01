#pragma once

#include "editrr/document/document.hpp"
#include <memory>
#include <string>

namespace editrr {

    struct ISyntaxHighlighter {
        virtual ~ISyntaxHighlighter() = default;

        virtual void set_filename(const std::string& filename) = 0;

        virtual bool highlight_row(Document& doc, int row_idx) = 0;

        virtual void highlight_from(Document& doc, int start_row) = 0;
    };

    std::unique_ptr<ISyntaxHighlighter> make_default_highlighter();

} // namespace editrr
#pragma once

#include "editrr/document/document.hpp"
#include <memory>
#include <string>

namespace editrr {

    struct ISyntaxHighlighter {
        virtual ~ISyntaxHighlighter() = default;

        // Wybór języka po filename (np. .cpp, .c, .py)
        virtual void set_filename(const std::string& filename) = 0;

        // Przelicz highlight dla jednego wiersza.
        // Zwraca true jeśli zmienił się stan "open multiline comment" w tym wierszu
        // (czyli trzeba przeliczyć kolejne wiersze).
        virtual bool highlight_row(Document& doc, int row_idx) = 0;

        // Przelicz highlight od start_row do końca dokumentu.
        virtual void highlight_from(Document& doc, int start_row) = 0;
    };

    // Factory: domyślny highlighter oparty o HLDB.
    std::unique_ptr<ISyntaxHighlighter> make_default_highlighter();

} // namespace editrr
#pragma once

#include "editrr/services/syntax/syntax.hpp"

#include "editrr/syntax_db.hpp"

namespace editrr {

    class HldbHighlighter final : public ISyntaxHighlighter {
    public:
        void set_filename(const std::string& filename) override;

        bool highlight_row(Document& doc, int row_idx) override;

        void highlight_from(Document& doc, int start_row) override;

    private:
        const EditorSyntax* syntax_{ nullptr };

        void select_syntax_(const std::string& filename);

        void update_syntax_(Document& doc, int row_idx);

        static bool is_separator_(char c);
        static bool is_quote_(char c);
    };

} // namespace editrr
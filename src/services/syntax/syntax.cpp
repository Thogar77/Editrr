#include "editrr/services/syntax/syntax.hpp"
#include "editrr/services/syntax/syntax_hldb.hpp"

namespace editrr {

    std::unique_ptr<ISyntaxHighlighter> make_default_highlighter() {
        return std::make_unique<HldbHighlighter>();
    }

} // namespace editrr
#pragma once
#include "editrr/document/document.hpp"
#include "editrr/core/types.hpp"
#include "editrr/core/viewport.hpp"
#include <string>

namespace editrr {

    class SearchService {
    public:
        // znajdź następne wystąpienie query, aktualizuje cursor/viewport; zwraca true jeśli znalazł
        bool find_next(Document& doc, Cursor& cur, Viewport& vp,
            const std::string& query, int& last_match, int& direction);
    };

} // namespace editrr
#pragma once
#include "editrr/document/row.hpp"
#include <string>

namespace editrr {

    inline std::string hl_to_color(Highlight h) {
        switch (h) {
        case Highlight::Number: return "31";
        case Highlight::String: return "32";
        case Highlight::Comment:return "90";
        case Highlight::Keyword1:return "34";
        case Highlight::Keyword2:return "35";
        default: return "39";
        }
    }

} // namespace editrr
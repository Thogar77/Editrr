#pragma once
#include "editrr/document/row.hpp"
#include <string>

namespace editrr {

    inline std::string hl_to_color(Highlight h) {
        switch (h) {
        case Highlight::Number: return "31";   // red
        case Highlight::String: return "32";   // green
        case Highlight::Comment:return "90";   // gray
        case Highlight::Keyword1:return "34";  // blue
        case Highlight::Keyword2:return "35";  // magenta
        default: return "39";
        }
    }

} // namespace editrr
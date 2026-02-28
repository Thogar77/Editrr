#pragma once
#include <cctype> 
#include <cstring>
#include "editor/config.hpp"

const size_t HLDB_ENTRIES = sizeof(HLDB) / sizeof(HLDB[0]);
static const char* hl_to_color(Highlight hl) {
    switch (hl) {
    case Highlight::Number:   return "31";
    case Highlight::Keyword1: return "33";
    case Highlight::Keyword2: return "32";
    case Highlight::Match:    return "34";
    case Highlight::String:   return "35";
    case Highlight::Comment:  return "36";
    case Highlight::Normal:
    default: return "39";
    }
}

static bool is_separator(int c) {
    return std::isspace(c) || c == '\0' || std::strchr(",.()+-/*=~%<>[]{};:", c);
}

static bool is_quote(char c) { return c == '"' || c == '\''; }
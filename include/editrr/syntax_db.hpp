#pragma once
#include <cstddef>

struct EditorSyntax {
    const char* filetype;
    const char** filematch;  // np. {".c", ".h", nullptr}
    const char** keywords;   // np. {"if", "else", "for", nullptr} (z '|' dla Keyword2)
    const char* singleline_comment_start; // "//"
    const char* multiline_comment_start;  // "/*"
    const char* multiline_comment_end;    // "*/"
};

// Musisz to zdefiniować w jakimś .cpp (nie w headerze!)
extern const EditorSyntax HLDB[];
extern const size_t HLDB_ENTRIES;
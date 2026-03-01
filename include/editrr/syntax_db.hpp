#pragma once
#include <cstddef>

struct EditorSyntax {
    const char* filetype;
    const char** filematch;
    const char** keywords;
    const char* singleline_comment_start;
    const char* multiline_comment_start;
    const char* multiline_comment_end;
};

extern const EditorSyntax HLDB[];
extern const size_t HLDB_ENTRIES;
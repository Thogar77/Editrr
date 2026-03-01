#include "editrr/syntax_db.hpp"

static const char* C_CPP_MATCH[] = {
  ".c", ".h", ".cpp", ".hpp", ".cc", ".hh", nullptr
};

static const char* C_CPP_KEYWORDS[] = {
  "if", "else", "for", "while", "do", "switch", "case", "break", "continue", "return",
  "class", "struct", "namespace", "public", "private", "protected",
  "template", "typename", "using", "auto", "const", "constexpr", "static",
  "void", "int", "float", "double", "char", "bool", "long", "short", "signed", "unsigned",
  "new", "delete", "try", "catch", "throw",
  "true|", "false|", "nullptr|",
  nullptr
};

const EditorSyntax HLDB[] = {
  {
    "c/cpp",
    C_CPP_MATCH,
    C_CPP_KEYWORDS,
    "//",
    "/*",
    "*/"
  }
};

const size_t HLDB_ENTRIES = sizeof(HLDB) / sizeof(HLDB[0]);
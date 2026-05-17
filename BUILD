load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

WARN_OPTS = [
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wshadow",
    "-Wconversion",
    "-Wsign-conversion",
    "-Wnull-dereference",
    "-Wdouble-promotion",
    "-Wformat=2",
]

cc_library(
    name = "editrr_lib",
    srcs = [
        "src/app/editor_app.cpp",
        "src/commands/dispatcher.cpp",
        "src/core/viewport.cpp",
        "src/document/document.cpp",
        "src/editor/syntax_db.cpp",
        "src/input/input_reader.cpp",
        "src/services/file_service.cpp",
        "src/services/prompt_service.cpp",
        "src/services/search_service.cpp",
        "src/services/syntax/syntax.cpp",
        "src/services/syntax/syntax_hldb.cpp",
        "src/ui/renderer.cpp",
        "src/document/gap_buffer.cpp",
    ],
    hdrs = glob(["include/**/*.hpp"]),
    copts = WARN_OPTS,
    includes = ["include"],
)

cc_binary(
    name = "editrr",
    srcs = ["src/main.cpp"],
    deps = [":editrr_lib"],
)

cmake_minimum_required(VERSION 3.20)

project(editrr
        VERSION 0.0.1
        DESCRIPTION "Editrr - modular terminal editor"
        LANGUAGES CXX
)

# ===== Standard =====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== Options =====
option(EDITRR_ENABLE_ASAN "Enable AddressSanitizer/UBSan in Debug/RelWithDebInfo" OFF)

# ===== Dependencies (via Conan) =====
find_package(magic_enum REQUIRED)
find_package(tomlplusplus REQUIRED)

# ===== Executable =====
add_executable(editrr
        src/main.cpp

        src/app/editor_app.cpp

        src/core/viewport.cpp

        src/document/document.cpp
        src/document/gap_buffer.cpp

        src/input/input_reader.cpp

        src/ui/renderer.cpp

        src/services/file_service.cpp
        src/services/search_service.cpp
        src/services/prompt_service.cpp

        src/editor/syntax_db.cpp

        src/commands/dispatcher.cpp

        src/services/syntax/syntax.cpp
        src/services/syntax/syntax_hldb.cpp
        src/config/parser/parser.cpp
        src/config/parser/parser.cpp
        src/config/config.cpp
        include/editrr/config/serializer/ISerializer.hpp
        include/editrr/config/serializer/toml_serializer.h
        include/editrr/config/config_data.h
        src/config/serializer/toml_serializer.cpp
)

target_include_directories(editrr
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(editrr
        PRIVATE
        magic_enum::magic_enum
        tomlplusplus::tomlplusplus
)

# ===== Compile options / warnings =====
if (MSVC)
    target_compile_options(editrr PRIVATE /W4 /permissive-)
    target_compile_definitions(editrr PRIVATE _CRT_SECURE_NO_WARNINGS)
else ()
    target_compile_options(editrr PRIVATE
            -Wall -Wextra -Wpedantic
            -Wshadow -Wconversion -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
    )
endif ()

# ===== Sanitizers (optional) =====
if (EDITRR_ENABLE_ASAN AND NOT MSVC)
    target_compile_options(editrr PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
            $<$<CONFIG:RelWithDebInfo>:-fsanitize=address,undefined>
    )
    target_link_options(editrr PRIVATE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
            $<$<CONFIG:RelWithDebInfo>:-fsanitize=address,undefined>
    )
endif ()

# ===== Platform notes =====
if (WIN32)
    message(WARNING "This target uses POSIX APIs (termios/ioctl). It is intended for Linux/macOS.")
endif ()

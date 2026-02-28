cmake_minimum_required(VERSION 3.20)

project(editrr
  VERSION 0.0.1
  DESCRIPTION "Editrr - modular terminal editor"
  LANGUAGES CXX
)

# ===== Options =====
option(EDITRR_ENABLE_ASAN "Enable AddressSanitizer/UBSan in Debug/RelWithDebInfo" OFF)

# ===== Standard =====
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== Executable =====
add_executable(editrr
  src/main.cpp

  src/app/editor_app.cpp

  src/core/viewport.cpp

  src/document/document.cpp

  src/input/input_reader.cpp

  src/ui/renderer.cpp

  src/services/file_service.cpp
  src/services/search_service.cpp
  src/services/prompt_service.cpp

  src/editor/syntax_db.cpp
  
  src/commands/dispatcher.cpp

  # syntax (HLDB strategy)
  src/services/syntax/syntax.cpp
  src/services/syntax/syntax_hldb.cpp
)

target_include_directories(editrr
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ===== Compile options / warnings =====
if (MSVC)
  target_compile_options(editrr PRIVATE /W4 /permissive-)
  target_compile_definitions(editrr PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(editrr PRIVATE
    -Wall -Wextra -Wpedantic
    -Wshadow -Wconversion -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
  )
endif()

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
endif()

# ===== Platform notes =====
# This is a POSIX terminal app; Windows would need a different backend.
if (WIN32)
  message(WARNING "This target uses POSIX APIs (termios/ioctl). It is intended for Linux/macOS.")
endif()
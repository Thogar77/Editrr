cmake_minimum_required(VERSION 3.10)
project(TextEditor)

set(CMAKE_CXX_STANDARD 20)


add_executable(editrr
  src/main.cpp
  src/terminal/terminal.cpp
  src/editor/editor.cpp
)

target_include_directories(editrr PRIVATE
  ${CMAKE_SOURCE_DIR}/include)

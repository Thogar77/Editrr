A CLI text editor written in C++, inspired by the kilo text editor. This project was developed as part of the "Inteligentny, nowoczesny i wydajny C++" (Intelligent, Modern and Efficient C++) course.

## Features

- **Text Editing**: Basic text editing capabilities
- **Syntax Highlighting**: Intelligent syntax highlighting for various programming languages
- **Search and Replace**: Powerful search functionality across documents
- **Command System**: Extensible command dispatcher for editor operations
- **Terminal Integration**: ANSI-themed rendering for terminal-based interface
- **File Management**: Robust file service for loading and saving documents
- **Input Handling**: Advanced input reader supporting various key combinations
- **Viewport Management**: Efficient viewport handling for large documents

## Requirements

- C++17 or later
- CMake 3.16 or later
- Conan package manager
- Pixi environment manager (optional, for Python-based tooling)

## Installation

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd Editrr
   ```

2. Install dependencies using Conan:
   ```bash
   conan install . --build=missing
   ```

3. (Optional) Set up the environment with Pixi:
   ```bash
   pixi install
   ```

## Building

1. Configure the project with CMake:
   ```bash
   cmake -S . -B build
   ```

2. Build the project:
   ```bash
   cmake --build build
   ```

## Running

After building, run the editor:
```bash
./build/src/Editrr
```

## Project Structure

- `include/`: Header files
  - `editrr/`: Main editor components
    - `app/`: Application logic
    - `commands/`: Command system
    - `core/`: Core types and viewport
    - `document/`: Document and row management
    - `input/`: Input handling
    - `services/`: Various services (file, prompt, search, syntax)
    - `ui/`: User interface components
- `src/`: Source files corresponding to headers
- `CMakeLists.txt`: CMake build configuration
- `conanfile.py`: Conan package dependencies
- `pixi.toml`: Pixi environment configuration

## Acknowledgments

- Inspired by the [kilo text editor](https://github.com/antirez/kilo)
- Developed as part of the "Inteligentny, nowoczesny i wydajny C++" course
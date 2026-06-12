from conan import ConanFile
from conan.tools.cmake import cmake_layout


class Editrr(ConanFile):
    name = "editrr"
    version = "0.0.1"
    author = "Jakub Kruczek"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        for dep in ["magic_enum/0.9.7", "tomlplusplus/3.4.0"]:
            self.requires(dep)

    def layout(self):
        cmake_layout(self)

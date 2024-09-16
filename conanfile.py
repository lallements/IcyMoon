from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout

class IcyMoonEngineRecipe(ConanFile):
    name = "icy_moon_engine"
    version = "0.1"
    package_type = "application"
    build_policy = "missing"

    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualBuildEnv"

    requires = {
        "gtest/1.15.0"
    }

    options = {

    }

    default_options = {

    }

    tool_requires = {
        "cmake/3.30.1",
        "ninja/1.12.1",
    }

    def layout(self):
        cmake_layout(self)

    def generate(self):
        toolChain = CMakeToolchain(self, generator="Ninja")
        toolChain.presets_prefix = ""
        toolChain.generate()

        cmake = CMakeDeps(self)
        cmake.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

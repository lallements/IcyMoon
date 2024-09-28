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
        "fmt/11.0.2",
        "gtest/1.15.0",
        "vulkan-headers/1.3.290.0",
    }

    options = {
        "coverage": [None, "on"],
    }

    default_options = {
        "coverage": None,
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

        if self.options.coverage == "on":
            toolChain.cache_variables["TEST_COVERAGE"] = True

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

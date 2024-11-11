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
        "cimg/3.3.2",
        "fmt/11.0.2",
        "glfw/3.4",
        "gtest/1.15.0",
        "vulkan-headers/1.3.243.0", # version depended on by vulkan-memory-allocator
        "vulkan-memory-allocator/cci.20231120",
        "whereami/cci.20220112",
    }

    options = {
        "coverage": [None, "on"],
    }

    default_options = {
        "coverage": None,
        "cimg/*:enable_fftw": False,
        "cimg/*:enable_jpeg": True,
        "cimg/*:enable_openexr": False,
        "cimg/*:enable_png": True,
        "cimg/*:enable_tiff": True,
        "cimg/*:enable_ffmpeg": False,
        "cimg/*:enable_opencv": False,
        "cimg/*:enable_magick": False,
        "cimg/*:enable_xrandr": False,
        "cimg/*:enable_xshm": False,
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

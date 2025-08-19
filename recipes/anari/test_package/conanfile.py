import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.build import can_run


class helloTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualBuildEnv"
    
    tool_requires = {
        "cmake/3.30.1",
        "ninja/1.12.1",
    }

    def requirements(self):
        self.requires(self.tested_reference_str)

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

    def layout(self):
        cmake_layout(self)

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindir, "example")
            self.run(cmd, env="conanrun")
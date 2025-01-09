from conan import ConanFile
from conan.tools.files import copy, get

import os


class OpenUsdRecipe(ConanFile):
    name = "open_usd"
    version = "24.11"
    package_type = "shared-library"

    settings = "os", "arch"
    generators = "VirtualBuildEnv"

    requires = {
        "opengl/system"
    }

    tool_requires = {
        "cmake/3.30.1",
    }

    def source(self):
        get(self, "https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v24.11.zip", strip_root=True)

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"
        self.folders.generators = "build/generators"
        self.cpp.build.builddirs = ["build"]

    def package(self):
        self.run(" ".join([
            "python",
            os.path.join(self.folders.source_folder, "build_scripts/build_usd.py"),
            "--use-cxx11-abi", "1",
            "--no-examples",
            "--no-tutorials",
            self.folders.package_folder
        ]))

    def package_info(self):
        # Disable the config package that would otherwise be generated by CMakeDeps
        self.cpp_info.set_property("cmake_find_mode", "none")

        self.cpp_info.builddirs = ['.', os.path.join('lib', 'cmake')]
        self.buildenv_info.append_path("PATH", os.path.join(self.folders.package_folder, "bin"))
        self.buildenv_info.append_path("PYTHONPATH", os.path.join(self.folders.package_folder, "lib", "python"))
        self.runenv_info.append_path("PATH", os.path.join(self.folders.package_folder, "bin"))
        self.runenv_info.append_path("PYTHONPATH", os.path.join(self.folders.package_folder, "lib", "python"))
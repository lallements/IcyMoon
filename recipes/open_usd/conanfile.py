from conan import ConanFile
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import get

import os


class OpenUsdRecipe(ConanFile):
    name = "open_usd"
    version = "24.11"
    package_type = "shared-library"

    settings = "os", "compiler", "build_type", "arch"
    generators = "VirtualBuildEnv"

    tool_requires = {
        "cmake/3.30.1",
    }

    def source(self):
        get(self, "https://github.com/PixarAnimationStudios/OpenUSD/archive/refs/tags/v24.11.zip", strip_root=True)

    def layout(self):
        self.folders.source = "src"
        self.folders.build = "build"
        self.folders.generators = "build/generators"

    def build(self):
        self.run(" ".join([
            "python",
            os.path.join(self.folders.source_folder, "build_scripts/build_usd.py"),
            "--use-cxx11-abi", "1",
            "--no-examples",
            "--no-tutorials",
            "./build"
        ]))

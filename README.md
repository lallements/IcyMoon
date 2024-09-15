# IcyMoon

- [How to Build](#how-to-build)
  - [Dependencies](#dependencies)
  - [Conan](#conan)
  - [Compiling](#compiling)


## How to Build

### Dependencies

The following must be installed manually:

- [Conan](https://conan.io/downloads.html) (tested with v2.7.1)

### Conan

The project is compiled using Conan and CMake.
Dependencies are automatically downloaded and installed using `conan install`.
To do so, first create a `build` folder from the root of the project:

```bash
mkdir build
cd build
```

Then, run the `conan install` command from the `build` folder with one of the profiles under the `profiles` folder:

```base
conan install .. -pr ../profiles/<my_profile> --build=missing
```

We use Conan to download some tools needed for compilation (e.g. CMake). To use these tools during compilation, we need to first start a virtual environment. To do so, Conan generates a `conanbuild.sh` and `deactivate_conanbuild.sh` scripts for each profile, located under `build/<my_profile>/generators/`. For example, to build `gcc-debug`, we start the corresponding virtual environment as follows:

```bash
source build/<my_preset>/generators/conanbuild.sh
```

The install command also creates a `CMakeUserPresets.json` file in the root folder. The latter will contain a CMake Preset for each Conan profile. They can be listed using cmake as follows:

```base
cmake --list-presets

Available configure presets:

  "gcc-debug"   - 'gcc-debug' config
  "gcc-release" - 'gcc-release' config
```

### Compiling

Once we have CMake Presets and an active Conan virtual environment, the project can be compiled with a chosen preset from the root folder:

```bash
cmake --preset <my_preset>
cmake --build --preset <my_preset>
```

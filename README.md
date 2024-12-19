# IcyMoon

- [How to Build](#how-to-build)
  - [Dependencies](#dependencies)
  - [OpenUSD](#openusd)
  - [Conan](#conan)
  - [Compiling](#compiling)
  - [Running tests](#running-tests)
  - [Test Coverage with GCC](#test-coverage-with-gcc)


## How to Build

### Dependencies

The following must be installed manually:

- [Conan](https://conan.io/downloads.html) (tested with 2.9.2)
- [Vulkan SDK](https://vulkan.lunarg.com/) (tested with 1.3.290.0)
- [OpenUSD](https://github.com/PixarAnimationStudios/OpenUSD) (tested with 24.11)

### OpenUSD

OpenUSD needs to be manually added to your Conan local cache. To do so, from the root folder of this project, run:

```bash
conan create ./recipes/open_usd --build=missing
```

Once complete, the following environment variables will be appended to get easy access to tools such as usdview:

| Environment Variable | Value to add                         |
|----------------------|--------------------------------------|
| `PYTHONPATH`         | `<path_to_OpenUSD>/build/lib/python` |
| `PATH`               | `<path_to_OpenUSD>/build/bin`        |

These variables will be available when using the Conan virtual environment (see `conanbuild.sh` below).

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
conan install .. -pr:a ../profiles/<my_profile> --build=missing
```

> Important: On Linux, conan may fail due to missing packages. In this case, a command of the form "apt-get install ..." will appear in the error message. Use this command in sudo to install the missing packages.

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

### Running tests

The project uses `ctest` to handle tests and supports presets. To get a list of ctest presets, run the following command from the root folder:
```bash
ctest --list-presets

Available test presets:

  "gcc-debug"
  "gcc-release"
```

All tests can be run with the `--preset` argument:
```bash
ctest --preset gcc-debug
```

Each test is labelled as a `"unit_test"` or an `"integration_test"`. This allows to filter out tests by type:
```bash
ctest --preset gcc-debug -L "unit_test" # to run unit tests only
ctest --preset gcc-debug -L "integration_test" # to run integration tests only
```

### Test Coverage with GCC

Test coverage is currently supported with lcov via the conan profile `gcc-coverage_on-debug`:
```bash
cd build
conan install .. --pr ../profiles/gcc-coverage_on-debug --build=missing
cd ..
source build/gcc-coverage_on-debug/generators/conanbuild.sh
cmake --preset gcc-coverage_on-debug
cmake --build --preset gcc-coverage_on-debug --target im3e_test_coverage
```

There exists multiple cmake targets to generate different test coverage reports:
- `im3e_test_coverage`: combines the coverage of all tests
- `im3e_unit_test_coverage`: generate a report for unit tests only (excluding integration tests)

## Building with Meson

This project uses [Meson](https://mesonbuild.com/) as its build system.

### Prerequisites

- **C++ compiler** (GCC, Clang, or MSVC)
- **Python**

### Installing Meson and Ninja

**Meson** (1.10.0 or later) and **Ninja** are required to build this project.

- On macOS:

  ```
  brew install meson ninja
  ```

- On Linux:

  ```
  sudo apt install meson ninja-build
  ```

  Or using pip:

  ```
  pip install meson ninja
  ```

- On Windows:
  ```
  pip install meson ninja
  ```

### Setup

1. Configure the build directory (only needed once):

   ```
   meson setup build
   ```

   For a release build use this setup:

   ```
   meson setup build -Dbuildtype=release
   ```

2. Compile the project:

   ```
   meson compile -C build
   ```

3. Copy files to build folder:

   ```
   build/elma.res
   build/lgr/default.lgr
   build/fonts/small.abc
   etc
   ```

4. Run the executable:
   ```
   cd build
   ./elma
   ```

### Configuration

You can list all of the options using `meson configure build` and set an option using `meson configure build -Doption=value`

Built-in options are documented at https://mesonbuild.com/Builtin-options.html.

Custom options are documented in the repository file `meson.options`

However, here are the relevant options:

#### buildtype

`meson configure build -Dbuildtype=`

- `debug` - Extra checks with errors or warnings
- `debugoptimized` - Debug, but O2-optimized build
- `release` - O3-optimized build

#### b_sanitize

`meson configure build -Db_sanitize=`

- `none` - No sanitizer
- `address` - Add AddressSanitizer (memory errors, support by Clang/GCC/MSVC)
- `address,undefined` - Add AddressSanitizer and UndefinedBehaviourSanitizer (Clang/GCC only)

#### profile_performance

`meson configure build -Dprofile_performance=`

- `false` - Disabled
- `true`- Enable performance timing checks

### Common Commands

- **Setup**: `meson setup build`
- **Build**: `meson compile -C build`
- **Clean**: `meson compile -C build --clean`
- **Full Rebuild**: `meson setup --wipe build`
- **Reconfigure Options**: `meson setup --reconfigure build`
- **List Options**: `meson configure build`
- **Set Options**: `meson configure build -Doption=value`

### Windows

If you don't have Visual Studio 2022 in your PATH, you must run meson within the Visual Studio terminal called `Developer Command Prompt for VS 2022`

To add this terminal to Visual Studio Code, add the following to .vscode/settings.json:

```
{
  "terminal.integrated.profiles.windows": {
    "VsDevCmd 64-bit (2022)": {
      "path": [
        "${env:windir}\\Sysnative\\cmd.exe",
        "${env:windir}\\System32\\cmd.exe"
      ],
      "args": [
        "/k",
        // Path below assumes a VS2022 Community install;
        // update as appropriate if your IDE installation path
        // is different, or if using the standalone build tools
        "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/Tools/VsDevCmd.bat",
        "-arch=x64",
        "-host_arch=x64"
      ],
      "overrideName": true,
      "icon": "terminal-cmd"
    }
  }
}
```

When running the setup step of meson you will need to pass `--vsenv`, like so:

```
meson setup build --vsenv
```

---

## Code Formatting with clang-format

To ensure consistent code formatting, use `clang-format`.

### Installation

- On macOS:

  ```
  $ brew install clang-format
  ```

- On Linux:

  ```
  $ sudo apt install clang-format-19
  ```

- On Windows:
  ```
  $ pip install clang-format~=19.0
  ```

### Running clang-format

To format all files in the repository, run the following command:

```
ninja -C build clang-format
```

---

## Static Analysis with clang-tidy

To catch bugs and improve code quality, use `clang-tidy`.

### Installation

- On macOS:

  ```
  $ brew install clang-tidy
  ```

- On Linux:

  ```
  $ sudo apt install clang-tidy-19
  ```

- On Windows:
  ```
  $ pip install clang-tidy~=19.0
  ```

### Running clang-tidy

To run static analysis on all source files, run the following command:

```
ninja -C build clang-tidy
```

The checks are configured in the `.clang-tidy` file at the project root.

---

## VS Code Configuration

VS Code configuration files are provided in the `docs/vscode/` directory for reference. These include:

- **`launch.json`** - Debug configurations for macOS (lldb) and Windows (cppvsdbg)
- **`tasks.json`** - Build tasks using Meson

To use these configurations:

1. Copy the files to your `.vscode/` folder:

   ```
   mkdir -p .vscode
   cp docs/vscode/launch.json .vscode/
   cp docs/vscode/tasks.json .vscode/
   ```

2. The `.vscode/` folder is gitignored, so you can customize these files locally without affecting the repository.

### Available Tasks

- **meson: compile** - Build the project (set as default build task)
- **meson: clean** - Clean build artifacts

### Debug Configurations

- **(macOS) Launch Elma** - Debug on macOS using lldb
- **(Windows) Launch Elma** - Debug on Windows using Visual Studio debugger

Both configurations automatically run the compile task before launching.

---

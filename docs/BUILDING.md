## Building with Meson

This project uses [Meson](https://mesonbuild.com/) as its build system.

### Prerequisites

- **C++ compiler** (GCC, Clang, or MSVC)

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

2. Compile the project:
   ```
   meson compile -C build
   ```

3. Run the executable:
   ```
   ./build/elma
   ```

### Common Build Commands

- **Build**: `meson compile -C build`
- **Clean**: `meson compile -C build --clean`
- **Reconfigure**: `meson setup --reconfigure build`
- **Full rebuild**: `meson setup --wipe build`

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

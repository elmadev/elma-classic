# Game release
This is NOT a playable release of Elasto Mania. If you're looking to play any of the Elasto Mania games, visit https://elastomania.com.

# Elasto Mania (2000) Source Code
This repository was uploaded in good faith for the purpose of exploring the original source code of this classic game. Non-source-code game assets (sounds, graphics, tools, etc.) are not part of this open source release.

See LICENSE.md for license information.

If you'd like to use this source code in ways other than permitted by the license and this document, contact us at info@elastomania.com.

If you'd like to support continued development of the Elasto Mania franchise, you can do so by buying our games on any store front linked on our website.

*The Elasto Mania Team*

https://elastomania.com

# Meson Build

Make sure you have meson and ninja installed on your computer.

You can then setup meson as follows:

```
meson setup build --vsenv
meson compile -C build
```

## Windows

To build in Windows, you must have Visual Studio 2022 installed, and then run meson inside of the `Developer Command Prompt for VS 2022`. If you would prefer to have a terminal inside of Visual Studio Code, you can add a file .vscode/settings.json, or append to the file if it exists already with the information below. You can then click on the + icon and directly add the VS 2022 terminal to Visual Studio Code

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
    },
    "VsDevCmd 32-bit (2022)": {
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
        "-arch=x86",
        "-host_arch=x64"
      ],
      "overrideName": true,
      "icon": "terminal-cmd"
    }
  }
}
```

You might want to set up a build environment for both 32-bit and 64-bit. If so, you should setup meson twice: once in the 32-bit terminal, and once in the 64-bit terminal:


```
32-bit terminal:
meson setup build32 --vsenv
meson compile -C build32

64-bit terminal:
meson setup build64 --vsenv
meson compile -C build64
```

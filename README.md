# Combat Music Fix NG

Simple SKSE plugin to fix the never-ending combat music bug. Compatible with multiple game runtimes, using [CommonLibSSE NG](https://github.com/alandtse/CommonLibVR/tree/ng)

## Requirements

- [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
    - Desktop development with C++
- [CMake](https://cmake.org/)
    - Add it to your `PATH`
- [Vcpkg](https://github.com/microsoft/vcpkg#quick-start-windows)
    - Add a new `VCPKG_ROOT` environment variable pointing to the root folder of vcpkg

## User Requirements

- [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
    - Needed for SSE/AE
- [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
    - Needed for VR

## Clone and Build
Open terminal (e.g., PowerShell) and run the following commands:

```
git clone --recurse-submodules -j4 https://github.com/gabriel-andreescu/CombatMusicFixNG.git
cd CombatMusicFixNG
.\BuildRelease.bat
```
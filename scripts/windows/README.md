# Windows

## Prerequisites

| Tool | Install |
|---|---|
| Windows 10/11 x64 | - |
| Visual Studio 2022 (C++ workload) | Visual Studio Installer |
| CMake 3.21+ | Visual Studio Installer or cmake.org |
| Git Bash | ships with Git for Windows |
| Qt 6.8+ | `aqt install-qt windows desktop 6.8.3 win64_msvc2022_64` |

## Build

Run from **Git Bash** (the scripts are bash):

```txt
export QT_ROOT_DIR=/c/Qt/6.8.3/msvc2022_64
scripts/fetch-deps.sh     # llama.cpp + models -> dist/
scripts/build.sh          # CMake preset "release" -> dist\gladys.exe
```

## Run

```txt
.\dist\gladys.exe
```

## Administrator privileges

The global hotkey (Ctrl+Alt+P) uses `SetWindowsHookEx`, which needs elevation:

```txt
gladys.exe > Properties > Compatibility
  -> check "Run this program as an administrator"
```

If the hotkey does nothing, check the console for:
`KeyGrab: SetWindowsHookEx failed. Run as Administrator.`

## Layout

```txt
scripts/windows/
└── README.md
```

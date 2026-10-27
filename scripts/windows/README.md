# Windows

## Prerequisites

| Tool | Install |
|---|---|
| Windows 10/11 x64 | - |
| Visual Studio 2022 (C++ workload) | Visual Studio Installer |
| CMake 3.21+ | Visual Studio Installer or cmake.org |
| Git Bash | ships with Git for Windows |
| Qt 6.8+ | `scripts/qt-install.sh windows` |

## Build

Run from **Git Bash** (the scripts are bash). Replace `X:` with your real drive:

```txt
export GLADYS_SPACE=X:/Users/mjesbar/gladys
scripts/qt-install.sh windows            # Qt -> X:/Users/mjesbar/gladys/qt-6.8.3/msvc2022_64
export QT_ROOT_DIR=X:/Users/mjesbar/gladys/qt-6.8.3/msvc2022_64
scripts/fetch-deps.sh                    # llama.cpp + models -> X:/Users/mjesbar/gladys/dist/
scripts/build.sh                         # build -> X:/Users/mjesbar/gladys/build, install -> dist/
```

## Run

```txt
X:\Users\mjesbar\gladys\dist\gladys.exe
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

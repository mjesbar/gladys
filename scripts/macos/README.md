# macOS

## Prerequisites

| Tool | Install |
|---|---|
| macOS 12+ (arm64) | - |
| Xcode Command Line Tools | `xcode-select --install` |
| CMake 3.21+ | `brew install cmake` |
| Qt 6.8+ | `scripts/qt-install.sh macos` |

## Build

```txt
scripts/qt-install.sh macos              # Qt -> /Users/mjesbar/gladys/qt-6.8.3/macos
export QT_ROOT_DIR=/Users/mjesbar/gladys/qt-6.8.3/macos
scripts/fetch-deps.sh                    # llama.cpp + models -> /Users/mjesbar/gladys/dist/
scripts/build.sh                         # build -> /Users/mjesbar/gladys/build, install -> dist/
```

`GLADYS_SPACE` overrides the build space (default `$HOME/gladys`).

## Run

```txt
/Users/mjesbar/gladys/dist/gladys.app
```

## Accessibility permission

The global hotkey (Cmd+Option+P) uses CGEventTap, which needs Accessibility access:

```txt
System Settings > Privacy & Security > Accessibility
  -> add Gladys (or Terminal when launched from a shell)
  -> restart Gladys
```

If the hotkey does nothing, check Console.app for:
`KeyGrab: CGEventTapCreate failed. Grant Accessibility permission.`

## Layout

```txt
scripts/macos/
└── README.md
```

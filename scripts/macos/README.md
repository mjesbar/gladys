# macOS

## Prerequisites

| Tool | Install |
|---|---|
| macOS 12+ (arm64) | - |
| Xcode Command Line Tools | `xcode-select --install` |
| CMake 3.21+ | `brew install cmake` |
| Qt 6.8+ | `scripts/qt-install.sh macos` |

## Build

The target is always arm64. An Intel host (e.g. dockurr/macos on x86 Linux) can compile and link it, but the binary only runs on Apple Silicon.

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

## Permissions

Gladys needs three permissions. macOS does not allow granting them automatically.

| Permission | Used for |
|---|---|
| Microphone | Recording the voice (prompted on first use) |
| Accessibility | Global hotkey (CGEventTap) and pasting text |
| Input Monitoring | Reading the hotkey keys |

```txt
System Settings > Privacy & Security
  -> Accessibility      : add Gladys, enable it
  -> Input Monitoring   : add Gladys, enable it
  -> restart Gladys
```

If the hotkey does nothing, check Console.app for:
`KeyGrab: CGEventTapCreate failed. Grant Accessibility permission.`

Permissions are tied to the app signature. After a rebuild, macOS may ask again or
list Gladys as disabled: remove it and add it back.

## Layout

```txt
scripts/macos/
└── README.md
```

# macOS

## Prerequisites

| Tool | Install |
|---|---|
| macOS 12+ (arm64) | - |
| Xcode Command Line Tools | `xcode-select --install` |
| CMake 3.21+ | `brew install cmake` |
| Qt 6.8+ | `aqt install-qt mac desktop 6.8.3 clang_64` |

## Build

```txt
export QT_ROOT_DIR=/path/to/Qt/6.8.3/macos
scripts/fetch-deps.sh     # llama.cpp + models -> dist/
scripts/build.sh          # CMake preset "release" -> dist/gladys
```

## Run

```txt
./dist/gladys
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

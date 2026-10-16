# Linux

## Prerequisites

| Tool | Install |
|---|---|
| CMake 3.21+, C++17 compiler | `sudo apt install cmake g++` |
| X11 headers | `sudo apt install libx11-dev` |
| Qt 6.8+ | `aqt install-qt linux desktop 6.8.3 linux_gcc_64` |

## Build

```txt
export QT_ROOT_DIR=/path/to/Qt/6.8.3/gcc_64
scripts/fetch-deps.sh     # llama.cpp + models -> dist/
scripts/build.sh          # CMake preset "release" -> dist/gladys
```

## Run

```txt
./dist/gladys
```

## Wayland paste (uinput)

Paste uses `/dev/uinput`. Grant access once:

```txt
scripts/linux/uinput.sh allow    # asks for root via pkexec
# log out and back in
scripts/linux/uinput.sh deny     # revoke
```

## Layout

```txt
scripts/linux/
├── README.md
└── uinput.sh
```

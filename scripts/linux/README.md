# Linux

## Prerequisites

| Tool | Install |
|---|---|
| CMake 3.21+, C++17 compiler | `sudo apt install cmake g++` |
| X11 headers | `sudo apt install libx11-dev` |
| Qt 6.8+ | `scripts/qt-install.sh linux` |

## Build

```txt
scripts/qt-install.sh linux              # Qt -> $HOME/gladys/qt-6.8.3/gcc_64
export QT_ROOT_DIR=$HOME/gladys/qt-6.8.3/gcc_64
scripts/fetch-deps.sh                    # llama.cpp + models -> $HOME/gladys/dist/
scripts/build.sh                         # build -> $HOME/gladys/build, install -> $HOME/gladys/dist/
```

`GLADYS_SPACE` overrides the build space (default `$HOME/gladys`).

## Run

```txt
$HOME/gladys/dist/gladys
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

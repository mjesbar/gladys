# Execution order

1. `qt-install.sh`: This install a custom qt version (6.8.3) in the user space (`~/gladys/qt-6.8.3`) just for from source build purposes.
2. `build.sh`: This uses CMake, Qt6.8+ and native tools to build the App. The `build/` and `dist/` folders are placed in user space (`~/gladys/dist/` & `~/gladys/build/`).
3. `package.sh`: Packages `~/gladys/dist/` into `~/gladys/release/`: Linux `.tar.bz2` + `.AppImage`, Windows `.zip` + `.msi`, macOS `.zip`. Each artifact gets a `.sha256` file. All are published in GitHub releases.
4. `linux/install.sh` and `macos/install.sh`: Public one-line installers, no root. They verify the `.sha256` before installing. Pass `--uninstall` to remove the app and the model caches.

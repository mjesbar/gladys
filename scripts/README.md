# Execution order

1. `qt-install.sh`: This install a custom qt version (6.8.3) in the user space (`~/gladys/qt-6.8.3`) just for from source build purposes.
2. `build.sh`: This uses CMake, Qt6.8+ and native tools to build the App. The `build/` and `dist/` folders are placed in user space (`~/gladys/dist/` & `~/gladys/build/`).
3. `package-sh`: This arrange the `~/gladys/dist/` compiled App into a `.zip` or `tar.bz` archive. Also, these archives will be available in github releases.
4. `install.sh`: Public onscript install for public use. This install the archive releases according to the OS standard (MacOS, Windows, Linux).

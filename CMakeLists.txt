cmake_minimum_required(VERSION 3.21)
# Project name, version and minimum CMake
project(Gladys VERSION 1.0.0 LANGUAGES CXX)

# Find Qt6 modules and set C++17
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Network Svg)
qt_standard_project_setup(REQUIRES 6.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Map host CPU to x64 or arm (used for the sherpa-onnx library path)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  set(GLADYS_ARCH "arm")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
  set(GLADYS_ARCH "x64")
else()
  message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Main app target and its C++ sources
qt_add_executable(gladys WIN32 MACOSX_BUNDLE
  src/main.cc
  src/modules/log/log.cc
  src/modules/ai/stt.cc
  src/modules/ai/llm.cc
  src/modules/key/toggle.cc
  src/modules/key/paste.cc
  src/modules/setting/setting.cc
  src/modules/ui/x11_shadow.cc
  src/modules/ui/audio_bars.cc
  src/modules/ui/settings.cc
  src/modules/ui/tray.cc
)

# App icon per OS: Windows exe resource (.ico), macOS bundle (.icns), Linux window icon set at runtime from icon.svg
if(WIN32)
  target_sources(gladys PRIVATE src/assets/icons/icon.rc)
elseif(APPLE)
  set_source_files_properties(src/assets/icons/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  target_sources(gladys PRIVATE src/assets/icons/icon.icns)
  set_target_properties(gladys PROPERTIES MACOSX_BUNDLE_ICON_FILE "icon.icns")
endif()

# QML module and its files; Theme is a singleton
set_source_files_properties(src/qml/components/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(gladys
  URI Gladys
  VERSION 1.0
  QML_FILES
    src/qml/components/Theme.qml
    src/qml/components/Mic.qml
    src/qml/components/Card.qml
    src/qml/components/IconButton.qml
    src/qml/components/ThemedCheckBox.qml
    src/qml/components/Field.qml
    src/qml/windows/Overlay.qml
    src/qml/windows/Splash.qml
    src/qml/windows/Settings.qml
)

# Embed fonts and icons into the binary
qt_add_resources(gladys app_resources PREFIX "/" BASE src/assets FILES
  src/assets/fonts/RobotoSlab-Regular.ttf
  src/assets/fonts/RobotoSlab-Bold.ttf
  src/assets/fonts/Molle-Regular.ttf
  src/assets/icons/spinner.svg
  src/assets/icons/title.svg
  src/assets/icons/icon.svg
  src/assets/icons/icon-app.svg
  src/assets/icons/plus.svg
  src/assets/icons/trash.svg
  src/assets/icons/chevron-right.svg
)

# Header search paths
target_include_directories(gladys PRIVATE
  src
  src/modules
  src/modules/ai
  src/modules/setting
  src/modules/ui
  src/modules/key
  src/include
)

# Miniaudio: compile its implementation in stt.cc only
set_source_files_properties(src/modules/ai/stt.cc PROPERTIES
  COMPILE_DEFINITIONS "MINIAUDIO_IMPLEMENTATION"
)

# Miniaudio debug output, enabled with: cmake -DGLADYS_MA_DEBUG=ON
option(GLADYS_MA_DEBUG "Enable miniaudio debug output" OFF)
if(GLADYS_MA_DEBUG)
  target_compile_definitions(gladys PRIVATE MA_DEBUG_OUTPUT)
endif()

# Link Qt modules
target_link_libraries(gladys PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Widgets Qt6::Network Qt6::Svg)

# Runtime search path so the app finds the bundled libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GLADYS_RPATH "@executable_path/../Frameworks")
else()
  set(GLADYS_RPATH "$ORIGIN;$ORIGIN/lib")
endif()
set_target_properties(gladys PROPERTIES
  INSTALL_RPATH "${GLADYS_RPATH}"
  BUILD_WITH_INSTALL_RPATH TRUE
)

# OS-specific system libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)
  target_link_libraries(gladys PRIVATE ${X11_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(gladys PRIVATE "-framework CoreGraphics" "-framework CoreFoundation")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(gladys PRIVATE user32)
endif()

# Link and install the speech-to-text runtime (sherpa-onnx)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SHERPA_ONNX_OS_DIR "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(SHERPA_ONNX_OS_DIR "macos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(SHERPA_ONNX_OS_DIR "win")
endif()

set(SHERPA_ONNX_LIB_DIR
  ${CMAKE_SOURCE_DIR}/src/include/sherpa-onnx/lib/${SHERPA_ONNX_OS_DIR}/${GLADYS_ARCH}
)

if(EXISTS ${SHERPA_ONNX_LIB_DIR})
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_libraries(gladys PRIVATE
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-c-api.lib
      ${SHERPA_ONNX_LIB_DIR}/sherpa-onnx-cxx-api.lib
      ${SHERPA_ONNX_LIB_DIR}/onnxruntime.lib
    )
    target_link_options(gladys PRIVATE
      /DELAYLOAD:sherpa-onnx-c-api.dll /DELAYLOAD:sherpa-onnx-cxx-api.dll
    )
  else()
    target_link_directories(gladys PRIVATE ${SHERPA_ONNX_LIB_DIR})
    target_link_libraries(gladys PRIVATE sherpa-onnx-c-api sherpa-onnx-cxx-api onnxruntime)
  endif()
  # macOS build-tree run: look for the dylibs next to the executable
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(gladys PROPERTIES BUILD_RPATH "@loader_path")
  endif()

  # Pick the runtime libraries for this OS
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.so*)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dylib)
  else()
    file(GLOB SHERPA_RUNTIME_LIBS ${SHERPA_ONNX_LIB_DIR}/*.dll)
  endif()
  # Install them where the app's runtime search path looks
  if(WIN32)
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION .)
  elseif(APPLE)
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION gladys.app/Contents/Frameworks)
  else()
    install(FILES ${SHERPA_RUNTIME_LIBS} DESTINATION lib)
  endif()
else()
  message(WARNING "sherpa-onnx library not found - speech-to-text will not work")
endif()

# Install the app; Qt runtime is bundled afterwards by *deployqt in scripts/build.sh
install(TARGETS gladys RUNTIME DESTINATION . BUNDLE DESTINATION .)
